#version 450

layout(location = 0) in vec3 in_position;

layout(location = 0) out gl_PerVertex
{
    vec4 gl_Position;
};
layout(location = 1) out vec2 fs_uv;

layout(location = 0) uniform mat4 u_projection;
layout(location = 1) uniform mat4 u_view;
layout(location = 2) uniform mat4 u_model;

void main()
{
    const vec4 mpos = (u_projection * u_view * u_model * vec4(in_position, 1.0));
    gl_Position = mpos;
}