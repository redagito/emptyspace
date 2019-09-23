
#define NOMINMAX
#include <emptyspace/geometry.hpp>
#include <emptyspace/physics.hpp>
#include <emptyspace/program.hpp>
#include <emptyspace/textures.hpp>
#include <emptyspace/types.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include "emptyspace/light.hpp"
#include "emptyspace/filewatcher.hpp"

static f64 g_MousePosXOld = {};
static f64 g_MousePosYOld = {};

s32 g_Screen_Width{};
s32 g_Screen_Height{};
u32 g_Window_Width = {1920};
u32 g_Window_Height = {1080};
GLFWwindow* g_Window = nullptr;

PhysicsScene* g_PhysicsScene = nullptr;
glm::mat4 g_Camera_View = glm::mat4(1.0f);

static float g_CubeAngle = 0.0f;

Program* g_Program_Final{ nullptr };
Program* g_Program_GBuffer{ nullptr };
Program* g_Program_MotionBlur{ nullptr };
Program* g_Program_Light{ nullptr };

inline glm::vec3 OrbitAxis(const f32 angle, const glm::vec3& axis, const glm::vec3& spread)
{
	return glm::angleAxis(angle, axis) * spread;
}

inline float Lerp(const f32 a, const f32 b, const f32 f)
{
	return a + f * (b - a);
}

#if _DEBUG
void APIENTRY DebugCallback(const u32 source, const u32 type, const u32 id, const u32 severity, s32 length, const GLchar* message, const void* userParam)
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

enum class Shape
{
	Cube = 0,
	Quad = 1,
	CubeInstanced = 2,
	Ship
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

void Cleanup()
{
	delete g_Program_GBuffer;
	delete g_Program_Final;
	delete g_Program_MotionBlur;
	delete g_Program_Light;
	delete g_PhysicsScene;
}

// shamelessly stolen from the learnopengl tutorial
std::vector<glm::mat4> CreateAsteroidInstances(const u32 instanceCount)
{
	std::vector<glm::mat4> modelMatrices;

	srand(static_cast<int>(glfwGetTime()));
	const auto radius = 200.0f;
	const auto offset = 100.5f;
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
		const auto scale = rand() % 60 / 100.0f + 2.05f;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		const auto rotAngle = f32(rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices.push_back(model);
	}

	return modelMatrices;
}

std::vector<Light> CreateRandomLights(const u32 instanceCount)
{
	std::vector<Light> lights;

	srand(static_cast<int>(glfwGetTime()));
	const auto radius = 200.0f;
	const auto offset = 100.5f;
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
		const auto position = glm::vec3(x, y, z);

		const auto color = glm::vec3((rand() % 256) / 256.0f, (rand() % 256) / 256.0f, (rand() % 256) / 256.0f);

		// 2. scale: Scale between 0.05 and 0.25f
		const auto scale = rand() % 60;// / 100.0f + 2.05f;
		const auto attenuation = glm::vec3(scale);

		// 4. now add to list of matrices
		lights.emplace_back(position, color, attenuation.r);
	}

	return lights;
}

std::vector<Light> CreateLights()
{
	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(-80, 1, +80), glm::vec3(0.0f, 0.0f, 1.0f), 64.0f);
	lights.emplace_back(glm::vec3(-80, 1, -80), glm::vec3(1.0f, 1.0f, 0.0f), 20.0f);
	lights.emplace_back(glm::vec3(+80, 1, -80), glm::vec3(0.0f, 1.0f, 0.0f), 20.0f);
	lights.emplace_back(glm::vec3(+80, 1, +80), glm::vec3(1.2f, 0.3f, 1.1f), 20.0f);

	return lights;
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

void GetWorkingArea(s32* screenWidth, s32* screenHeight)
{
	const auto primaryMonitor = glfwGetPrimaryMonitor();

	s32 workingAreaXPos{};
	s32 workingAreaYPos{};

	glfwGetMonitorWorkarea(primaryMonitor, &workingAreaXPos, &workingAreaYPos, screenWidth, screenHeight);
}

