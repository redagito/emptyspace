#include <emptyspace/types.hpp>
#include <emptyspace/physics.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <array>
#include <fstream>

static f64 g_MousePosXOld = {};
static f64 g_MousePosYOld = {};

u32 g_Window_Width = {1920};
u32 g_Window_Height = {1080};
GLFWwindow* g_Window = nullptr;

PhysicsScene* g_PhysicsScene = nullptr;
glm::mat4 g_Camera_View = glm::mat4(1.0f);

static float g_CubeAngle = 0.0f;

inline std::string ReadTextFile(std::string_view filepath)
{
	if (!std::filesystem::exists(filepath.data()))
	{
		std::ostringstream message;
		message << "file " << filepath.data() << " does not exist.";
		throw std::filesystem::filesystem_error(message.str(),
		                                        std::make_error_code(std::errc::no_such_file_or_directory));
	}
	std::ifstream file(filepath.data());
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 Texcoord;

	Vertex(glm::vec3 const& position, glm::vec3 const& color, glm::vec3 const& normal, glm::vec2 const& texcoord)
		: Position(position), Color(color), Normal(normal), Texcoord(texcoord)
	{
	}
};

struct AttributeFormat
{
	GLuint Index;
	GLint Size;
	GLenum Type;
	GLuint RelativeOffset;
};

template <typename T>
constexpr std::pair<GLint, GLenum> TypeToSize()
{
	if constexpr (std::is_same_v<T, float>)
		return std::make_pair(1, GL_FLOAT);
	if constexpr (std::is_same_v<T, int>)
		return std::make_pair(1, GL_INT);
	if constexpr (std::is_same_v<T, unsigned int>)
		return std::make_pair(1, GL_UNSIGNED_INT);
	if constexpr (std::is_same_v<T, glm::vec2>)
		return std::make_pair(2, GL_FLOAT);
	if constexpr (std::is_same_v<T, glm::vec3>)
		return std::make_pair(3, GL_FLOAT);
	if constexpr (std::is_same_v<T, glm::vec4>)
		return std::make_pair(4, GL_FLOAT);
	throw std::runtime_error("GL: Unsupported type");
}

template <typename T>
inline AttributeFormat CreateAttributeFormat(GLuint index, GLuint relateOffset)
{
	auto const [comp_count, type] = TypeToSize<T>();
	return AttributeFormat{index, comp_count, type, relateOffset};
}

template <typename T>
inline GLuint CreateBuffer(std::vector<T> const& buff, const u32 flags = GL_DYNAMIC_STORAGE_BIT)
{
	GLuint name = 0;
	glCreateBuffers(1, &name);
	glNamedBufferStorage(name, sizeof(typename std::vector<T>::value_type) * buff.size(), buff.data(), flags);
	return name;
}

template <typename T>
std::tuple<GLuint, GLuint, GLuint> CreateGeometry(const std::vector<T>& vertices, const std::vector<u8>& indices, const std::vector<AttributeFormat>& attributeFormats)
{
	u32 vao = 0;
	auto vbo = CreateBuffer(vertices, GL_MAP_READ_BIT);
	auto ibo = CreateBuffer(indices, GL_MAP_READ_BIT);

	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(T));
	glVertexArrayElementBuffer(vao, ibo);

	for (auto const& format : attributeFormats)
	{
		glEnableVertexArrayAttrib(vao, format.Index);
		glVertexArrayAttribFormat(vao, format.Index, format.Size, format.Type, GL_FALSE, format.RelativeOffset);
		glVertexArrayAttribBinding(vao, format.Index, 0);
	}

	return std::make_tuple(vao, vbo, ibo);
}

void ValidateProgram(const u32 shader, const std::string_view filename)
{
	s32 compiled = 0;
	glProgramParameteri(shader, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glGetProgramiv(shader, GL_LINK_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		std::array<char, 1024> compilerLog{};
		glGetProgramInfoLog(shader, compilerLog.size(), nullptr, compilerLog.data());
		glDeleteShader(shader);

		std::ostringstream message;
		message << "shader " << filename << " contains error(s):\n\n" << compilerLog.data() << '\n';
		std::clog << message.str();
	}
}

std::tuple<u32, u32, u32> CreateProgram(const std::string_view vertexShaderFilePath, const std::string_view fragmentShaderFilePath)
{
	auto const vertexShaderSource = ReadTextFile(vertexShaderFilePath);
	auto const fragmentShaderSource = ReadTextFile(fragmentShaderFilePath);

	auto const vertexShaderData = vertexShaderSource.data();
	auto const fragmentShaderData = fragmentShaderSource.data();
	u32 pipeline = 0;
	auto vert = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShaderData);
	auto frag = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShaderData);

	ValidateProgram(vert, vertexShaderFilePath);
	ValidateProgram(frag, fragmentShaderFilePath);

	glCreateProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vert);
	glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, frag);

	return std::make_tuple(pipeline, vert, frag);
}

