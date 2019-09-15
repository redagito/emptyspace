#include <emptyspace/types.hpp>
#include <emptyspace/graphics/attributeformat.hpp>
#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/instancebuffer.hpp>
#include <emptyspace/graphics/vertexpositioncolornormaluv.hpp>
#include <emptyspace/graphics/shader.hpp>
#include <emptyspace/math/camera.hpp>
#include <emptyspace/physics.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

u32 g_WindowWidth = {1920};
u32 g_WindowHeight = {1080};
GLFWwindow* g_Window = nullptr;

Camera* g_Camera = nullptr;
PhysicsScene* g_PhysicsScene = nullptr;

static f64 g_MousePosXOld = {};
static f64 g_MousePosYOld = {};

glm::mat4 g_CameraProjectionMatrix = glm::mat4(1.0f);

Geometry* g_GeometryPlane = nullptr;
Geometry* g_GeometryCube = nullptr;
Shader* g_BasicShader = nullptr;
Shader* g_BasicShaderInstanced = nullptr;
InstanceBuffer* g_AsteroidInstanceBuffer = nullptr;

static float g_CubeAngle = 0.0f;

void CleanUp()
{
	delete g_GeometryCube;
	delete g_GeometryPlane;
	delete g_AsteroidInstanceBuffer;
	delete g_BasicShader;
	delete g_BasicShaderInstanced;
	
	
	delete g_Camera;
	delete g_PhysicsScene;

	glfwDestroyWindow(g_Window);
	glfwTerminate();
}

void Draw(const float deltaTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_BasicShader->Use();

	g_BasicShader->SetValue("u_model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
	g_GeometryPlane->Bind();
	g_GeometryPlane->DrawElements();

	g_CubeAngle += static_cast<float>(deltaTime);
	const auto rotationMatrix = glm::rotate(glm::mat4(1.0f), g_CubeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	g_BasicShader->SetValue("u_model", glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix);
	g_GeometryCube->Bind();
	g_GeometryCube->DrawElements();

	g_BasicShaderInstanced->Use();
	g_GeometryCube->Bind();
	g_AsteroidInstanceBuffer->Bind();
	g_GeometryCube->DrawElementsInstanced(1000);
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
		auto displacement = (rand() % static_cast<int>(2.0f * offset * 100)) / 100.0f - offset;

		const auto x = sin(angle) * radius + displacement;
		displacement = (rand() % static_cast<int>(2.0f * offset * 100)) / 100.0f - offset;

		const auto y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % static_cast<int>(2.0f * offset * 100)) / 100.0f - offset;

		const auto z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		const auto scale = (rand() % 60) / 100.0f + 0.05f;
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

	g_Camera->MovementSpeed = SPEED;
	if (glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		g_Camera->MovementSpeed = SPEED * 40;
	}

	if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Forward);
		g_Camera->ProcessKeyboard(CameraMovement::Forward, deltaTime);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Backward);
		g_Camera->ProcessKeyboard(CameraMovement::Backward, deltaTime);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Left);
		g_Camera->ProcessKeyboard(CameraMovement::Left, deltaTime);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Right);
		g_Camera->ProcessKeyboard(CameraMovement::Right, deltaTime);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Up);
		g_Camera->ProcessKeyboard(CameraMovement::Up, deltaTime);
	}

	if (glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		g_PhysicsScene->Boost(Direction::Down);
		g_Camera->ProcessKeyboard(CameraMovement::Down, deltaTime);
	}

}

void InitializeOpenGL()
{
	glViewport(0, 0, g_WindowWidth, g_WindowHeight);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
}

void InitializePhysics()
{
	g_PhysicsScene = new PhysicsScene();
}

