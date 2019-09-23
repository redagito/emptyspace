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

//struct Light
//{
//    vec4 Position;
//	vec4 Color;
//	vec4 Attenuation;
//};
//
//layout(std430, binding = 0) buffer lightBuffer
//{
//    Light u_lights[];
//};
//
//float CalculateDiffuse_Lambert(vec3 fragmentPosition, vec3 normal, vec3 lightPosition)
//{
//    vec3 lightDir = normalize(lightPosition - fragmentPosition);
//    float diffuse = max(dot(lightDir, normal), 0.0);
//    return diffuse;
//}
//
//float CalculateSpecular_BlinnPhong(vec3 fragmentPosition, vec3 normal, vec3 lightPosition, vec3 cameraPosition, float attenuationExponent)
//{
//    vec3 lightDir = normalize(lightPosition - fragmentPosition);
//    vec3 viewDir = cameraPosition - fragmentPosition; 
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    
//    float spec = pow(max(dot(normal, halfwayDir), 0.0), attenuationExponent);
//    return spec;
//}
//
//float CalculateAttenuation(vec3 fragmentPosition, vec3 lightPosition, float attenuation)
//{
//    float dist = length(fragmentPosition - lightPosition);
//    float value = clamp(dist / 1 + attenuation, 0.0, 1.0f);
//    value = sqrt(value);
//    return value;
//}

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

	vec3 finalLight = vec3(0.0f);
//	vec3 diffuseLight = vec3(0.0f);
//	vec3 ambientLight = vec3(0.0f);
//	vec3 specularLight = vec3(0.0f);
//
//	for (int lightIndex = 0; lightIndex < 4; lightIndex++)
//	{
//        vec3 lightPosition = u_lights[lightIndex].Position.xyz;
//		vec3 lightColor = u_lights[lightIndex].Color.rgb;
//		float lightAttenuation = u_lights[lightIndex].Attenuation.r;
//
//	    float attenuation = CalculateAttenuation(position, lightPosition, lightAttenuation);
//		
//		diffuseLight += attenuation * CalculateDiffuse_Lambert(position, normal, lightPosition) * lightColor;
//		specularLight += attenuation * CalculateSpecular_BlinnPhong(position, normal, lightPosition, u_camera_position, 8);
//	}
//	finalLight = (ambientLight + diffuseLight + (specular * specularLight));

	finalLight = texture(t_lights, i.texcoord).rgb;

	fragmentColor.xyz = finalLight * albedo;

	if (depth == 1.0)
	{
		fs_color = texture(tc_skybox, i.ray);
	}
	else
	{
		fs_color = fragmentColor;
	}
}