u32 CreateTexture2D(const u32 internalFormat, const u32 format, const s32 width, const s32 height, void* data = nullptr, const u32 filter = GL_LINEAR, const u32 repeat = GL_REPEAT)
{
	u32 name = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glTextureStorage2D(name, 1, internalFormat, width, height);

	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, filter);
	glTextureParameteri(name, GL_TEXTURE_WRAP_S, repeat);
	glTextureParameteri(name, GL_TEXTURE_WRAP_T, repeat);

	if (data)
	{
		glTextureSubImage2D(name, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	}

	return name;
}

template <typename T = nullptr_t>
u32 CreateTextureCube(const u32 internalFormat, const u32 format, const s32 width, const s32 height, std::array<T*, 6> const& data)
{
	u32 name = 0;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
	glTextureStorage2D(name, 1, internalFormat, width, height);

	for (s32 i = 0; i < 6; ++i)
	{
		if (data[i])
		{
			glTextureSubImage3D(name, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
		}
	}

	return name;
}

using stb_comp_t = decltype(STBI_default);

GLuint CreateTexture2DFromfile(const std::string_view filepath, stb_comp_t comp = STBI_rgb_alpha)
{
	s32 width{};
	s32 height{};
	s32 components{};
	
	if (!std::filesystem::exists(filepath.data()))
	{
		std::ostringstream message;
		message << "file " << filepath.data() << " does not exist.";
		throw std::runtime_error(message.str());
	}
	const auto data = stbi_load(filepath.data(), &width, &height, &components, comp);

	auto const [internalFormat, format] = [comp]()
	{
		switch (comp)
		{
		case STBI_rgb_alpha: return std::make_pair(GL_RGBA8, GL_RGBA);
		case STBI_rgb: return std::make_pair(GL_RGB8, GL_RGB);
		case STBI_grey: return std::make_pair(GL_R8, GL_RED);
		case STBI_grey_alpha: return std::make_pair(GL_RG8, GL_RG);
		default: throw std::runtime_error("GL: Invalid format");
		}
	}();

	const auto name = CreateTexture2D(internalFormat, format, width, height, data);
	stbi_image_free(data);
	return name;
}

GLuint CreateTextureCubeFromFile(const std::array<std::string_view, 6>& filepath, stb_comp_t comp = STBI_rgb_alpha)
{
	s32 width{};
	s32 height{};
	s32 components{};
	
	std::array<stbi_uc*, 6> faces{};

	auto const [in, ex] = [comp]()
	{
		switch (comp)
		{
		case STBI_rgb_alpha: return std::make_pair(GL_RGBA8, GL_RGBA);
		case STBI_rgb: return std::make_pair(GL_RGB8, GL_RGB);
		case STBI_grey: return std::make_pair(GL_R8, GL_RED);
		case STBI_grey_alpha: return std::make_pair(GL_RG8, GL_RG);
		default: throw std::runtime_error("invalid format");
		}
	}();

	for (auto i = 0; i < 6; i++)
	{
		faces[i] = stbi_load(filepath[i].data(), &width, &height, &components, comp);
	}

	const auto name = CreateTextureCube(in, ex, width, height, faces);

	for (auto face : faces)
	{
		stbi_image_free(face);
	}
	return name;
}

u32 CreateFramebuffer(const std::vector<GLuint>& colorAttachments, u32 depthAttachment = GL_NONE)
{
	u32 name = 0;
	glCreateFramebuffers(1, &name);

	for (auto i = 0; i < colorAttachments.size(); i++)
	{
		glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT0 + i, colorAttachments[i], 0);
	}

	std::array<u32, 32> drawBuffers{};
	for (u32 i = 0; i < colorAttachments.size(); i++)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glNamedFramebufferDrawBuffers(name, colorAttachments.size(), drawBuffers.data());

	if (depthAttachment != GL_NONE)
	{
		glNamedFramebufferTexture(name, GL_DEPTH_ATTACHMENT, depthAttachment, 0);
	}

	if (glCheckNamedFramebufferStatus(name, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("incomplete framebuffer");
	}
	return name;
}

template <typename T>
inline void SetProgramUniform(u32 shader, s32 location, T const& value)
{
	if constexpr (std::is_same_v<T, s32>)
	{
		glProgramUniform1i(shader, location, value);
	}
	else if constexpr (std::is_same_v<T, u32>)
	{
		glProgramUniform1ui(shader, location, value);
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		glProgramUniform1ui(shader, location, value);
	}
	else if constexpr (std::is_same_v<T, f32>)
	{
		glProgramUniform1f(shader, location, value);
	}
	else if constexpr (std::is_same_v<T, f64>)
	{
		glProgramUniform1d(shader, location, value);
	}
	else if constexpr (std::is_same_v<T, glm::vec2>)
	{
		glProgramUniform2fv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::vec3>)
	{
		glProgramUniform3fv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::vec4>)
	{
		glProgramUniform4fv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::ivec2>)
	{
		glProgramUniform2iv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::ivec3>)
	{
		glProgramUniform3iv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::ivec4>)
	{
		glProgramUniform4iv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::uvec2>)
	{
		glProgramUniform2uiv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::uvec3>)
	{
		glProgramUniform3uiv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::uvec4>)
	{
		glProgramUniform4uiv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::quat>)
	{
		glProgramUniform4fv(shader, location, 1, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::mat3>)
	{
		glProgramUniformMatrix3fv(shader, location, 1, GL_FALSE, glm::value_ptr(value));
	}
	else if constexpr (std::is_same_v<T, glm::mat4>)
	{
		glProgramUniformMatrix4fv(shader, location, 1, GL_FALSE, glm::value_ptr(value));
	}
	else throw std::runtime_error("unsupported type");
}

inline void DeleteShader(u32 programPipeline, u32 vertexShader, u32 fragmentShader)
{
	glDeleteProgramPipelines(1, &programPipeline);
	glDeleteProgram(vertexShader);
	glDeleteProgram(fragmentShader);
}

inline glm::vec3 OrbitAxis(const f32 angle, const glm::vec3& axis, const glm::vec3& spread)
{
	return glm::angleAxis(angle, axis) * spread;
}

inline float Lerp(const f32 a, const f32 b, const f32 f)
{
	return a + f * (b - a);
}

#if _DEBUG
void APIENTRY GLDebugCallback(const u32 source, const u32 type, const u32 id, const u32 severity, s32 length, const GLchar* message, const void* userParam)
{
	std::ostringstream str;
	str << "---------------------GL CALLBACK---------------------\n";
	str << "Message: " << message << '\n';
	str << "Source: ";
	switch (source) 
	{
		case GL_DEBUG_SOURCE_API:
		    str << "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		    str << "WINDOW_SYSTEM";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		    str << "SHADER_COMPILER";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		    str << "THIRD_PARTY";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
		    str << "APPLICATION";
			break;
		case GL_DEBUG_SOURCE_OTHER:
		    str << "OTHER";
			break;
		default:
			str << "UNKNOWN SOURCE";
			break;
	}
	str << '\n';
	str << "Type: ";
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
		    str << "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		    str << "DEPRECATED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		    str << "UNDEFINED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
		    str << "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
		    str << "PERFORMANCE";
			break;
		case GL_DEBUG_TYPE_MARKER:
		    str << "MARKER";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
		    str << "PUSH_GROUP";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
		    str << "POP_GROUP";
			break;
		case GL_DEBUG_TYPE_OTHER:
		    str << "OTHER";
			break;
		default:
			str << "UNKNOWN TYPE";
			break;
	}
	str << '\n';
	str << "Id: " << id << '\n';
	str << "Severity: ";
	switch (severity)
	{
	    case GL_DEBUG_SEVERITY_NOTIFICATION:
		    str << "NOTIFICATION";
			break;
		case GL_DEBUG_SEVERITY_LOW:
		    str << "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
		    str << "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
		    str << "HIGH";
			break;
		default:
			str << "UNKNOWN SEVERITY";
			break;
	}
	str << '\n';
	str << "---------------------GL CALLBACK---------------------\n";

	std::clog << str.str();
}
#endif

template <typename ... Args>
std::string FormatString(const std::string& format, Args ... args)
{
	const size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	const std::unique_ptr<char[]> buffer(new char[size]);
	snprintf(buffer.get(), size, format.c_str(), args ...);
	return std::string(buffer.get(), buffer.get() + size - 1);
}

enum class Shape
{
	Cube = 0,
	Quad = 1
};

struct SceneObject
{
	glm::mat4 ModelViewProjection;
	glm::mat4 ModelViewProjectionPrevious;
	Shape ObjectShape;
	bool ExcludeFromMotionBlur;

	SceneObject(const Shape shape = Shape::Cube, const bool except = false)
	    : ModelViewProjection(), ModelViewProjectionPrevious(), ObjectShape(shape), ExcludeFromMotionBlur(except)
	{
	}
};

template <typename T = std::chrono::milliseconds>
int64_t Now()
{
	return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

// shamelessly stolen from the learnopengl tutorial
std::vector<glm::mat4> GameCreateAsteroidInstances(const u32 instanceCount)
{
	std::vector<glm::mat4> modelMatrices;

	srand(static_cast<int>(glfwGetTime()));
	const auto radius = 50.0f;
	const auto offset = 25.5f;
	for (u32 i = 0; i < instanceCount; i++)
	{
		auto model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		const auto angle = static_cast<float>(i) / static_cast<float>(instanceCount) * 360.0f;
		auto displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

		const auto x = sin(angle) * radius + displacement;
		displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

		const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = rand() % static_cast<int>(2.0f * offset * 100) / 100.0f - offset;

		const auto z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		const auto scale = rand() % 60 / 100.0f + 0.05f;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		const auto rotAngle = f32(rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices.push_back(model);
	}

	return modelMatrices;
}


void HandleInput(const f32 deltaTime)
{
	if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(g_Window, true);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Forward);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Backward);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Left);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Right);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::RollCW);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::RollCCW);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Stop);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Up);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Down);
	}
}

