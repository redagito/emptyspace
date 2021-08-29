#version 450

layout(location = 1) in vec2 fs_uv;
layout(location = 2) in vec3 fs_ray;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D t_position;
layout(binding = 1) uniform sampler2D t_normal;
layout(binding = 2) uniform sampler2D t_albedo;
layout(binding = 3) uniform sampler2D t_depth;
layout(binding = 4) uniform samplerCube tc_skybox;
layout(binding = 5) uniform sampler2D t_lights;

layout(location = 0) uniform float u_blend_factor;

void main()
{
    const vec3 position = texture(t_position, fs_uv).rgb;
    const vec3 normal = texture(t_normal, fs_uv).rgb;
    const vec4 albedo_specular = texture(t_albedo, fs_uv);
    const vec3 albedo = albedo_specular.rgb;
    const float specular = albedo_specular.a;
    const float depth = texture(t_depth, fs_uv).r;
    
    vec4 fragmentColor = vec4(1.0);
    vec3 ambientColor = vec3(0.2);

    fragmentColor.xyz = texture(t_lights, fs_uv).rgb * albedo;

    //vec3 n = 0.5f * normal + 0.5f;

    if (depth == 1.0)
    {
        out_color = texture(tc_skybox, fs_ray);
    }
    else
    {
        out_color = fragmentColor;
    }
}