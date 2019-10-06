
#define NOMINMAX
#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/graphicsdevice.hpp>
#include <emptyspace/graphics/material.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/textures.hpp>
#include <emptyspace/graphics/light.hpp>
#include <emptyspace/io/filewatcher.hpp>
#include <emptyspace/math/frustum.hpp>
#include <emptyspace/physics.hpp>
#include <emptyspace/scenes/scenenode.hpp>
#include <emptyspace/scenes/spacescene.hpp>
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

static f64 g_MousePosXOld = {};
static f64 g_MousePosYOld = {};

s32 g_Screen_Width{};
s32 g_Screen_Height{};
s32 g_Window_Width{ 1920 };
s32 g_Window_Height{ 1080 };
GLFWwindow* g_Window{ nullptr };

PhysicsScene* g_PhysicsScene{ nullptr };
glm::mat4 g_Camera_View{ glm::mat4(1.0f) };
Light* g_Camera_Light{ nullptr };

Program* g_Program_Final{ nullptr };
Program* g_Program_GBuffer{ nullptr };
Program* g_Program_MotionBlur{ nullptr };
Program* g_Program_Light{ nullptr };
Program* g_Program_Quad{ nullptr };

Geometry* g_Geometry_Empty{ nullptr };
Geometry* g_Geometry_Cube{ nullptr };
Geometry* g_Geometry_Plane{ nullptr };
Geometry* g_Geometry_Ship{ nullptr };
Geometry* g_Geometry_PointLight{ nullptr };

Buffer* g_Buffer_Asteroids{ nullptr };

u32 g_Framebuffer_Geometry{ };
u32 g_Framebuffer_Final{ };
u32 g_Framebuffer_Motionblur{ };
u32 g_Framebuffer_Lights{ };
u32 g_Framebuffer_Transition{ };

std::vector<Material*> g_Materials;
std::vector<Scene*> g_Scenes;
Scene* g_Scene_Current{ nullptr };

Frustum g_Frustum;

bool g_Enable_MotionBlur{ false };
bool g_Enable_VSync{ false };

bool g_Enable_TransitionEffect{ false };
glm::vec4 g_Transition_Factor{ 0.0f, 0.0f, 0.0f, 0.0f };

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
	if (id == 131185)
	{
		return;
	}
	
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
		    //str << "NOTIFICATION";
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

void Cleanup()
{
	delete g_Program_GBuffer;
	delete g_Program_Final;
	delete g_Program_MotionBlur;
	delete g_Program_Light;
	delete g_Program_Quad;

	delete g_Geometry_Cube;
	delete g_Geometry_Plane;
	delete g_Geometry_Empty;
	delete g_Geometry_Ship;
	delete g_Geometry_PointLight;

	glDeleteFramebuffers(1, &g_Framebuffer_Geometry);
	glDeleteFramebuffers(1, &g_Framebuffer_Final);
	glDeleteFramebuffers(1, &g_Framebuffer_Motionblur);
	glDeleteFramebuffers(1, &g_Framebuffer_Lights);
	glDeleteFramebuffers(1, &g_Framebuffer_Transition);

	delete g_Buffer_Asteroids;
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
		lights.emplace_back(0, position, color, glm::vec3(0.032f, 0.09f, attenuation.r));
	}

	return lights;
}

std::vector<Light> CreateLights()
{
	std::vector<Light> lights;
	lights.emplace_back(0, glm::vec3(-80, 1, +80), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.032f, 0.09f, 60.0f));
	lights.emplace_back(0, glm::vec3(-80, 1, -80), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.032f, 0.09f, 60.0f));
	lights.emplace_back(0, glm::vec3(+80, 1, -80), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.032f, 0.09f, 60.0f));
	lights.emplace_back(0, glm::vec3(+80, 1, +80), glm::vec3(1.2f, 0.3f, 1.1f), glm::vec3(0.032f, 0.09f, 60.0f));

	return lights;
}