void InitializeGame()
{
	g_Camera = new Camera(glm::vec3{ 0, 0, 5 });
	g_Camera->MouseSensitivity = 0.1f;
	g_CameraProjectionMatrix = glm::perspective(glm::pi<f32>() / 4.0f,
		static_cast<float>(g_WindowWidth) / static_cast<float>(g_WindowHeight), 0.1f, 512.0f);

	const std::vector<VertexPositionColorNormalUv> planeVertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
	};

	const std::vector<VertexPositionColorNormalUv> cubeVertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
	};

	const std::vector<u8> planeIndices =
	{
		0, 1, 2, 2, 3, 0,
	};

	const std::vector<u8> cubeIndices =
	{
		0,   1,  2,  2,  3,  0,
		4,   5,  6,  6,  7,  4,
		8,   9, 10, 10, 11,  8,

		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	const std::vector<AttributeFormat> geometryVertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Uv))
	};

	const auto asteroidInstances = GameCreateAsteroidInstances(10000);

	g_AsteroidInstanceBuffer = new InstanceBuffer();
	g_AsteroidInstanceBuffer->UpdateBuffer(asteroidInstances);

	g_GeometryPlane = new Geometry(planeVertices, planeIndices, geometryVertexFormat);
	g_GeometryCube = new Geometry(cubeVertices, cubeIndices, geometryVertexFormat);

	g_BasicShader = new Shader("res/shaders/basic.vs.glsl", "res/shaders/basic.ps.glsl");
	g_BasicShaderInstanced = new Shader("res/shaders/basic_instanced.vs.glsl", "res/shaders/basic.ps.glsl");
}

void Update(const float deltaTime)
{
	glfwPollEvents();
	HandleInput(deltaTime);

	g_PhysicsScene->Step(deltaTime);
	g_Camera->Position = g_PhysicsScene->Fetch();

	g_BasicShader->Use();
	g_BasicShader->SetValue("u_model", glm::mat4x4(1.0f));
	g_BasicShader->SetValue("u_view", g_Camera->GetViewMatrix());
	g_BasicShader->SetValue("u_projection", g_CameraProjectionMatrix);
	  
	g_BasicShaderInstanced->Use();
	g_BasicShaderInstanced->SetValue("u_view", g_Camera->GetViewMatrix());
	g_BasicShaderInstanced->SetValue("u_projection", g_CameraProjectionMatrix);
}


void WindowOnFramebufferResized(GLFWwindow* window, const int width, const int height)
{
	glViewport(0, 0, width, height);
}

void WindowOnMouseMove(GLFWwindow* window, const double xpos, const double ypos)
{
	if (g_Camera != nullptr)
	{
		g_Camera->ProcessMouseMovement(f32(xpos - g_MousePosXOld), f32(g_MousePosYOld - ypos));
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
	s32 screenWidth;
	s32 screenHeight;
	s32 workingAreaXPos;
	s32 workingAreaYPos;

	glfwGetMonitorWorkarea(primaryMonitor, &workingAreaXPos, &workingAreaYPos, &screenWidth, &screenHeight);

	g_WindowWidth = u32(0.8f * screenWidth);
	g_WindowHeight = u32(0.8f * screenHeight);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	g_Window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "emptyspace", nullptr, nullptr);
	if (g_Window == nullptr)
	{
		std::cerr << "GLFW: Unable to create a window.\n";
		glfwTerminate();
		return 1;
	}

	glfwSetFramebufferSizeCallback(g_Window, WindowOnFramebufferResized);
	glfwSetCursorPosCallback(g_Window, WindowOnMouseMove);
	   	
	glfwSetWindowPos(g_Window, screenWidth / 2 - (g_WindowWidth / 2), screenHeight / 2 - (g_WindowHeight / 2));

	glfwMakeContextCurrent(g_Window);
	glfwSwapInterval(1);
	if (!gladLoadGL())
	{
		std::cerr << "GLAD: Unable to initialize.\n";
		glfwDestroyWindow(g_Window);
		glfwTerminate();
	}

	std::clog << "GL VENDOR: " << glGetString(GL_VENDOR) << '\n';
	std::clog << "GL VERSION: " << glGetString(GL_VERSION) << '\n';
	std::clog << "GL GLSL VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(g_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	InitializeOpenGL();
	InitializePhysics();
	InitializeGame();

	auto t1 = glfwGetTime();

	auto deltaTimeAverage = 0.0f;
	auto deltaTimeAverageSquared = 0.0f;

	auto framesToAverage = 100;
	auto frameCounter = 0;

	glfwSwapInterval(0);

	g_MousePosXOld = g_WindowWidth / 2.0f;
	g_MousePosYOld = g_WindowHeight / 2.0f;
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
		Draw(deltaTime);

		glfwSwapBuffers(g_Window);
	}

	CleanUp();

	return 0;
}