#version 450

layout (location = 0) out vec4 fs_color;

layout (binding = 0) uniform sampler2D t_position;
layout (binding = 1) uniform sampler2D t_normal;
layout (binding = 2) uniform sampler2D t_albedo;
layout (binding = 3) uniform sampler2D t_depth;
layout (binding = 4) uniform samplerCube tc_skybox;
layout (binding = 5) uniform sampler2D t_lights;

layout (location = 0) uniform vec3 u_camera_position;

in in_block
{
	vec2 texcoord;
	vec3 ray;
} i;

void main()
{
	const vec3 position = texture(t_position, i.texcoord).rgb;
	const vec3 normal = texture(t_normal, i.texcoord).rgb;
	const vec4 albedo_specular = texture(t_albedo, i.texcoord);
	const vec3 albedo = albedo_specular.rgb;
	const float specular = albedo_specular.a;
	const float depth = texture(t_depth, i.texcoord).r;
	
	vec4 fragmentColor = vec4(1.0);
	vec3 ambientColor = vec3(0.2);

	fragmentColor.xyz = texture(t_lights, i.texcoord).rgb * albedo;

	if (depth == 1.0)
	{
		fs_color = texture(tc_skybox, i.ray);
	}
	else
	{
		fs_color = fragmentColor;
	}
}