void HandleInput(const f32 deltaTime)
{
	if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(g_Window, true);
	}

	f32 acceleration = 0.05f;
	if (glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT))
	{
		acceleration *= 10;
	}

	if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Forward, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Backward, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Left, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Right, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::RollCW, 0.05f);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::RollCCW, 0.05f);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Stop, 0.0f);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Up, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Down, acceleration);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		g_Enable_TransitionEffect = true;
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

	viewMatrix = glm::inverse(viewMatrix);

	if (g_Enable_TransitionEffect)
	{
		g_Transition_Factor.w += 0.01f;
		if (g_Transition_Factor.w >= 1.0f)
		{
			g_Enable_TransitionEffect = false;
			g_Transition_Factor.w = 0.0f;
		}
	}

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

GLFWwindow* CreateMainWindow(const s32 width, const s32 height, const std::string_view title)
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

	glfwSetWindowPos(window, g_Screen_Width / 2 - width / 2, g_Screen_Height / 2 - height / 2);
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	return window;
}

void InitializeOpenGL(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
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

void RenderLights(const u32 gbufferPosition, 
	const u32 gbufferNormal, 
	const u32 gbufferDepth, 
	std::vector<Light>& lights, 
	glm::mat4 const cameraProjection, 
	const glm::vec3 cameraPosition, 
	const glm::vec3 cameraDirection, 
	int& visibleLights)
{
	auto constexpr depthClearValue = 1.0f;
	glClearNamedFramebufferfv(g_Framebuffer_Lights, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
	glClearNamedFramebufferfv(g_Framebuffer_Lights, GL_DEPTH, 0, &depthClearValue);
	
	glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer_Lights);

	glBindTextureUnit(0, gbufferPosition);
	glBindTextureUnit(1, gbufferNormal);
	glBindTextureUnit(2, gbufferDepth);
		
	g_Program_Light->Use();
	g_Geometry_PointLight->Bind();
	glCullFace(GL_FRONT);
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	auto lightIndex = 0;
	visibleLights = 0;
	for (auto& light : lights)
	{
		if (light.Type == 1)
		{
			light.Position = cameraPosition;
			light.Direction = cameraDirection;
			light.Attenuation = glm::vec3(0.001, 0.01, 20);
		}
		//if (lightIndex == 1)
		//{
		//	light.Position = cameraPosition + (-10.0f * cameraDirection);
		//	light.Color = glm::vec3(1.0f, 0.8f, 0.0f);
		//	light.Attenuation = glm::vec3(100, 100, 100);
		//}
		lightIndex++;


		if (!g_Frustum.SphereInFrustum(light.Position.x, light.Position.y, light.Position.z, light.Attenuation.z))
		{
			continue;
		}
		visibleLights++;
		auto model = glm::translate(glm::mat4(1.0f), glm::vec3(light.Position));
		model = glm::scale(model, glm::vec3(light.Attenuation.z, light.Attenuation.z, light.Attenuation.z));
		g_Program_Light->SetVertexShaderUniform(0, cameraProjection);
		g_Program_Light->SetVertexShaderUniform(1, g_Camera_View);
		g_Program_Light->SetVertexShaderUniform(2, model);

		g_Program_Light->SetFragmentShaderUniform(0, light.Type);
		g_Program_Light->SetFragmentShaderUniform(1, light.Position);
		g_Program_Light->SetFragmentShaderUniform(2, light.Color);
		g_Program_Light->SetFragmentShaderUniform(3, light.Direction);
		g_Program_Light->SetFragmentShaderUniform(4, light.Attenuation);
		g_Program_Light->SetFragmentShaderUniform(5, light.CutOff);
		g_Program_Light->SetFragmentShaderUniform(6, cameraPosition);
			
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 240, 1, 0);
	}
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
}

