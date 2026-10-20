#define NOMINMAX

#include "camera.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "types.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string_view>

namespace
{
	GLFWwindow* g_Window{ nullptr };
	PhysicsScene* g_PhysicsScene{ nullptr };

	f64 g_MousePosXOld{};
	f64 g_MousePosYOld{};

	void HandleInput(Renderer& renderer)
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
			renderer.SetTransitionEffectEnabled(true);
		}
	}

	void Update(Renderer& renderer, const f32 deltaTime)
	{
		glfwPollEvents();
		HandleInput(renderer);

		g_PhysicsScene->Step(deltaTime);
	}

	void WindowOnFramebufferResized(GLFWwindow* /*window*/, const int width, const int height)
	{
		glViewport(0, 0, width, height);
	}

	void WindowOnMouseMove(GLFWwindow* /*window*/, const double xPos, const double yPos)
	{
		if (g_PhysicsScene != nullptr)
		{
			g_PhysicsScene->Tumble(static_cast<f32>(xPos - g_MousePosXOld),
								   static_cast<f32>(yPos - g_MousePosYOld));
			g_MousePosXOld = xPos;
			g_MousePosYOld = yPos;
		}
	}

	void GetWorkingArea(s32* screenWidth, s32* screenHeight)
	{
		const auto primaryMonitor = glfwGetPrimaryMonitor();

		glfwGetMonitorWorkarea(primaryMonitor, nullptr, nullptr, screenWidth, screenHeight);
	}

	GLFWwindow* CreateMainWindow(
		const s32 screenWidth,
		const s32 screenHeight,
		const s32 windowWidth,
		const s32 windowHeight,
		const std::string_view title)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#if _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

		const auto window = glfwCreateWindow(windowWidth, windowHeight, title.data(), nullptr, nullptr);
		if (window == nullptr)
		{
			glfwTerminate();
			return nullptr;
		}

		glfwSetFramebufferSizeCallback(window, WindowOnFramebufferResized);
		glfwSetCursorPosCallback(window, WindowOnMouseMove);

		glfwSetWindowPos(window, screenWidth / 2 - windowWidth / 2, screenHeight / 2 - windowHeight / 2);
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		return window;
	}

	bool InitializeOpenGL(GLFWwindow* window)
	{
		glfwMakeContextCurrent(window);
		if (!gladLoadGL())
		{
			std::cerr << "GLAD: Unable to initialize.\n";
			glfwDestroyWindow(window);
			glfwTerminate();
			return false;
		}

		return true;
	}
}

int main(int /*argc*/, char** /*argv*/)
{
	if (!glfwInit())
	{
		std::cerr << "GLFW: Unable to initialize.\n";
		return 1;
	}

	s32 screenWidth{};
	s32 screenHeight{};
	GetWorkingArea(&screenWidth, &screenHeight);

	const auto windowWidth = static_cast<s32>(0.8f * screenWidth);
	const auto windowHeight = static_cast<s32>(0.8f * screenHeight);

	g_Window = CreateMainWindow(screenWidth, screenHeight, windowWidth, windowHeight, "emptyspace");
	if (g_Window == nullptr)
	{
		std::cerr << "GLFW: Unable to create a window.\n";
		return 1;
	}

	if (!InitializeOpenGL(g_Window))
	{
		return 1;
	}

	g_PhysicsScene = new PhysicsScene();

	const FrameSize frameSize{ windowWidth, windowHeight };

	{
		Renderer renderer(frameSize);

		auto t1 = glfwGetTime();

		auto deltaTimeAverage = 0.0f;
		auto deltaTimeAverageSquared = 0.0f;

		auto framesToAverage = 100;
		auto frameCounter = 0;

		glfwSwapInterval(1);

		g_MousePosXOld = windowWidth / 2;
		g_MousePosYOld = windowHeight / 2;
		glfwSetCursorPos(g_Window, g_MousePosXOld, g_MousePosYOld);
		glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glViewport(0, 0, frameSize.Width, frameSize.Height);

		while (!glfwWindowShouldClose(g_Window))
		{
			const auto t2 = glfwGetTime();
			const auto deltaTime = static_cast<f32>(t2 - t1);
			t1 = t2;

			deltaTimeAverage += deltaTime;
			deltaTimeAverageSquared += (deltaTime * deltaTime);
			frameCounter++;

			if (frameCounter == framesToAverage)
			{
				deltaTimeAverage /= static_cast<f32>(framesToAverage);
				deltaTimeAverageSquared /= static_cast<f32>(framesToAverage);
				const auto deltaTimeStandardError = sqrt(deltaTimeAverageSquared - deltaTimeAverage * deltaTimeAverage) /
					sqrt(framesToAverage);

				char str[128];
				snprintf(str, sizeof(str), "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames, %d visible lights, %.3f", deltaTimeAverage * 1000.0f,
					1000.0f * deltaTimeStandardError, 1.0f / deltaTimeAverage, framesToAverage, renderer.VisibleLights(), 0.0f);
				glfwSetWindowTitle(g_Window, str);

				framesToAverage = static_cast<int>(1.0f / deltaTimeAverage);

				deltaTimeAverage = 0.0f;
				deltaTimeAverageSquared = 0.0f;
				frameCounter = 0;
			}

			Update(renderer, deltaTime);

			const auto camera = Camera::FromPhysicsScene(*g_PhysicsScene);

			renderer.Render(camera, deltaTime, frameSize);

			glFinish();
			glfwSwapBuffers(g_Window);
		}
	}

	delete g_PhysicsScene;
	glfwDestroyWindow(g_Window);
	glfwTerminate();

	return 0;
}
