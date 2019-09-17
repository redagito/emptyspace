#version 450

layout (location = 0) out vec4 fs_color;

layout (binding = 0) uniform sampler2D t_color;
layout (binding = 1) uniform sampler2D t_velocity;

layout (location = 0) uniform float u_velocity_scale;

in in_block
{
	vec2 texcoord;
} inp;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(t_color, 0));
    vec2 tex_coords = gl_FragCoord.xy * texel_size;
    vec2 velocity = texture(t_velocity, tex_coords).rg;
    velocity *= u_velocity_scale;

    float speed = length(velocity / texel_size);
    int samples = clamp(int(speed), 1, 32);

    fs_color = texture(t_color, inp.texcoord);

    for (int i = 1; i < samples; ++i)
    {
        vec2 offset = velocity * (float(i) / float(samples - 1) - 0.5);
        fs_color += texture(t_color, tex_coords + offset);
    }

    fs_color /= float(samples);
}