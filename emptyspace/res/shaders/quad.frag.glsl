#version 450

layout(location = 1) in vec2 fs_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D t_final;
layout(location = 0) uniform vec4 u_blend_color;

void main()
{
    vec3 v_final_color = texture(t_final, fs_uv).rgb;

    out_color = vec4(mix(v_final_color, u_blend_color.rgb, u_blend_color.w), 1.0f);
}