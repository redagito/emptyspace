#version 430 core

out vec4 fragment_color;

in vec3 ps_color;
in vec2 ps_uv;

void main()
{
	fragment_color = vec4(ps_color.rgb, 1.0f);//mix(texture(texture1, ps_uv), texture(texture2, ps_uv), 0.2);
}