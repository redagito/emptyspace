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

struct Light
{
    vec3 Position;
	vec3 Color;
	float Attenuation;

	vec4 padding1;
	float padding2;
};

layout(std430, binding = 0) buffer lightBuffer
{
    Light u_lights[];
};

float CalculateDiffuse_L(vec3 FragPos, vec3 Normal, vec3 LightPos)
{
    vec3 lightDir = normalize(LightPos - FragPos);
    float diffuse = max(dot(lightDir, Normal), 0.0);
    return diffuse;
}

float CalculateSpecular_BP(vec3 FragPos, vec3 Normal, vec3 LightPos, vec3 cameraPosition, float exp)
{
    vec3 lightDir = normalize(LightPos - FragPos);
    vec3 viewDir = cameraPosition - FragPos; 
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), exp);
    return spec;
}

float CalculateAttenuation(vec3 FragPos, vec3 LightPos, float Attenuation)
{
    float distance = length(FragPos - LightPos);
    float value = clamp(1.0f - distance / Attenuation, 0.0, 1.0f);
    value = value;
    value = sqrt(value);
    // return 1.0;
    return value;
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
	vec3 ambient_color = vec3(0.2);

	vec3 final_light = vec3(0.0f);
	vec3 diffuse_light = vec3(0.0f);
	vec3 ambient_light = vec3(0.0f);
	vec3 specular_light = vec3(0.0f);
	for (int j = 0; j < 3; j++)
	{
	    float attenuation = CalculateAttenuation(position, u_lights[j].Position.xyz, u_lights[j].Attenuation.r);

		vec3 light_color = u_lights[j].Color.rgb;
	    vec3 light_position = u_lights[j].Position.xyz;
		diffuse_light += attenuation * CalculateDiffuse_L(position, normal, u_lights[j].Position.xyz) * u_lights[j].Color.rgb;
		specular_light += attenuation * CalculateSpecular_BP(position, normal, u_lights[j].Position.xyz, u_camera_position, 8);
	}
	final_color.xyz = (ambient_light + diffuse_light + (specular_light * specular)) * albedo;

	if (depth == 1.0)
	{
		fs_color = texture(tc_skybox, i.ray);
	}
	else
	{
		fs_color = final_color;
	}
}