void RenderGBuffer(const u32 texture_skybox, 
	const u32 texture_gbuffer_position,
	const u32 texture_gbuffer_normal,
	const u32 texture_gbuffer_albedo,
	const u32 texture_gbuffer_depth,
	const u32 texture_lbuffer_lights,
	const int screenWidth, 
	const int screenHeight, 
	const float fieldOfView)
{
	auto constexpr kUniformCameraDirection = 0;
	auto constexpr kUniformFieldOfView = 1;
	auto constexpr kUniformAspectRatio = 2;
	auto constexpr kUniformUvsDiff = 3;
	auto constexpr depthClearValue = 1.0f;
	
	glClearNamedFramebufferfv(g_Framebuffer_Final, GL_COLOR, 0, glm::value_ptr(glm::vec3(1.0f)));
	glClearNamedFramebufferfv(g_Framebuffer_Final, GL_DEPTH, 0, &depthClearValue);

	glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer_Final);

	glBindTextureUnit(0, texture_gbuffer_position);
	glBindTextureUnit(1, texture_gbuffer_normal);
	glBindTextureUnit(2, texture_gbuffer_albedo);
	glBindTextureUnit(3, texture_gbuffer_depth);
	glBindTextureUnit(4, texture_skybox);
	glBindTextureUnit(5, texture_lbuffer_lights);		

	g_Geometry_Empty->Bind();
	g_Program_Final->Use();
	g_Program_Final->SetVertexShaderUniform(kUniformCameraDirection, glm::inverse(glm::mat3(g_Camera_View)));
	g_Program_Final->SetVertexShaderUniform(kUniformFieldOfView, fieldOfView);
	g_Program_Final->SetVertexShaderUniform(kUniformAspectRatio, f32(screenWidth) / f32(screenHeight));
	g_Program_Final->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
}

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

	const auto screenWidth = g_Window_Width / 1;
	const auto screenHeight = g_Window_Height / 1;

	const auto graphicsDevice = new GraphicsDevice();
	//const auto spaceScene = new SpaceScene(*graphicsDevice);
	//spaceScene->Initialize();
	//delete spaceScene;

	auto const texture_cube_diffuse = CreateTexture2DFromfile("./res/textures/T_Default_D.png", STBI_rgb);
	auto const texture_cube_specular = CreateTexture2DFromfile("./res/textures/T_Default_S.png", STBI_grey);
	auto const texture_cube_normal = CreateTexture2DFromfile("./res/textures/T_Default_N.png", STBI_rgb);
	auto const texture_skybox = CreateTextureCubeFromFiles({
		"./res/textures/TC_SkySpace_Xn.png",
		"./res/textures/TC_SkySpace_Xp.png",
		"./res/textures/TC_SkySpace_Yn.png",
		"./res/textures/TC_SkySpace_Yp.png",
		"./res/textures/TC_SkySpace_Zn.png",
		"./res/textures/TC_SkySpace_Zp.png"
	});
	   
	/* framebuffer textures */
	auto const texture_gbuffer_final = CreateTexture2D(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	
	auto const texture_gbuffer_position = CreateTexture2D(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_normal = CreateTexture2D(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_albedo = CreateTexture2D(GL_RGBA16F, GL_RGBA, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_depth = CreateTexture2D(GL_DEPTH_COMPONENT32, GL_DEPTH, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_velocity = CreateTexture2D(GL_RG16F, GL_RG, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_lbuffer_lights = CreateTexture2D(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_motion_blur = CreateTexture2D(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_transition = CreateTexture2D(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);

	g_Framebuffer_Geometry = CreateFramebuffer({ texture_gbuffer_position, texture_gbuffer_normal, texture_gbuffer_albedo, texture_gbuffer_velocity }, texture_gbuffer_depth);
	g_Framebuffer_Final = CreateFramebuffer({ texture_gbuffer_final });
	g_Framebuffer_Motionblur = CreateFramebuffer({ texture_motion_blur });
	g_Framebuffer_Lights = CreateFramebuffer({ texture_lbuffer_lights });
	g_Framebuffer_Transition = CreateFramebuffer({ texture_transition });

	g_Geometry_Empty = Geometry::CreateEmpty();
	g_Geometry_Cube = Geometry::CreateCube(1, 1, 1);
	g_Geometry_Plane = Geometry::CreatePlane(1, 1);
	g_Geometry_Ship = Geometry::CreateFromFile("./res/models/shipA_noWindshield.obj");
	g_Geometry_PointLight = Geometry::CreateFromFilePlain("./res/models/PointLight.obj");
	   	
	auto const asteroidInstances = CreateAsteroidInstances(5000);

	g_Buffer_Asteroids = new Buffer(asteroidInstances);

	auto lights = CreateRandomLights(100); 
	lights.emplace_back(1, glm::vec3(0), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0032f, 0.09f, 32.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	/* shaders */
	g_Program_Final = new Program("../../emptyspace/res/shaders/main.vert.glsl", "../../emptyspace/res/shaders/main.frag.glsl");
	g_Program_GBuffer = new Program("../../emptyspace/res/shaders/gbuffer.vert.glsl", "../../emptyspace/res/shaders/gbuffer.frag.glsl");
	g_Program_MotionBlur = new Program("../../emptyspace/res/shaders/motionblur.vert.glsl", "../../emptyspace/res/shaders/motionblur.frag.glsl");
	g_Program_Light = new Program("../../emptyspace/res/shaders/light.vert.glsl", "../../emptyspace/res/shaders/light.frag.glsl");
	g_Program_Quad = new Program("../../emptyspace/res/shaders/quad.vert.glsl", "../../emptyspace/res/shaders/quad.frag.glsl");

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
	auto const cameraProjection = glm::perspective(fieldOfView, f32(g_Window_Width) / f32(g_Window_Height), 0.1f, 1000.0f);
	
	g_Program_GBuffer->SetVertexShaderUniform(kUniformProjection, cameraProjection);

	// SCENE SETUP BEGIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<SceneObject> objects =
	{
		SceneObject(Shape::Quad),
		SceneObject(Shape::Cube),
	    SceneObject(Shape::Ship),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::Cube),
		SceneObject(Shape::CubeInstanced),
	};

	// SCENE SETUP END //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	auto t1 = glfwGetTime();

	auto deltaTimeAverage = 0.0f;
	auto deltaTimeAverageSquared = 0.0f;

	auto framesToAverage = 100;
	auto frameCounter = 0;

	auto visibleLights = 0;

	glfwSwapInterval(g_Enable_VSync ? 1 : 0);

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

			char str[128];
			sprintf_s(str, "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames, %d visible lights, %.3f", deltaTimeAverage * 1000.0f,
				1000.0f * deltaTimeStandardError, 1.0f / deltaTimeAverage, framesToAverage, visibleLights, g_Transition_Factor);
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

		///////////////////////// SCENE UPDATE BEGIN /////////////////////////
		/* Cube orbit */
		static auto cubeSpeed = 0.125f;
		static auto orbitProgression = 0.0f;
		auto const orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::quat q = glm::rotate(glm::mat4(1.0f), cameraDirection.y, glm::vec3(0.0f, 1.0f, 0.0f));

		objects[0].ModelViewProjection = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
		objects[1].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter) * glm::rotate(glm::mat4(1.0f), orbitProgression * cubeSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::quat r = glm::conjugate(glm::toQuat(glm::lookAt(cameraPosition, cameraPosition - cameraDirection, glm::vec3(0, 1, 0))));

		auto shipModel = glm::translate(glm::mat4(1.0f), cameraPosition + 0.25f * cameraDirection + glm::vec3(0.25f, -0.5f, 0.0f));

		auto angle = glm::atan(cameraDirection.x, cameraDirection.z);
		glm::quat shipQuat = { 0.0f, 1 * glm::sin(angle / 2.0f), 0.0f, glm::cos(angle / 2.0f) };
		shipModel *= glm::toMat4(r);//glm::rotate(shipModel, cameraDirection.x, glm::vec3(0.0f, 1.0f, 0.0f));
		objects[2].ModelViewProjection = shipModel;// *glm::translate(glm::mat4(1.0f), cameraPosition + 2.0f * cameraDirection);

		const auto objectCount = objects.size();
		for (std::size_t i = 3; i < objectCount; i++)
		{
			auto const orbitAmount = (orbitProgression * cubeSpeed + f32(i) * 90.0f * glm::pi<f32>() / 180.0f);
			auto const orbitPosition = OrbitAxis(orbitAmount, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(2.0f, 0.0f, 2.0f)) + glm::vec3(0.0f, 0.0f, 0.0f);
			objects[i].ModelViewProjection = glm::translate(glm::mat4(1.0f), orbitCenter + orbitPosition) * glm::rotate(glm::mat4(1.0f), orbitAmount * 7.0f, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		orbitProgression += 0.1f;

		///////////////////////// SCENE UPDATE END /////////////////////////

		g_Frustum.CalculateFrustum(cameraProjection, g_Camera_View);
		g_Program_GBuffer->SetVertexShaderUniform(kUniformView, g_Camera_View);

		/* g-buffer pass ================================================================================================== begin */

		glViewport(0, 0, screenWidth, screenHeight);
		
		auto constexpr depthClearValue = 1.0f;
		glClearNamedFramebufferfv(g_Framebuffer_Geometry, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(g_Framebuffer_Geometry, GL_COLOR, 1, glm::value_ptr(glm::vec3(0.0f)));
		glClearNamedFramebufferfv(g_Framebuffer_Geometry, GL_COLOR, 2, glm::value_ptr(glm::vec4(0.0f)));
		glClearNamedFramebufferfv(g_Framebuffer_Geometry, GL_COLOR, 3, glm::value_ptr(glm::vec2(0.0f)));
		glClearNamedFramebufferfv(g_Framebuffer_Geometry, GL_DEPTH, 0, &depthClearValue);

		glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer_Geometry);

		glBindTextureUnit(0, texture_cube_diffuse);
		glBindTextureUnit(1, texture_cube_specular);
		glBindTextureUnit(2, texture_cube_normal);

		g_Program_GBuffer->Use();
		
		///////////////////////// SCENE RENDER BEGIN /////////////////////////
		//for (auto& object : g_Scene_Current->Objects())
		for (auto& object : objects)
		{
			switch (object.ObjectShape)
			{
			    case Shape::Cube: g_Geometry_Cube->Bind(); break;
			    case Shape::CubeInstanced:
				{
					g_Geometry_Cube->Bind();
					g_Buffer_Asteroids->BindAsStorageBuffer();
					object.ExcludeFromMotionBlur = true;
					break;
				}
				case Shape::Ship: g_Geometry_Ship->Bind(); break;
			    case Shape::Quad: g_Geometry_Plane->Bind(); break;
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
			    case Shape::Cube: g_Geometry_Cube->DrawElements(); break;
			    case Shape::CubeInstanced: glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr, asteroidInstances.size()); break;
			    case Shape::Quad: g_Geometry_Plane->DrawElements(); break;
			    case Shape::Ship: g_Geometry_Ship->DrawArrays(); break;
			}
		}
		/* g-buffer pass ================================================================================================== end */
		/* lights ======================================================================================================= begin */

		RenderLights(texture_gbuffer_position, texture_gbuffer_normal, texture_gbuffer_depth, lights, cameraProjection, cameraPosition, cameraDirection, visibleLights);

		/* lights ================================================================================ end */

		/* resolve gbuffer ===================================================================== begin */
		RenderGBuffer(texture_skybox, texture_gbuffer_position, texture_gbuffer_normal, texture_gbuffer_albedo,
		              texture_gbuffer_depth, texture_lbuffer_lights, screenWidth, screenHeight, fieldOfView);
		/* resolve gbuffer ======================================================================= end */

		//if (g_Enable_TransitionEffect)
		{
			/* ============== TRANSITION EFFECT =================== */
			graphicsDevice->ClearFramebuffer(g_Framebuffer_Transition, glm::vec3(0.0f), false);
			//glClearNamedFramebufferfv(g_Framebuffer_Transition, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
			glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer_Transition);

			glBindTextureUnit(0, texture_gbuffer_final);

			g_Program_Quad->Use();
			g_Program_Quad->SetFragmentShaderUniform(0, g_Transition_Factor);
			
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
		}
		/* ============== TRANSITION EFFECT =================== */

		if (g_Enable_MotionBlur)
		{
			/* motion blur ========================================================================= begin */
			glClearNamedFramebufferfv(g_Framebuffer_Motionblur, GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f)));
			glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer_Motionblur);

			glBindTextureUnit(0, texture_transition);
			glBindTextureUnit(1, texture_gbuffer_velocity);

			g_Geometry_Empty->Bind();
			g_Program_MotionBlur->Use();
			g_Program_MotionBlur->SetFragmentShaderUniform(kUniformBlurBias, 4.0f);
			g_Program_MotionBlur->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));

			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
			/* motion blur =========================================================================== end */
		}
		
		/* final output */
		glViewport(0, 0, screenWidth, screenHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBlitNamedFramebuffer(g_Enable_MotionBlur 
			? g_Framebuffer_Motionblur 
			: g_Enable_TransitionEffect 
			    ? g_Framebuffer_Transition
			    : g_Framebuffer_Final, 0, 0, 0, screenWidth, screenHeight, 0, 0, g_Window_Width, g_Window_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glFinish();
		glfwSwapBuffers(g_Window);
	}

	Cleanup();

	return 0;
}
