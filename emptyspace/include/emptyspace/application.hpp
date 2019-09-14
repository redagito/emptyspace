#pragma once
#include <emptyspace/types.hpp>
#include <glm/mat4x4.hpp>

struct GLFWwindow;

class Camera;
class Geometry;
class InstanceBuffer;
class Shader;

class Application final
{
public:
	Application();
	virtual ~Application();
	void Run();
private:
	void Cleanup() const;
	void Draw(const f32 deltaTime) const;
	void HandleInput(const f32 deltaTime) const;
	void Initialize();

	void Update(const f32 deltaTime) const;

	GLFWwindow* _window;
	int _windowHeight;
	int _windowWidth;

	InstanceBuffer* _asteroidInstanceBuffer{};
	Geometry* _planeGeometry{};
	Geometry* _cubeGeometry{};
	Camera* _camera{};
	Shader* _basicShader{};
	glm::mat4x4 _projection{};
};