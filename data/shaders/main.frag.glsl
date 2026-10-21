#version 450

layout(location = 1) in vec2 fs_uv;
layout(location = 2) in vec3 fs_ray;

layout(location = 0) out vec4 out_color;

layout(binding = 2) uniform sampler2D t_albedo;
layout(binding = 3) uniform sampler2D t_depth;
layout(binding = 4) uniform samplerCube tc_skybox;
layout(binding = 5) uniform sampler2D t_lights;
layout(binding = 6) uniform sampler2D t_emission;

void main()
{
    const vec4 albedo_specular = texture(t_albedo, fs_uv);
    const vec3 albedo = albedo_specular.rgb;
    const float depth = texture(t_depth, fs_uv).r;

    vec3 fragmentColor = texture(t_lights, fs_uv).rgb * albedo;

    if (depth == 1.0)
    {
        out_color = texture(tc_skybox, fs_ray);
    }
    else
    {
        out_color = vec4(fragmentColor + texture(t_emission, fs_uv).rgb, 1.0);
    }
}