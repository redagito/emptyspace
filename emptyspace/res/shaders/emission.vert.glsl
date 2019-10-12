#version 450

out gl_PerVertex{ vec4 gl_Position; };

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

out FragmentData
{
	vec2 Position;
    vec2 Uv;
} fs_color;

void main()
{
	gl_Position = vec4(in_position.xy, 0.0, 1.0);
	fs_color.Position = in_position;
	fs_color.Uv = in_uv;
}