GLFWwindow* CreateMainWindow(const u32 width, const u32 height, const std::string_view title)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	const auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (window == nullptr)
	{
		glfwTerminate();
		return nullptr;
	}

	glfwSetFramebufferSizeCallback(window, WindowOnFramebufferResized);
	glfwSetCursorPosCallback(window, WindowOnMouseMove);

	glfwSetWindowPos(window, (g_Screen_Width / 2) - (g_Window_Width / 2), (g_Screen_Height / 2) - (g_Window_Height / 2));
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	return window;
}

void InitializeOpenGL(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	if (!gladLoadGL())
	{
		std::cerr << "GLAD: Unable to initialize.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	std::clog << "GL: VENDOR = " << glGetString(GL_VENDOR) << '\n';
	std::clog << "GL: VERSION = " << glGetString(GL_VERSION) << '\n';
	std::clog << "GL: GLSL VERSION = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

#if _DEBUG
	if (glDebugMessageCallback)
	{
		std::clog << "GL: Registered OpenGL Debug Callback.\n";
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(DebugCallback, nullptr);
		u32 unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
	}
	else
	{
		std::cerr << "GL: glDebugMessageCallback not available.\n";
	}
#endif

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void InitializePhysics()
{
	g_PhysicsScene = new PhysicsScene();
}

//void InitializeFileSystemWatcher()
//{
//	FileWatcher fileWatcher{ "../../emptyspace/res/shaders", std::chrono::milliseconds(2000) };
//	fileWatcher.Start([](std::string pathToWatch, FileStatus fileStatus) -> void
//		{
//			if (fileStatus == FileStatus::Modified)
//			{
//				std::clog << pathToWatch << " modified.\n";
//
//				if ((pathToWatch.find("main.fs.glsl") != std::string::npos || pathToWatch.find("main.vs.glsl") != std::string::npos))
//				{
//					delete g_Program_Final;
//					g_Program_Final = new Program("../../emptyspace/res/shaders/main.vs.glsl", "../../emptyspace/res/shaders/main.fs.glsl");
//				}
//			}
//		});
//}

int main(int argc, char* argv[])
{
	if (!glfwInit())
	{
		std::cerr << "GLFW: Unable to initialize.\n";
		return 1;
	}

	GetWorkingArea(&g_Screen_Width, &g_Screen_Height);

	g_Window_Width = u32(0.8f * g_Screen_Width);
	g_Window_Height = u32(0.8f * g_Screen_Height);

	g_Window = CreateMainWindow(g_Window_Width, g_Window_Height, "emptyspace");
	if (g_Window == nullptr)
	{
		std::cerr << "GLFW: Unable to create a window.\n";
		return 1;
	}

	InitializeOpenGL(g_Window);
	InitializePhysics();
   	 
	std::vector<Vertex> const cubeVertices =
	{
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
	};

	std::vector<Vertex> const quadVertices =
	{
		Vertex(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
	};

	std::vector<u8> const cubeIndices =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,

		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	std::vector<u8> const quadIndices =
	{
		0, 1, 2, 2, 3, 0,
	};

	auto const texture_cube_diffuse = CreateTexture2DFromfile("./res/textures/T_Default_D.png", STBI_rgb);
	auto const texture_cube_specular = CreateTexture2DFromfile("./res/textures/T_Default_S.png", STBI_grey);
	auto const texture_cube_normal = CreateTexture2DFromfile("./res/textures/T_Default_N.png", STBI_rgb);
	auto const texture_skybox = CreateTextureCubeFromFiles({
		"./res/textures/TC_SkyRed_Xn.png",
		"./res/textures/TC_SkyRed_Xp.png",
		"./res/textures/TC_SkyRed_Yn.png",
		"./res/textures/TC_SkyRed_Yp.png",
		"./res/textures/TC_SkyRed_Zn.png",
		"./res/textures/TC_SkyRed_Zp.png"
	});

	const auto screen_width = g_Window_Width;
	const auto screen_height = g_Window_Height;

	/* framebuffer textures */
	auto const texture_gbuffer_final = CreateTexture2D(GL_RGB8, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	
	auto const texture_gbuffer_position = CreateTexture2D(GL_RGB16F, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_normal = CreateTexture2D(GL_RGB16F, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_albedo = CreateTexture2D(GL_RGBA16F, GL_RGBA, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_depth = CreateTexture2D(GL_DEPTH_COMPONENT32, GL_DEPTH, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_gbuffer_velocity = CreateTexture2D(GL_RG16F, GL_RG, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_lbuffer_lights = CreateTexture2D(GL_RGB16F, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_motion_blur = CreateTexture2D(GL_RGB8, GL_RGB, screen_width, screen_height, nullptr, GL_NEAREST);
	auto const texture_motion_blur_mask = CreateTexture2D(GL_R8, GL_RED, screen_width, screen_height, nullptr, GL_NEAREST);

	auto const framebufferGeometry = CreateFramebuffer({ texture_gbuffer_position, texture_gbuffer_normal, texture_gbuffer_albedo, texture_gbuffer_velocity }, texture_gbuffer_depth);
	auto const framebufferFinal = CreateFramebuffer({ texture_gbuffer_final });
	auto const framebufferBlur = CreateFramebuffer({ texture_motion_blur });
	auto const framebufferLights = CreateFramebuffer({ texture_lbuffer_lights });

	/* vertex formatting information */
	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(Vertex, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(Vertex, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(Vertex, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(Vertex, Texcoord))
	};

	std::vector<AttributeFormat> const lightVertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPosition, Position))
	};

	/* geometry buffers */
	auto const emptyVao = []
	{
		u32 name = 0;
		glCreateVertexArrays(1, &name);
		return name;
	}();
	auto const [cubeVao, cubeVbo, cubeIbo] = CreateGeometry(cubeVertices, cubeIndices, vertexFormat);
	auto const [quadVao, quadVbo, quadIbo] = CreateGeometry(quadVertices, quadIndices, vertexFormat);
	auto const [shipVao, shipVbo, shipIbo, shipVertexCount, shipIndexCount] = CreateGeometryFromFile("./res/models/shipA_noWindshield.obj", vertexFormat);
	auto const [pointLightVao, pointLightVbo, pointLightIbo, pointLightVertexCount, pointLightIndexCount] = CreatePlainGeometryFromFile("./res/models/PointLight2.obj", lightVertexFormat);

	auto const asteroidInstances = CreateAsteroidInstances(50000);
	auto const asteroidsInstanceBuffer = CreateBuffer(asteroidInstances);

	auto const lights = CreateRandomLights(200); 
	//auto const lightsBuffer = CreateShaderStorageBuffer(lights.data(), lights.size());

	/* shaders */
	g_Program_Final = new Program("../../emptyspace/res/shaders/main.vs.glsl", "../../emptyspace/res/shaders/main.fs.glsl");
	g_Program_GBuffer = new Program("../../emptyspace/res/shaders/gbuffer.vs.glsl", "../../emptyspace/res/shaders/gbuffer.fs.glsl");
	g_Program_MotionBlur = new Program("../../emptyspace/res/shaders/blur.vs.glsl", "../../emptyspace/res/shaders/blur.fs.glsl");
	g_Program_Light = new Program("../../emptyspace/res/shaders/pointlight.vert.glsl", "../../emptyspace/res/shaders/pointlight.frag.glsl");

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
	
	g_Program_GBuffer->SetVertexShaderUniform(kUniformProjection, cameraProjection);

	std::vector<SceneObject> objects =
	{
		SceneObject(Shape::Quad),
		SceneObject(Shape::Cube),
	    SceneObject(Shape::Ship),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::CubeInstanced)
	};
	
	auto t1 = glfwGetTime();

	auto deltaTimeAverage = 0.0f;
	auto deltaTimeAverageSquared = 0.0f;

	auto framesToAverage = 100;
	auto frameCounter = 0;

	glfwSwapInterval(1);

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
			sprintf_s(str, "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames", deltaTimeAverage * 1000.0f,
			          1000.0f * deltaTimeStandardError, 1.0f / deltaTimeAverage, framesToAverage);
			glfwSetWindowTitle(g_Window, str);

			framesToAverage = static_cast<int>(1.0f / deltaTimeAverage);

			deltaTimeAverage = 0.0f;
			deltaTimeAverageSquared = 0.0f;
			frameCounter = 0;
		}

		Update(deltaTime);
		const auto cameraPositionRaw = g_PhysicsScene->Camera->getGlobalPose().p;
		const auto cameraOrientation = g_PhysicsScene->Camera->getGlobalPose().q;
		const auto cameraDirectionRaw = cameraOrientation.getBasisVector2();
		const auto cameraPosition = glm::vec3(cameraPositionRaw.x, cameraPositionRaw.y, cameraPositionRaw.z);
		const auto cameraDirection = glm::vec3(cameraDirectionRaw.x, cameraDirectionRaw.y, cameraDirectionRaw.z);

		/* Cube orbit */
		static auto cubeSpeed = 0.125f;
		auto const orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		static auto orbitProgression = 0.0f;

		glm::quat q = glm::rotate(glm::mat4(1.0f), cameraDirection.y, glm::vec3(0.0f, 1.0f, 0.0f));

		objects[0].ModelViewProjection = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
		objects[1].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter) * glm::rotate(glm::mat4(1.0f), orbitProgression * cubeSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
		objects[2].ModelViewProjection = glm::translate(glm::mat4(1.0f), cameraPosition - 2.0f * cameraDirection) * glm::rotate(glm::mat4(1.0f), cameraDirection.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), cameraPosition + 2.0f * cameraDirection);

		const auto objectCount = objects.size();
		for (std::size_t i = 3; i < objectCount; i++)
		{
			auto const orbitAmount = (orbitProgression * cubeSpeed + float(i) * 90.0f * glm::pi<float>() / 180.0f);
			auto const orbitPosition = OrbitAxis(orbitAmount, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(2.0f, 0.0f, 2.0f)) + glm::vec3(0.0f, 0.0f, 0.0f);
			objects[i].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter + orbitPosition) * glm::rotate(glm::mat4(1.0f), orbitAmount * 7.0f, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		orbitProgression += 0.1f;

		g_Program_GBuffer->SetVertexShaderUniform(kUniformView, g_Camera_View);

		/* g-buffer pass ================================================================================================== begin */
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

		g_Program_GBuffer->Use();

		for (auto& object : objects)
		{
			switch (object.ObjectShape)
			{
			    case Shape::Cube: glBindVertexArray(cubeVao); break;
			    case Shape::CubeInstanced:
				{
					glBindVertexArray(cubeVao);
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, asteroidsInstanceBuffer);
					object.ExcludeFromMotionBlur = true;
					break;
				}
				case Shape::Ship: glBindVertexArray(shipVao); break;
			    case Shape::Quad: glBindVertexArray(quadVao); break;
			}

			auto const currentModelViewProjection = cameraProjection * g_Camera_View * object.ModelViewProjection;

			g_Program_GBuffer->SetVertexShaderUniform(kUniformModel, object.ModelViewProjection);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformModelViewProjection, currentModelViewProjection);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformModelViewProjectionInverse, object.ModelViewProjectionPrevious);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformBlurExcept, object.ExcludeFromMotionBlur);
			g_Program_GBuffer->SetVertexShaderUniform(6, object.ObjectShape == Shape::CubeInstanced);

			object.ModelViewProjectionPrevious = currentModelViewProjection;

			switch (object.ObjectShape)
			{
			    case Shape::Cube: glDrawElements(GL_TRIANGLES, u32(cubeIndices.size()), GL_UNSIGNED_BYTE, nullptr); break;
			    case Shape::CubeInstanced: glDrawElementsInstanced(GL_TRIANGLES, u32(cubeIndices.size()), GL_UNSIGNED_BYTE, nullptr, asteroidInstances.size()); break;
			    case Shape::Quad: glDrawElements(GL_TRIANGLES, u32(quadIndices.size()), GL_UNSIGNED_BYTE, nullptr); break;
			    case Shape::Ship: glDrawArrays(GL_TRIANGLES, 0, shipVertexCount); break;
			}
		}
		/* g-buffer pass ================================================================================================== end */
		/* lights ======================================================================================================= begin */
		
		glClearNamedFramebufferfv(framebufferLights, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(framebufferLights, GL_DEPTH, 0, &depthClearValue);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferLights);

		glBindTextureUnit(0, texture_gbuffer_position);
		glBindTextureUnit(1, texture_gbuffer_normal);
		glBindTextureUnit(2, texture_gbuffer_depth);
		
		g_Program_Light->Use();
		glBindVertexArray(pointLightVao);
		glCullFace(GL_FRONT);
		for (auto& light : lights)
		{
			auto model = glm::translate(glm::mat4(1.0f), glm::vec3(light.Position));
			model = glm::scale(model, glm::vec3(light.Attenuation.x, light.Attenuation.x, light.Attenuation.x));
			g_Program_Light->SetVertexShaderUniform(0, cameraProjection);
			g_Program_Light->SetVertexShaderUniform(1, g_Camera_View);
			g_Program_Light->SetVertexShaderUniform(2, model);
			g_Program_Light->SetFragmentShaderUniform(0, light.Position);
			g_Program_Light->SetFragmentShaderUniform(1, light.Color);
			g_Program_Light->SetFragmentShaderUniform(2, light.Attenuation);
			g_Program_Light->SetFragmentShaderUniform(3, cameraPosition);
			
			glDrawArrays(GL_TRIANGLES, 0, pointLightVertexCount);
		}
		glCullFace(GL_BACK);

		/* lights ================================================================================ end */

		/* resolve gbuffer */
		glClearNamedFramebufferfv(framebufferFinal, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(framebufferFinal, GL_DEPTH, 0, &depthClearValue);

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferFinal);

		glBindTextureUnit(0, texture_gbuffer_position);
		glBindTextureUnit(1, texture_gbuffer_normal);
		glBindTextureUnit(2, texture_gbuffer_albedo);
		glBindTextureUnit(3, texture_gbuffer_depth);
		glBindTextureUnit(4, texture_skybox);
		glBindTextureUnit(5, texture_lbuffer_lights);

		g_Program_Final->Use();
		glBindVertexArray(emptyVao);

		g_Program_Final->SetFragmentShaderUniform(kUniformCameraPosition, glm::vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
		g_Program_Final->SetVertexShaderUniform(kUniformCameraDirection, glm::inverse(glm::mat3(g_Camera_View)));
		g_Program_Final->SetVertexShaderUniform(kUniformFieldOfView, fieldOfView);
		g_Program_Final->SetVertexShaderUniform(kUniformAspectRatio, float(ViewportWidth) / float(ViewportHeight));
		g_Program_Final->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(
			float(ViewportWidth) / float(screen_width),
			float(ViewportHeight) / float(screen_height)
		));
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightsBuffer);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* motion blur */

		glClearNamedFramebufferfv(framebufferBlur, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferBlur);

		glBindTextureUnit(0, texture_gbuffer_final);
		glBindTextureUnit(1, texture_gbuffer_velocity);

		g_Program_MotionBlur->Use();
		glBindVertexArray(emptyVao);

		g_Program_MotionBlur->SetFragmentShaderUniform(kUniformBlurBias, 4.0f);
		g_Program_MotionBlur->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(
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

	Cleanup();

	return 0;
}
