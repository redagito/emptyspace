#version 450

layout (location = 0) out vec4 fs_color;

layout (binding = 0) uniform sampler2D t_position;
layout (binding = 1) uniform sampler2D t_normal;
layout (binding = 2) uniform sampler2D t_depth;

layout(location = 0) uniform vec4 u_light_position;
layout(location = 1) uniform vec4 u_light_color;
layout(location = 2) uniform vec4 u_light_attenuation;
layout(location = 3) uniform vec3 u_camera_position;


in in_block
{
	vec2 texcoord;
} inp;

float CalculateDiffuse_Lambert(vec3 fragmentPosition, vec3 normal, vec3 lightPosition)
{
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float diffuse = max(dot(lightDir, normal), 0.0);
    return diffuse;
}

float CalculateSpecular_BlinnPhong(vec3 fragmentPosition, vec3 normal, vec3 lightPosition, vec3 cameraPosition, float attenuationExponent)
{
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    vec3 viewDir = cameraPosition - fragmentPosition; 
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float spec = pow(max(dot(normal, halfwayDir), 0.0), attenuationExponent);
    return spec;
}

float CalculateAttenuation(vec3 fragmentPosition, vec3 lightPosition, float attenuation)
{
    float dist = length(fragmentPosition - lightPosition);
    float value = clamp(1 - dist / attenuation, 0.0, 1.0f);
    //value = value * value;
    return value;
}

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(t_position, 0));
    vec2 tex_coords = gl_FragCoord.xy * texel_size;

	const vec3 position = texture(t_position, tex_coords).rgb;
	const vec3 normal = texture(t_normal, tex_coords).rgb;
    	
	vec3 finalLight = vec3(0.0f);
	vec3 diffuseLight = vec3(0.0f);
	vec3 ambientLight = vec3(0.0f);
	vec3 specularLight = vec3(0.0f);

	const vec3 lightPosition = u_light_position.xyz;
	const vec3 lightColor = u_light_color.rgb;
	const float lightAttenuation = u_light_attenuation.r;
	
	float attenuation = CalculateAttenuation(position, lightPosition, lightAttenuation);
		
	diffuseLight += attenuation * CalculateDiffuse_Lambert(position, normal, lightPosition) * lightColor;
	specularLight += attenuation * CalculateSpecular_BlinnPhong(position, normal, lightPosition, u_camera_position, 8);

	finalLight = (ambientLight + diffuseLight + (specularLight));

    fs_color = vec4(finalLight, 1.0);
}