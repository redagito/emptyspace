#version 450

layout (location = 1) in vec2 fs_uv;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform sampler2D t_color;
layout (binding = 1) uniform sampler2D t_velocity;

layout (location = 0) uniform float u_velocity_scale;

void main()
{
    vec2 v_texel_size = 1.0 / vec2(textureSize(t_color, 0));
    vec2 v_uv = gl_FragCoord.xy * v_texel_size;
    vec2 v_velocity = texture(t_velocity, v_uv).rg * u_velocity_scale;

    float v_speed = length(v_velocity / v_texel_size);
    int v_samples = clamp(int(v_speed), 1, 32);

    out_color = texture(t_color, fs_uv);

    for (int i = 1; i < v_samples; ++i)
    {
        vec2 v_offset = v_velocity * (float(i) / float(v_samples - 1) - 0.5);
        out_color += texture(t_color, v_uv + v_offset);
    }

    out_color /= float(v_samples);
}