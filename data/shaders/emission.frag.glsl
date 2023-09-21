#version 450

layout(location = 0) out vec4 fs_color;

layout(binding = 0) uniform sampler2D t_color;
layout(binding = 1) uniform sampler2D t_emission;

layout(location = 0) uniform float u_threshold;

in FragmentData
{
    vec2 Position;
    vec2 Uv;
} in_data;

void main()
{
    fs_color = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 fragColor = texture(t_color, in_data.Uv).rgb;
    vec3 emissiveColor = texture(t_emission, in_data.Uv).rgb;

    // Clamp to avoid exceeding max float
    fragColor = min(vec3(256 * 10, 256 * 10, 256 * 10), max(vec3(0.0), fragColor.rgb));

    // Calculate luminance of scene
    float brightness = dot(fragColor, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > u_threshold || length(emissiveColor) != 0)
    {
        vec3 op = clamp(fragColor, vec3(0), vec3(256));
        fs_color = vec4(op * 0.1, 1.0) + vec4(max(vec3(0.0), emissiveColor), 1.0);
    }
}