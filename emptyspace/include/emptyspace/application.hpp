#pragma once
#include <glm/mat4x4.hpp>

struct GLFWwindow;

class Camera;
class Geometry;
class Shader;

class Application final
{
public:
	Application();
	virtual ~Application();
	void Run();
private:
	void Cleanup() const;
	void Draw(const double deltaTime) const;
	void HandleInput(const double deltaTime) const;
	void Initialize();

	void Update(const double deltaTime) const;

	GLFWwindow* _window;
	int _windowHeight;
	int _windowWidth;

	Geometry* _planeGeometry{};
	Geometry* _cubeGeometry{};
	Camera* _camera{};
	Shader* _basicShader{};
	glm::mat4x4 _projection{};
};