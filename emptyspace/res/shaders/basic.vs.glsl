#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;

out vec3 ps_color;
out vec2 ps_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(in_position, 1.0f);
	ps_color = in_color;
	ps_uv = in_uv;
}