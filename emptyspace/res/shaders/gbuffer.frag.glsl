#version 450

in in_block
{
	vec3 position;
	vec3 normal;
	vec2 uv;
	smooth vec4 current_position;
	smooth vec4 previous_position;
} i;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_albedo;
layout(location = 3) out vec2 out_velocity;
layout(location = 4) out vec4 out_emission;

layout(binding = 0) uniform sampler2D t_diffuse;
layout(binding = 1) uniform sampler2D t_specular;
layout(binding = 2) uniform sampler2D t_normal;

void main()
{
	vec3 diffuse = texture(t_diffuse, i.uv).rgb;
	vec3 specular = texture(t_specular, i.uv).rgb;
	vec3 normal = texture(t_normal, i.uv).rgb;

	out_position = i.position;
	out_normal = normalize(cross(i.normal, normal));
	out_albedo.rgb = diffuse;
	out_albedo.a = specular.r;
	out_velocity = ((i.current_position.xy / i.current_position.w) * 0.5 + 0.5) - ((i.previous_position.xy / i.previous_position.w) * 0.5 + 0.5);
	out_emission = vec4(i.position.x, diffuse.g, normal.r, 1.0f) * 1.2f;
}