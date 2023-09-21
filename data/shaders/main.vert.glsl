#version 450

layout(location = 0) out gl_PerVertex
{
    vec4 gl_Position;
};
layout(location = 1) out vec2 fs_uv;
layout(location = 2) out vec3 fs_ray;

layout(location = 0) uniform mat3 u_camera_direction;
layout(location = 1) uniform float u_fov;
layout(location = 2) uniform float u_ratio;
layout(location = 3) uniform vec2 u_uv_diff;

vec3 skyray(vec2 texcoord, float fovy, float aspect)
{
    float d = 0.5 / tan(fovy / 2.0);
    return normalize(vec3((texcoord.x - 0.5) * aspect, texcoord.y - 0.5, -d));
}

void main()
{
    vec2 vertices[] = 
    {
        vec2(-1.0f, 1.0f),
        vec2(1.0f, 1.0f),
        vec2(1.0f,-1.0f),
        vec2(-1.0f,-1.0f)
    };
    vec2 uvs[] = 
    {
        vec2(0.0f, 1.0f),
        vec2(1.0f, 1.0f),
        vec2(1.0f, 0.0f),
        vec2(0.0f, 0.0f)

    };
    uint indices[] = { 0, 3, 2, 2, 1, 0 };

    const vec2 v_position = vertices[indices[gl_VertexID]];
    const vec2 v_uv = uvs[indices[gl_VertexID]];

    fs_ray = u_camera_direction * skyray(v_uv, u_fov, u_ratio);
    fs_uv = v_uv * u_uv_diff;
    gl_Position = vec4(v_position, 0.0, 1.0);
}