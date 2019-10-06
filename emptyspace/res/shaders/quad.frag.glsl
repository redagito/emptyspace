#version 450

layout(location = 0) out vec4 fs_color;

layout(binding = 0) uniform sampler2D t_final;

layout(location = 0) uniform float u_blend_factor;

in in_block
{
	vec2 uv;
} i;

void main()
{
	vec3 targetColor = vec3(0.0f, 0.0f, 1.0f);
	vec3 finalColor = texture(t_final, i.uv).rgb;

	fs_color = vec4(mix(finalColor, targetColor, u_blend_factor), 1.0f);
}