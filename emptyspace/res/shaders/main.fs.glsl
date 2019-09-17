#version 450

layout (location = 0) out vec4 fs_color;

layout (binding = 0) uniform sampler2D t_position;
layout (binding = 1) uniform sampler2D t_normal;
layout (binding = 2) uniform sampler2D t_albedo;
layout (binding = 3) uniform sampler2D t_depth;
layout (binding = 4) uniform samplerCube tc_skybox;

layout (location = 0) uniform vec3 u_camera_position;

in in_block
{
	vec2 texcoord;
	vec3 ray;
} i;

vec3 calculate_specular(float strength, vec3 color, vec3 view_pos, vec3 vert_pos, vec3 light_dir, vec3 normal)
{
	vec3 view_dir = normalize(view_pos - vert_pos);
	vec3 ref_dir = reflect(-light_dir, normal);
	
	float spec = pow(max(dot(view_dir, ref_dir), 0.0), 32.0);
	return strength * spec * color;
}

void main()
{
	const vec3 position = texture(t_position, i.texcoord).rgb;
	const vec3 normal = texture(t_normal, i.texcoord).rgb;
	const vec4 albedo_specular = texture(t_albedo, i.texcoord);
	const vec3 albedo = albedo_specular.rgb;
	const float specular = albedo_specular.a;
	const float depth = texture(t_depth, i.texcoord).r;
	
	vec4 final_color = vec4(1.0);
	vec3 ambient_col = vec3(0.2);

	vec3 light_col = vec3(1.0);
	vec3 light_pos = vec3(0.0, 8.0, 0.0);
	vec3 light_dir = normalize(light_pos - position);
	float light_dif = max(dot(normal, light_dir), 0.0);
		
	vec3 light_spec = calculate_specular(specular, light_col, u_camera_position, position, light_dir, normal);

	final_color.xyz = (ambient_col + (light_dif * light_col) + light_spec) * albedo;
	if (depth == 1.0)
	{
		fs_color = texture(tc_skybox, i.ray);
	}
	else
	{
		fs_color = final_color;
	}
}