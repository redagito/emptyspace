#pragma once

struct GLFWwindow;

class Geometry;

class Application final
{
public:
	Application();
	virtual ~Application();
	void Run();
private:
	void Cleanup();
	void Draw(double deltaTime);
	void HandleInput(double deltaTime);
	void Initialize();
	void Update(double deltaTime);
	
	GLFWwindow* _window;
	int _windowHeight;
	int _windowWidth;

	Geometry* _planeGeometry;
};