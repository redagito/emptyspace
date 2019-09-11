#include <emptyspace/types.hpp>
#include <emptyspace/application.hpp>
#include <emptyspace/graphics/geometry.hpp>
#include <emptyspace/graphics/vertexpositioncolornormaluv.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <vector>
#include "emptyspace/graphics/attributeformat.hpp"
#include <iostream>
#include <sstream>

#if _DEBUG
void APIENTRY DebugCallback(u32 source, u32 type, u32 id, u32 severity, s32 length, const char* message, const void* userParam)
{
	std::ostringstream str;
	str << "---------------------opengl-callback-start------------\n";
	str << "message: " << message << '\n';
	str << "type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: str << "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: str << "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: str << "UNDEFINED_BEHAVIOR";	break;
	case GL_DEBUG_TYPE_PORTABILITY: str << "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: str << "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER: str << "OTHER"; break;
	}
	str << '\n';
	str << "id: " << id << '\n';
	str << "severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW: str << "LOW"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: str << "MEDIUM";	break;
	case GL_DEBUG_SEVERITY_HIGH: str << "HIGH";	break;
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
		const auto deltaTime = t2 - t1;
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

void Application::Cleanup()
{
	delete _planeGeometry;
}

void Application::Draw(double deltaTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_planeGeometry->Bind();
	_planeGeometry->Draw();
}

void Application::HandleInput(double deltaTime)
{
}

void Application::Initialize()
{
	std::clog << glGetString(GL_VERSION) << '\n';

#if _DEBUG
	if (glDebugMessageCallback)
	{
		std::clog << "registered opengl debug callback\n";
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
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	const std::vector<VertexPositionColorNormalUv> planeVertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
	};

	const std::vector<u8> planeIndices =
	{
		0, 1, 2, 2, 3, 0,
	};

	const std::vector<AttributeFormat> planeVertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Uv))
	};

	_planeGeometry = new Geometry(planeVertices, planeIndices, planeVertexFormat);
}

void Application::Update(double deltaTime)
{
	glfwPollEvents();
	HandleInput(deltaTime);
}

