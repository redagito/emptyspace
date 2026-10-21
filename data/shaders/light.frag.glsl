#version 450

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D t_gbuffer_position;
layout(binding = 1) uniform sampler2D t_gbuffer_normal;

layout(location = 0) uniform int u_light_type;
layout(location = 1) uniform vec3 u_light_position;
layout(location = 2) uniform vec3 u_light_color;
layout(location = 3) uniform vec3 u_light_direction;
layout(location = 4) uniform float u_light_attenuation;
layout(location = 5) uniform vec2 u_light_cutoff;
layout(location = 6) uniform vec3 u_camera_position;

const int LIGHT_TYPE_SPOT = 1;

void main()
{
    vec2 v_texel_size = 1.0 / vec2(textureSize(t_gbuffer_position, 0));
    vec2 v_uv = gl_FragCoord.xy * v_texel_size;

    const vec4 v_gbuffer_position = texture(t_gbuffer_position, v_uv);
    const vec3 position = v_gbuffer_position.rgb;
    const vec3 normal = texture(t_gbuffer_normal, v_uv).rgb;

    // The specular map of the material is kept in the alpha of the position.
    const float specularStrength = v_gbuffer_position.a;

    const vec3 toLight = u_light_position - position;
    const float distance = length(toLight);
    const vec3 lightDirection = distance > 0.0 ? toLight / distance : vec3(0.0, 0.0, 0.0);

    // The light fades out linearly over its attenuation radius.
    const float attenuation = u_light_attenuation > 0.0
        ? clamp(1.0 - distance / u_light_attenuation, 0.0, 1.0)
        : 0.0;

    const float diffuse = max(dot(lightDirection, normal), 0.0);

    const vec3 viewDirection = normalize(u_camera_position - position);
    const float specular = specularStrength * pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), 8.0);

    // Only a spot light is limited to a cone.
    float intensity = 1.0;
    if (u_light_type == LIGHT_TYPE_SPOT)
    {
        const float theta = dot(lightDirection, normalize(-u_light_direction));
        const float epsilon = u_light_cutoff.x - u_light_cutoff.y;
        intensity = clamp((theta - u_light_cutoff.y) / epsilon, 0.0, 1.0);
    }

    out_color = vec4(u_light_color * attenuation * intensity * (diffuse + specular), 1.0);
}
