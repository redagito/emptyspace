
#define NOMINMAX
#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/graphicsdevice.hpp>
#include <emptyspace/graphics/material.hpp>
#include <emptyspace/graphics/program.hpp>
#include <emptyspace/graphics/texturecube.hpp>
#include <emptyspace/graphics/textures.hpp>
#include <emptyspace/graphics/light.hpp>
#include <emptyspace/graphics/framebuffer.hpp>
#include <emptyspace/io/filewatcher.hpp>
#include <emptyspace/math/frustum.hpp>
#include <emptyspace/physics.hpp>
#include <emptyspace/scenes/scenenode.hpp>
#include <emptyspace/scenes/spacescene.hpp>
#include <emptyspace/types.hpp>
#include <emptyspace/camera.hpp>

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
Program* g_Program_Emission{ nullptr };

Geometry* g_Geometry_Empty{ nullptr };
Geometry* g_Geometry_Cube{ nullptr };
Geometry* g_Geometry_Plane{ nullptr };
Geometry* g_Geometry_Ship{ nullptr };
Geometry* g_Geometry_PointLight{ nullptr };

Framebuffer* g_Framebuffer_Geometry{ };
Framebuffer* g_Framebuffer_Emission{ };
Framebuffer* g_Framebuffer_Final{ };
Framebuffer* g_Framebuffer_Motionblur{ };
Framebuffer* g_Framebuffer_Lights{ };
Framebuffer* g_Framebuffer_Transition{ };

TextureCube* g_Texture_Skybox{ };

std::vector<Material*> g_Materials;
std::vector<Scene*> g_Scenes;
Scene* g_Scene_Current{ nullptr };

Frustum g_Frustum;

bool g_Enable_MotionBlur{ true };
bool g_Enable_VSync{ true };

bool g_Enable_TransitionEffect{ false };
glm::vec4 g_Transition_Factor{ 0.0f, 0.0f, 0.0f, 0.0f };

inline float Lerp(const f32 a, const f32 b, const f32 f)
{
	return a + f * (b - a);
}

void Cleanup()
{
	delete g_Program_GBuffer;
	delete g_Program_Final;
	delete g_Program_MotionBlur;
	delete g_Program_Light;
	delete g_Program_Quad;
	delete g_Program_Emission;

	delete g_Geometry_Cube;
	delete g_Geometry_Plane;
	delete g_Geometry_Empty;
	delete g_Geometry_Ship;
	delete g_Geometry_PointLight;

	delete g_Framebuffer_Geometry;
	delete g_Framebuffer_Final;
	delete g_Framebuffer_Motionblur;
	delete g_Framebuffer_Lights;
	delete g_Framebuffer_Transition;
	delete g_Framebuffer_Emission;

	delete g_Texture_Skybox;

	for (auto material : g_Materials)
	{
		delete material;
	}
	for (auto scene : g_Scenes)
	{
		delete scene;
	}

	delete g_PhysicsScene;
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
}

void InitializePhysics()
{
	g_PhysicsScene = new PhysicsScene();
}