void Update(const float deltaTime)
{
	glfwPollEvents();
	HandleInput(deltaTime);

	g_PhysicsScene->Step(deltaTime);
	const auto physicsCamera = g_PhysicsScene->Camera;
	const auto transform = physicsCamera->getGlobalPose();

	const auto pos = glm::vec3(transform.p.x, transform.p.y, transform.p.z);
	const auto quat = glm::quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
	auto euler = glm::eulerAngles(quat);

	auto viewMatrix = glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(quat);

	// View is the inverse of camera
	// I.e. camera moves left, view moves right
	viewMatrix = glm::inverse(viewMatrix);

	g_Camera_View = viewMatrix;
}


void WindowOnFramebufferResized(GLFWwindow* window, const int width, const int height)
{
	g_Window_Width = width;
	g_Window_Height = height;
	glViewport(0, 0, width, height);
}

void WindowOnMouseMove(GLFWwindow* window, const double xpos, const double ypos)
{
	if (g_PhysicsScene != nullptr)
	{
		g_PhysicsScene->Tumble(static_cast<float>(xpos - g_MousePosXOld),
		                       static_cast<float>(ypos - g_MousePosYOld));
		g_MousePosXOld = xpos;
		g_MousePosYOld = ypos;
	}
}

int main(int argc, char* argv[])
{
	if (!glfwInit())
	{
		std::cerr << "GLFW: Unable to initialize.\n";
		return 1;
	}

	const auto primaryMonitor = glfwGetPrimaryMonitor();
	s32 screenWidth{};
	s32 screenHeight{};
	s32 workingAreaXPos{};
	s32 workingAreaYPos{};

	glfwGetMonitorWorkarea(primaryMonitor, &workingAreaXPos, &workingAreaYPos, &screenWidth, &screenHeight);

	g_Window_Width = u32(0.8f * screenWidth);
	g_Window_Height = u32(0.8f * screenHeight);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	g_Window = glfwCreateWindow(g_Window_Width, g_Window_Height, "emptyspace", nullptr, nullptr);
	if (g_Window == nullptr)
	{
		std::cerr << "GLFW: Unable to create a window.\n";
		glfwTerminate();
		return 1;
	}

	glfwSetFramebufferSizeCallback(g_Window, WindowOnFramebufferResized);
	glfwSetCursorPosCallback(g_Window, WindowOnMouseMove);

	glfwSetWindowPos(g_Window, screenWidth / 2 - (g_Window_Width / 2), screenHeight / 2 - (g_Window_Height / 2));

	glfwMakeContextCurrent(g_Window);
	glfwSwapInterval(1);
	if (!gladLoadGL())
	{
		std::cerr << "GLAD: Unable to initialize.\n";
		glfwDestroyWindow(g_Window);
		glfwTerminate();
	}

	std::clog << "GL: VENDOR = " << glGetString(GL_VENDOR) << '\n';
	std::clog << "GL: VERSION = " << glGetString(GL_VERSION) << '\n';
	std::clog << "GL: GLSL VERSION = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(g_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

//#if _DEBUG
//	if (glDebugMessageCallback)
//	{
//		std::clog << "GL: Registered OpenGL Debug Callback.\n";
//		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//		glDebugMessageCallback(GLDebugCallback, nullptr);
//		u32 unusedIds = 0;
//		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
//	}
//	else
//	{
//		std::cerr << "GL: glDebugMessageCallback not available.\n";
//	}
//#endif

	g_PhysicsScene = new PhysicsScene();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	std::vector<Vertex> const cubeVertices =
	{
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		         glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		         glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		         glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		         glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		         glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		         glm::vec2(1.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
		         glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
		         glm::vec2(1.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f),
		         glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f),
		         glm::vec2(1.0f, 1.0f)),
	};

	std::vector<Vertex> const quadVertices =
	{
		Vertex(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec2(0.0f, 1.0f)),
	};

	std::vector<uint8_t> const cubeIndices =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,

		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	std::vector<uint8_t> const quadIndices =
	{
		0, 1, 2, 2, 3, 0,
	};

	auto const texture_cube_diffuse = CreateTexture2DFromfile("./res/textures/T_Default_D.png", STBI_rgb);
	auto const texture_cube_specular = CreateTexture2DFromfile("./res/textures/T_Default_S.png", STBI_grey);
	auto const texture_cube_normal = CreateTexture2DFromfile("./res/textures/T_Default_N.png", STBI_rgb);
	auto const texture_skybox = CreateTextureCubeFromFile({
		"./res/textures/TC_SkySpace_Xn.png",
		"./res/textures/TC_SkySpace_Xp.png",
		"./res/textures/TC_SkySpace_Yn.png",
		"./res/textures/TC_SkySpace_Yp.png",
		"./res/textures/TC_SkySpace_Zn.png",
		"./res/textures/TC_SkySpace_Zp.png"
	});

	const auto screen_width = g_Window_Width;
	const auto screen_height = g_Window_Height;

	/* framebuffer textures */
	auto const texture_gbuffer_color = CreateTexture2D(GL_RGB8, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_position = CreateTexture2D(GL_RGB16F, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_normal = CreateTexture2D(GL_RGB16F, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_albedo = CreateTexture2D(GL_RGBA16F, GL_RGBA, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_depth = CreateTexture2D(GL_DEPTH_COMPONENT32, GL_DEPTH, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_velocity = CreateTexture2D(GL_RG16F, GL_RG, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_motion_blur = CreateTexture2D(GL_RGB8, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_motion_blur_mask = CreateTexture2D(GL_R8, GL_RED, screen_width, screen_height, nullptr, GL_NEAREST);

	auto const framebufferGeometry = CreateFramebuffer({
		                                           texture_gbuffer_position, texture_gbuffer_normal,
		                                           texture_gbuffer_albedo, texture_gbuffer_velocity
	                                           }, texture_gbuffer_depth);
	auto const framebufferFinal = CreateFramebuffer({texture_gbuffer_color});
	auto const framebufferBlur = CreateFramebuffer({texture_motion_blur});

	/* vertex formatting information */
	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(Vertex, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(Vertex, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(Vertex, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(Vertex, Texcoord))
	};

	/* geometry buffers */
	auto const emptyVao = []
	{
		GLuint name = 0;
		glCreateVertexArrays(1, &name);
		return name;
	}();
	auto const [cubeVao, cubeVbo, cubeIbo] = CreateGeometry(cubeVertices, cubeIndices, vertexFormat);
	auto const [quadVao, quadVbo, quadIbo] = CreateGeometry(quadVertices, quadIndices, vertexFormat);

	/* shaders */
	auto const [finalPipelineProgram, finalVertexShader, finalFragmentShader] = CreateProgram("./res/shaders/main.vs.glsl", "./res/shaders/main.fs.glsl");
	auto const [gBufferPipelineProgram, gBufferVertexShader, gBufferFragmentShader] = CreateProgram("./res/shaders/gbuffer.vs.glsl", "./res/shaders/gbuffer.fs.glsl");
	auto const [motionblurPipelineProgram, motionblurVertexShader, motionblurFragmentShader] = CreateProgram("./res/shaders/blur.vs.glsl", "./res/shaders/blur.fs.glsl");

	/* uniforms */
	constexpr auto kUniformProjection = 0;
	constexpr auto kUniformCameraPosition = 0;
	constexpr auto kUniformCameraDirection = 0;
	constexpr auto kUniformView = 1;
	constexpr auto kUniformFieldOfView = 1;
	constexpr auto kUniformAspectRatio = 2;
	constexpr auto kUniformModel = 2;
	constexpr auto kUniformLight = 3;
	constexpr auto kUniformBlurBias = 0;
	constexpr auto kUniformUvsDiff = 3;
	constexpr auto kUniformModelViewProjection = 3;
	constexpr auto kUniformModelViewProjectionInverse = 4;
	constexpr auto kUniformBlurExcept = 5;

	constexpr auto fieldOfView = glm::radians(60.0f);
	auto const cameraProjection = glm::perspective(fieldOfView, float(g_Window_Width) / float(g_Window_Height), 0.1f, 1000.0f);
	SetProgramUniform(gBufferVertexShader, kUniformProjection, cameraProjection);

	std::vector<SceneObject> objects =
	{
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Quad)
	};


	auto t1 = glfwGetTime();

	auto deltaTimeAverage = 0.0f;
	auto deltaTimeAverageSquared = 0.0f;

	auto framesToAverage = 100;
	auto frameCounter = 0;

	glfwSwapInterval(0);

	g_MousePosXOld = g_Window_Width / 2.0f;
	g_MousePosYOld = g_Window_Height / 2.0f;
	glfwSetCursorPos(g_Window, g_MousePosXOld, g_MousePosYOld);
	glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (!glfwWindowShouldClose(g_Window))
	{
		const auto t2 = glfwGetTime();
		const auto deltaTime = f32(t2 - t1);
		t1 = t2;

		deltaTimeAverage += deltaTime;
		deltaTimeAverageSquared += (deltaTime * deltaTime);
		frameCounter++;

		if (frameCounter == framesToAverage)
		{
			deltaTimeAverage /= framesToAverage;
			deltaTimeAverageSquared /= framesToAverage;
			const auto deltaTimeStandardError = sqrt(deltaTimeAverageSquared - deltaTimeAverage * deltaTimeAverage) /
				sqrt(framesToAverage);

			char str[76];
			sprintf_s(str, "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames", (deltaTimeAverage * 1000.0f),
			          1000.0f * deltaTimeStandardError, 1.0f / deltaTimeAverage, framesToAverage);
			glfwSetWindowTitle(g_Window, str);

			framesToAverage = static_cast<int>(1.0f / deltaTimeAverage);

			deltaTimeAverage = 0.0f;
			deltaTimeAverageSquared = 0.0f;
			frameCounter = 0;
		}

		Update(deltaTime);

		/* Cube orbit */
		static float cubeSpeed = 0.125f;
		auto const orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		static auto orbitProgression = 0.0f;

		objects[0].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter) * glm::rotate(glm::mat4(1.0f), orbitProgression * cubeSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

		for (auto i = 0; i < 4; i++)
		{
			auto const orbitAmount = (orbitProgression * cubeSpeed + float(i) * 90.0f * glm::pi<float>() / 180.0f);
			auto const orbitPosition = OrbitAxis(orbitAmount, glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f)) + glm::vec3(-2.0f, 0.0f, 0.0f);
			objects[1 + i].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter + orbitPosition) * glm::rotate(glm::mat4(1.0f), orbitAmount, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		orbitProgression += 0.1f;

		objects[5].ModelViewProjection = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));

		SetProgramUniform(gBufferVertexShader, kUniformView, g_Camera_View);

		/* g-buffer pass */
		static auto const ViewportWidth = screen_width;
		static auto const ViewportHeight = screen_height;
		glViewport(0, 0, ViewportWidth, ViewportHeight);

		auto const depthClearValue = 1.0f;
		glClearNamedFramebufferfv(framebufferGeometry, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(framebufferGeometry, GL_COLOR, 1, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(framebufferGeometry, GL_COLOR, 2, glm::value_ptr(glm::vec4(0.0f)));
		glClearNamedFramebufferfv(framebufferGeometry, GL_COLOR, 3, glm::value_ptr(glm::vec2(0.0f)));
		glClearNamedFramebufferfv(framebufferGeometry, GL_DEPTH, 0, &depthClearValue);

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferGeometry);

		glBindTextureUnit(0, texture_cube_diffuse);
		glBindTextureUnit(1, texture_cube_specular);
		glBindTextureUnit(2, texture_cube_normal);

		glBindProgramPipeline(gBufferPipelineProgram);

		for (auto& object : objects)
		{
			switch (object.ObjectShape)
			{
			    case Shape::Cube: glBindVertexArray(cubeVao); break;
			    case Shape::Quad: glBindVertexArray(quadVao); break;
			}

			auto const currentModelViewProjection = cameraProjection * g_Camera_View * object.ModelViewProjection;

			SetProgramUniform(gBufferVertexShader, kUniformModel, object.ModelViewProjection);
			SetProgramUniform(gBufferVertexShader, kUniformModelViewProjection, currentModelViewProjection);
			SetProgramUniform(gBufferVertexShader, kUniformModelViewProjectionInverse, object.ModelViewProjectionPrevious);
			SetProgramUniform(gBufferVertexShader, kUniformBlurExcept, object.ExcludeFromMotionBlur);

			object.ModelViewProjectionPrevious = currentModelViewProjection;

			switch (object.ObjectShape)
			{
			    case Shape::Cube: glDrawElements(GL_TRIANGLES, cubeIndices.size(), GL_UNSIGNED_BYTE, nullptr); break;
			    case Shape::Quad: glDrawElements(GL_TRIANGLES, quadIndices.size(), GL_UNSIGNED_BYTE, nullptr); break;
			}
		}

		/* resolve gbuffer */
		glClearNamedFramebufferfv(framebufferFinal, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(framebufferFinal, GL_DEPTH, 0, &depthClearValue);

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferFinal);

		glBindTextureUnit(0, texture_gbuffer_position);
		glBindTextureUnit(1, texture_gbuffer_normal);
		glBindTextureUnit(2, texture_gbuffer_albedo);
		glBindTextureUnit(3, texture_gbuffer_depth);
		glBindTextureUnit(4, texture_skybox);

		glBindProgramPipeline(finalPipelineProgram);
		glBindVertexArray(emptyVao);

		const auto cameraPosition = g_PhysicsScene->Camera->getGlobalPose().p;
		SetProgramUniform(finalFragmentShader, kUniformCameraPosition, glm::vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
		SetProgramUniform(finalVertexShader, kUniformCameraDirection, glm::inverse(glm::mat3(g_Camera_View)));
		SetProgramUniform(finalVertexShader, kUniformFieldOfView, fieldOfView);
		SetProgramUniform(finalVertexShader, kUniformAspectRatio, float(ViewportWidth) / float(ViewportHeight));
		SetProgramUniform(finalVertexShader, kUniformUvsDiff, glm::vec2(
			float(ViewportWidth) / float(screen_width),
			float(ViewportHeight) / float(screen_height)
		));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* motion blur */

		glClearNamedFramebufferfv(framebufferBlur, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferBlur);

		glBindTextureUnit(0, texture_gbuffer_color);
		glBindTextureUnit(1, texture_gbuffer_velocity);

		glBindProgramPipeline(motionblurPipelineProgram);
		glBindVertexArray(emptyVao);

		SetProgramUniform(motionblurFragmentShader, kUniformBlurBias, 4.0f);
		SetProgramUniform(motionblurVertexShader, kUniformUvsDiff, glm::vec2(
			float(ViewportWidth) / float(screen_width),
			float(ViewportHeight) / float(screen_height)
		));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* final output */
		glViewport(0, 0, g_Window_Width, g_Window_Height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBlitNamedFramebuffer(framebufferBlur, 0, 0, 0, ViewportWidth, ViewportHeight, 0, 0, g_Window_Width, g_Window_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glfwSwapBuffers(g_Window);
	}

	return 0;
}
