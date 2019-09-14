#include <emptyspace/types.hpp>
#include <emptyspace/application.hpp>
#include <emptyspace/graphics/attributeformat.hpp>
#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/vertexpositioncolornormaluv.hpp>
#include <emptyspace/graphics/shader.hpp>
#include <emptyspace/math/camera.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>

void OnFramebufferResized(GLFWwindow* window, int width, int height);
void OnMouseMove(GLFWwindow* window, double xpos, double ypos);

static double oldxpos;
static double oldypos;

#if _DEBUG
void APIENTRY DebugCallback(u32 source, u32 type, u32 id, u32 severity, s32 length, const char* message,
                            const void* userParam)
{
	std::ostringstream str;
	str << "---------------------opengl-callback-start------------\n";
	str << "message: " << message << '\n';
	str << "type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: str << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: str << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: str << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY: str << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE: str << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER: str << "OTHER";
		break;
	}
	str << '\n';
	str << "id: " << id << '\n';
	str << "severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW: str << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: str << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH: str << "HIGH";
		break;
	}
	str << "\n---------------------opengl-callback-end--------------\n";

	std::clog << str.str();
}
#endif

Application::Application()
	: _window(nullptr), _windowHeight(1080), _windowWidth(1920)
{
	if (!glfwInit())
	{
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	_window = glfwCreateWindow(_windowWidth, _windowHeight, "emptyspace", nullptr, nullptr);
	if (_window == nullptr)
	{
	}

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	glfwSetFramebufferSizeCallback(_window, OnFramebufferResized);
	glfwSetCursorPosCallback(_window, OnMouseMove);

	const auto primaryMonitor = glfwGetPrimaryMonitor();
	int screenWidth;
	int screenHeight;
	int xpos;
	int ypos;

	glfwGetMonitorWorkarea(primaryMonitor, &xpos, &ypos, &screenWidth, &screenHeight);
	glfwSetWindowPos(_window, screenWidth / 2 - (_windowWidth / 2), screenHeight / 2 - (_windowHeight / 2));

	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);
	if (!gladLoadGL())
	{
	}
}

Application::~Application()
{
	glfwDestroyWindow(_window);
	_window = nullptr;
	glfwTerminate();
}

void Application::Run()
{
	Initialize();

	auto t1 = glfwGetTime();

	auto deltaTimeAverage = 0.0;
	auto deltaTimeAverageSquared = 0.0;

	auto framesToAverage = 100;
	auto frameCounter = 0;

	glfwSwapInterval(0);

	while (!glfwWindowShouldClose(_window))
	{
		const auto t2 = glfwGetTime();
		const auto deltaTime = f32(t2 - t1);
		t1 = t2;

		deltaTimeAverage += deltaTime;
		deltaTimeAverageSquared += deltaTime * deltaTime;
		frameCounter++;

		if (frameCounter == framesToAverage)
		{
			deltaTimeAverage /= framesToAverage;
			deltaTimeAverageSquared /= framesToAverage;
			const auto deltaTimeStandardError = sqrt(deltaTimeAverageSquared - deltaTimeAverage * deltaTimeAverage) /
				sqrt(framesToAverage);

			char str[76];
			sprintf_s(str, "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames", 1000.0 * deltaTimeAverage,
			          1000.0 * deltaTimeStandardError, 1.0 / deltaTimeAverage, framesToAverage);
			glfwSetWindowTitle(_window, str);

			framesToAverage = static_cast<int>(1.0 / deltaTimeAverage);

			deltaTimeAverage = 0.0;
			deltaTimeAverageSquared = 0.0;
			frameCounter = 0;
		}

		Update(deltaTime);
		Draw(deltaTime);

		glfwSwapBuffers(_window);
	}

	Cleanup();
}

void Application::Cleanup() const
{
	delete _planeGeometry;
	delete _cubeGeometry;
}

static float angle = 0.0f;

void Application::Draw(const f32 deltaTime) const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_basicShader->Use();
	_basicShader->SetValue("u_model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
	_planeGeometry->Bind();
	_planeGeometry->Draw();

	angle += static_cast<float>(deltaTime);
	const auto rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	_basicShader->SetValue("u_model", glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix);
	_cubeGeometry->Bind();
	_cubeGeometry->Draw();
}

void Application::HandleInput(const f32 deltaTime) const
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(_window, true);
	}

	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		_camera->ProcessKeyboard(CameraMovement::Forward, deltaTime);
	}
	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		_camera->ProcessKeyboard(CameraMovement::Backward, deltaTime);
	}
	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		_camera->ProcessKeyboard(CameraMovement::Left, deltaTime);
	}
	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		_camera->ProcessKeyboard(CameraMovement::Right, deltaTime);
	}
}

void Application::Initialize()
{
	std::clog << glGetString(GL_VERSION) << '\n';

#if _DEBUG
	if (glDebugMessageCallback)
	{
		std::clog << "registered debug callback\n";
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(DebugCallback, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
	}
	else
	{
		std::clog << "glDebugMessageCallback not available\n";
	}
#endif

	glViewport(0, 0, _windowWidth, _windowHeight);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	const std::vector<VertexPositionColorNormalUv> planeVertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
	};

	const std::vector<VertexPositionColorNormalUv> const cubeVertices =
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

	const std::vector<u8> const cubeIndices =
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

	_planeGeometry = new Geometry(planeVertices, planeIndices, geometryVertexFormat);
	_cubeGeometry = new Geometry(cubeVertices, cubeIndices, geometryVertexFormat);

	_basicShader = new Shader("res/shaders/basic.vs.glsl", "res/shaders/basic.ps.glsl");

	_camera = new Camera(glm::vec3{0, 0, 5});
	_camera->MouseSensitivity = 0.1f;
	_projection = glm::perspective(glm::pi<f32>() / 4.0f,
	                               static_cast<float>(_windowWidth) / static_cast<float>(_windowHeight), 0.1f, 512.0f);

	oldxpos = _windowWidth / 2.0f;
	oldypos = _windowHeight / 2.0f;
	glfwSetCursorPos(_window, oldxpos, oldypos);
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(_window, _camera);
}

void Application::Update(const f32 deltaTime) const
{
	glfwPollEvents();
	HandleInput(deltaTime);

	_basicShader->SetValue("u_model", glm::mat4x4(1.0f));
	_basicShader->SetValue("u_view", _camera->GetViewMatrix());
	_basicShader->SetValue("u_projection", _projection);
}

void OnFramebufferResized(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	const auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (camera != nullptr)
	{
		camera->ProcessMouseMovement(xpos - oldxpos, oldypos - ypos);
		oldxpos = xpos;
		oldypos = ypos;
	}
}