void RenderLights(const Texture& gbufferPosition, 
	const Texture& gbufferNormal,
	const Texture& gbufferDepth,
	glm::mat4 const cameraProjection, 
	const glm::vec3& cameraPosition, 
	const glm::vec3& cameraDirection, 
	int& visibleLights)
{
	auto constexpr depthClearValue = 1.0f;
	g_Framebuffer_Lights->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	g_Framebuffer_Lights->ClearDepth(depthClearValue);
	g_Framebuffer_Lights->Bind();
	
	glBindTextureUnit(0, gbufferPosition.Id());
	glBindTextureUnit(1, gbufferNormal.Id());
	glBindTextureUnit(2, gbufferDepth.Id());
		
	g_Program_Light->Use();
	g_Geometry_PointLight->Bind();
	glCullFace(GL_FRONT);
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	auto lightIndex = 0;
	visibleLights = 0;

	auto lights = g_Scene_Current->Lights();
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

void RenderGBuffer(const TextureCube& texture_skybox, 
	const Texture& texture_gbuffer_position,
	const Texture& texture_gbuffer_normal,
	const Texture& texture_gbuffer_albedo,
	const Texture& texture_gbuffer_depth,
	const Texture& texture_lbuffer_lights,
	const Texture& texture_emission,
	const int screenWidth, 
	const int screenHeight, 
	const float fieldOfView)
{
	auto constexpr kUniformCameraDirection = 0;
	auto constexpr kUniformFieldOfView = 1;
	auto constexpr kUniformAspectRatio = 2;
	auto constexpr kUniformUvsDiff = 3;
	auto constexpr depthClearValue = 1.0f;

	g_Framebuffer_Final->Clear(0, glm::value_ptr(glm::vec3(1.0f)));
	g_Framebuffer_Final->ClearDepth(depthClearValue);
	g_Framebuffer_Final->Bind();

	glBindTextureUnit(0, texture_gbuffer_position.Id());
	glBindTextureUnit(1, texture_gbuffer_normal.Id());
	glBindTextureUnit(2, texture_gbuffer_albedo.Id());
	glBindTextureUnit(3, texture_gbuffer_depth.Id());
	glBindTextureUnit(4, texture_skybox.Id());
	glBindTextureUnit(5, texture_lbuffer_lights.Id());
	glBindTextureUnit(6, texture_emission.Id());

	g_Geometry_Empty->Bind();
	g_Program_Final->Use();
	g_Program_Final->SetVertexShaderUniform(kUniformCameraDirection, glm::inverse(glm::mat3(g_Camera_View)));
	g_Program_Final->SetVertexShaderUniform(kUniformFieldOfView, fieldOfView);
	g_Program_Final->SetVertexShaderUniform(kUniformAspectRatio, f32(screenWidth) / f32(screenHeight));
	g_Program_Final->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
}

void RenderEmission(const Texture& texture_light, const Texture& texture_emission)
{
	g_Framebuffer_Emission->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
	g_Framebuffer_Emission->Bind();

	glBindTextureUnit(0, texture_light.Id());
	glBindTextureUnit(1, texture_emission.Id());

	g_Geometry_Empty->Bind();
	g_Program_Emission->Use();
	g_Program_Emission->SetFragmentShaderUniform(0, 0.7f);

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
	g_Scene_Current = new SpaceScene(*graphicsDevice);

	auto const texture_gbuffer_final = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	
	auto const texture_gbuffer_position = graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_normal = graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_albedo = graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGBA, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_depth = graphicsDevice->CreateTexture(GL_DEPTH_COMPONENT32, GL_DEPTH, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_gbuffer_velocity = graphicsDevice->CreateTexture(GL_RG16F, GL_RG, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_lbuffer = graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_motion_blur = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_transition = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, screenWidth, screenHeight, nullptr, GL_NEAREST);
	auto const texture_emission = graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGBA, screenWidth, screenHeight, nullptr, GL_NEAREST);

	g_Framebuffer_Geometry = new Framebuffer({ texture_gbuffer_position, texture_gbuffer_normal, texture_gbuffer_albedo, texture_gbuffer_velocity, texture_emission }, texture_gbuffer_depth);
	g_Framebuffer_Final = new Framebuffer({ texture_gbuffer_final });
	g_Framebuffer_Emission = new Framebuffer({ texture_emission });
	g_Framebuffer_Motionblur = new Framebuffer({ texture_motion_blur });
	g_Framebuffer_Lights = new Framebuffer({ texture_lbuffer });
	g_Framebuffer_Transition = new Framebuffer({ texture_transition });

	const std::string_view labelFramebufferGeometry("FB-Geometry");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Geometry->Id(), labelFramebufferGeometry.length(), labelFramebufferGeometry.data());
	
	const std::string_view labelFramebufferFinal("FB-Final");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Final->Id(), labelFramebufferFinal.length(), labelFramebufferFinal.data());
	
	const std::string_view labelFramebufferEmission("FB-Emission");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Emission->Id(), labelFramebufferEmission.length(), labelFramebufferEmission.data());
	
	const std::string_view labelFramebufferMotionblur("FB-Motionblur");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Motionblur->Id(), labelFramebufferMotionblur.length(), labelFramebufferMotionblur.data());
	
	const std::string_view labelFramebufferLights("FB-Lights");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Lights->Id(), labelFramebufferLights.length(), labelFramebufferLights.data());
	
	const std::string_view labelFramebufferTransition("FB-Transition");
	glObjectLabel(GL_FRAMEBUFFER, g_Framebuffer_Transition->Id(), labelFramebufferTransition.length(), labelFramebufferTransition.data());
	
	g_Texture_Skybox = TextureCube::FromFiles({
	"./res/textures/TC_SkySpace_Xn.png",
	"./res/textures/TC_SkySpace_Xp.png",
	"./res/textures/TC_SkySpace_Yn.png",
	"./res/textures/TC_SkySpace_Yp.png",
	"./res/textures/TC_SkySpace_Zn.png",
	"./res/textures/TC_SkySpace_Zp.png"
		});

	g_Geometry_Empty = Geometry::CreateEmpty();
	g_Geometry_Cube = Geometry::CreateCube(1, 1, 1);
	g_Geometry_Plane = Geometry::CreatePlane(1, 1);
	g_Geometry_Ship = Geometry::CreateFromFile("./res/models/shipA_noWindshield.obj");
	g_Geometry_PointLight = Geometry::CreateFromFilePlain("./res/models/PointLight.obj");
	   	
	/* shaders */
	g_Program_Final = new Program("../../emptyspace/res/shaders/main.vert.glsl", "../../emptyspace/res/shaders/main.frag.glsl");
	g_Program_GBuffer = new Program("../../emptyspace/res/shaders/gbuffer.vert.glsl", "../../emptyspace/res/shaders/gbuffer.frag.glsl");
	g_Program_MotionBlur = new Program("../../emptyspace/res/shaders/motionblur.vert.glsl", "../../emptyspace/res/shaders/motionblur.frag.glsl");
	g_Program_Light = new Program("../../emptyspace/res/shaders/light.vert.glsl", "../../emptyspace/res/shaders/light.frag.glsl");
	g_Program_Quad = new Program("../../emptyspace/res/shaders/quad.vert.glsl", "../../emptyspace/res/shaders/quad.frag.glsl");
	g_Program_Emission = new Program("../../emptyspace/res/shaders/emission.vert.glsl", "../../emptyspace/res/shaders/emission.frag.glsl");

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

	//std::vector<SceneObject> objects =
	//{
	//	SceneObject(Shape::Quad),
	//	SceneObject(Shape::Cube),
	//    SceneObject(Shape::Ship),
	//	SceneObject(Shape::Cube),
	//	SceneObject(Shape::Cube),
	//	SceneObject(Shape::Cube),
	//	SceneObject(Shape::Cube),
	//	SceneObject(Shape::CubeInstanced),
	//};
	g_Scene_Current->Initialize();

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

	Camera* camera = new Camera();

	glEnable(GL_SCISSOR_TEST);

	glViewport(0, 0, screenWidth, screenHeight);
	glScissor(0, 0, screenWidth, screenHeight);

	Material* currentMaterial = nullptr;
	
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

		camera->Position = cameraPosition;
		camera->Direction = cameraDirection;

		///////////////////////// SCENE UPDATE BEGIN /////////////////////////

		g_Scene_Current->Update(deltaTime, *camera);

		///////////////////////// SCENE UPDATE END /////////////////////////

		g_Frustum.CalculateFrustum(cameraProjection, g_Camera_View);
		g_Program_GBuffer->SetVertexShaderUniform(kUniformView, g_Camera_View);

		/* g-buffer pass ================================================================================================== begin */

		glViewport(0, 0, screenWidth, screenHeight);
		
		auto constexpr depthClearValue = 1.0f;
		g_Framebuffer_Geometry->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
		g_Framebuffer_Geometry->Clear(1, glm::value_ptr(glm::vec3(0.0f)));
		g_Framebuffer_Geometry->Clear(2, glm::value_ptr(glm::vec4(0.0f)));
		g_Framebuffer_Geometry->Clear(3, glm::value_ptr(glm::vec2(0.0f)));
		g_Framebuffer_Geometry->Clear(4, glm::value_ptr(glm::vec4(0.0f)));
		g_Framebuffer_Geometry->ClearDepth(depthClearValue);
		g_Framebuffer_Geometry->Bind();

		g_Program_GBuffer->Use();

		//std::sort(g_Scene_Current->Objects().begin(), g_Scene_Current->Objects().end());
		
		///////////////////////// SCENE RENDER BEGIN /////////////////////////
		for (auto& object : g_Scene_Current->Objects())
		{
			object->ObjectMaterial->Apply();
			switch (object->ObjectShape)
			{
			    case Shape::Cube: g_Geometry_Cube->Bind(); break;
			    case Shape::CubeInstanced:
				{
					g_Geometry_Cube->Bind();
					//g_Buffer_Asteroids->BindAsStorageBuffer();
					reinterpret_cast<SpaceScene*>(g_Scene_Current)->GetAsteroidInstanceBuffer()->BindAsStorageBuffer();
					object->ExcludeFromMotionBlur = true;
					break;
				}
				case Shape::Ship: g_Geometry_Ship->Bind(); break;
			    case Shape::Quad: g_Geometry_Plane->Bind(); break;
			}

			auto const currentModelViewProjection = cameraProjection * g_Camera_View * object->ModelViewProjection;

			g_Program_GBuffer->SetVertexShaderUniform(kUniformModel, object->ModelViewProjection);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformModelViewProjection, currentModelViewProjection);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformModelViewProjectionInverse, object->ModelViewProjectionPrevious);
			g_Program_GBuffer->SetVertexShaderUniform(kUniformBlurExcept, object->ExcludeFromMotionBlur);
			g_Program_GBuffer->SetVertexShaderUniform(6, object->ObjectShape == Shape::CubeInstanced);

			object->ModelViewProjectionPrevious = currentModelViewProjection;

			switch (object->ObjectShape)
			{
			    case Shape::Cube: g_Geometry_Cube->DrawElements(); break;
			    case Shape::CubeInstanced: glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr, 5000); break;
			    case Shape::Quad: g_Geometry_Plane->DrawElements(); break;
			    case Shape::Ship: g_Geometry_Ship->DrawArrays(); break;
			}
		}
		/* g-buffer pass ================================================================================================== end */
		/* lights ======================================================================================================= begin */

		RenderLights(*texture_gbuffer_position, *texture_gbuffer_normal, *texture_gbuffer_depth, cameraProjection, cameraPosition, cameraDirection, visibleLights);
		RenderEmission(*texture_lbuffer, *texture_emission);
		/* lights ================================================================================ end */

		/* resolve gbuffer ===================================================================== begin */
		RenderGBuffer(*g_Texture_Skybox, *texture_gbuffer_position, *texture_gbuffer_normal, *texture_gbuffer_albedo,
		              *texture_gbuffer_depth, *texture_lbuffer, *texture_emission, screenWidth, screenHeight, fieldOfView);
		/* resolve gbuffer ======================================================================= end */

		if (g_Transition_Factor.w > 0.0f)
		{
			/* ============== TRANSITION EFFECT =================== */
			g_Framebuffer_Transition->Clear(0, glm::value_ptr(glm::vec3(0.0)));
			g_Framebuffer_Transition->Bind();

			glBindTextureUnit(0, texture_gbuffer_final->Id());

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
			g_Framebuffer_Motionblur->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
			g_Framebuffer_Motionblur->Bind();

			glBindTextureUnit(0, g_Transition_Factor.w > 0.0f ? texture_transition->Id() : texture_gbuffer_final->Id());
			glBindTextureUnit(1, texture_gbuffer_velocity->Id());

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
			? g_Framebuffer_Motionblur->Id()
			: g_Enable_TransitionEffect
			    ? g_Framebuffer_Transition->Id()
			    : g_Framebuffer_Final->Id(), 0, 0, 0, screenWidth, screenHeight, 0, 0, g_Window_Width, g_Window_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glFinish();
		glfwSwapBuffers(g_Window);
	}

	Cleanup();

	return 0;
}
