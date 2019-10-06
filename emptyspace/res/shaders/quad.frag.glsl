#version 450

layout(location = 0) out vec4 fs_color;

layout(binding = 0) uniform sampler2D t_final;

layout(location = 0) uniform vec4 u_blend_color;

in vec2 texCoord;

void main()
{
	vec3 finalColor = texture(t_final, texCoord).rgb;

	fs_color = vec4(mix(finalColor, u_blend_color.rgb, u_blend_color.w), 1.0f);
}