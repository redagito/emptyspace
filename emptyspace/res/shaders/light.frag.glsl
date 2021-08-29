#version 450

layout(location = 1) in vec2 fs_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D t_gbuffer_position;
layout(binding = 1) uniform sampler2D t_gbuffer_normal;
layout(binding = 2) uniform sampler2D t_gbuffer_depth;
layout(binding = 3) uniform sampler2D t_gbuffer_specular;

layout(location = 0) uniform int u_light_type;
layout(location = 1) uniform vec3 u_light_position;
layout(location = 2) uniform vec3 u_light_color;
layout(location = 3) uniform vec3 u_light_direction;
layout(location = 4) uniform vec3 u_light_attenuation;
layout(location = 5) uniform vec2 u_light_cutoff;
layout(location = 6) uniform vec3 u_camera_position;

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
    vec2 v_texel_size = 1.0 / vec2(textureSize(t_gbuffer_position, 0));
    vec2 v_uv = gl_FragCoord.xy * v_texel_size;

    const vec4 v_gbuffer_position = texture(t_gbuffer_position, v_uv);

    const vec3 position = v_gbuffer_position.rgb;
    const vec3 normal = texture(t_gbuffer_normal, v_uv).rgb;
    const float v_specular = v_gbuffer_position.a;
        
    vec3 finalLight = vec3(0.0f);
    vec3 diffuseLight = vec3(0.0f);
    vec3 ambientLight = vec3(0.0f);
    vec3 specularLight = vec3(0.0f);

    const vec3 lightPosition = u_light_position.xyz;
    const vec3 lightColor = u_light_color.rgb;
    const vec3 lightAttenuation = u_light_attenuation;

    if (u_light_type == 1)
    {
        float attenuation = CalculateAttenuation(position, lightPosition, lightAttenuation.z);
        
        diffuseLight += attenuation * CalculateDiffuse_Lambert(position, normal, lightPosition) * lightColor;
        specularLight += attenuation * CalculateSpecular_BlinnPhong(position, normal, lightPosition, u_camera_position, 8);

        finalLight = (ambientLight + diffuseLight + (specularLight));
    }
    else
    {
        const vec3 lightDirection = u_light_direction;
        const float lightCutOffInner = u_light_cutoff.x;
        const float lightCutOffOuter = u_light_cutoff.y;

        vec3 lightDir = normalize(lightPosition - position);

        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);

        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 viewDir = normalize(u_camera_position - lightPosition);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
        spec = v_specular + (0.00001 * spec);

        // attenuation
        float distance = length(lightPosition - position);
        float attenuation = 1.0 / (lightAttenuation.z + lightAttenuation.y * distance + lightAttenuation.x * (distance * distance));    

        // spotlight intensity
        float theta = dot(lightDir, normalize(-lightDirection)); 
        float epsilon = lightCutOffInner - lightCutOffOuter;
        float intensity = clamp((theta - lightCutOffOuter) / epsilon, 0.0, 1.0);

        // combine results
        vec3 ambient = lightColor; // * vec3(texture(material.diffuse, fs_uv));
        vec3 diffuse = lightColor * diff; // * vec3(texture(material.diffuse, fs_uv));
        vec3 specular = lightColor * spec; // * vec3(texture(material.specular, fs_uv));
        ambient *= attenuation * intensity;
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;

        finalLight = lightColor * diff * intensity;
        //finalLight = diffuseLight += diffuse;
        //finalLight += specularLight += specular;

//		return (ambientLight + diffuseLight + specularLight);
    }

    out_color = vec4(finalLight, 1.0);
}