#version 450

out gl_PerVertex{ vec4 gl_Position; };

layout (location = 0) in vec3 in_position;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

out out_block
{
	vec2 texcoord;
} o;

void main()
{
	const vec4 mpos = (u_projection * u_view * u_model * vec4(in_position, 1.0));
	gl_Position = mpos;
}