#version 450

layout(location = 0) out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 1) out vec2 fs_uv;

void main()
{
    gl_Position = vec4(float(gl_VertexID / 2) * 4.0 - 1.0, float(gl_VertexID % 2) * 4.0 - 1.0, 0.0, 1.0);
    fs_uv = vec2(float(gl_VertexID / 2) * 2.0, float(gl_VertexID % 2) * 2.0);
}