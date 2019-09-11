#include <emptyspace/types.hpp>
#include <emptyspace/application.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>

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

	auto deltaTimeAverage = 0.0;  // first moment
	auto deltaTimeAverageSquared = 0.0; // second moment

	auto framesToAverage = 100;
	auto frameCounter = 0;
	
	glfwSwapInterval(1);
	
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
			const auto deltaTimeStandardError = sqrt(deltaTimeAverageSquared - deltaTimeAverage * deltaTimeAverage) / sqrt(framesToAverage);

			char str[64];
			sprintf_s(str, "time frame = %.3fms +/- %.4fms, fps = %.1f, %d frames", 1000.0 * deltaTimeAverage, 1000.0 * deltaTimeStandardError, 1.0 / deltaTimeAverage, framesToAverage);
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
}

void Application::Draw(double deltaTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::Initialize()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Application::Update(double deltaTime)
{
	glfwPollEvents();
}