#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_uvw;
layout(location = 4) in vec4 i_tangent;

layout(location = 0) out gl_PerVertex
{
    vec4 gl_Position;
};
layout(location = 1) out vec3 fs_fragment_position;
layout(location = 2) out vec3 fs_normal;
layout(location = 3) out vec2 fs_uv;
layout(location = 4) out vec4 fs_tangent;
layout(location = 5) out flat int fs_material_id;
layout(location = 6) out smooth vec4 fs_current_position;
layout(location = 7) out smooth vec4 fs_previous_position;
layout(location = 8) out flat mat4 fs_model_matrix;

layout(location = 0) uniform mat4 u_projection;
layout(location = 1) uniform mat4 u_view;
layout(location = 2) uniform mat4 u_model;
layout(location = 3) uniform mat4 u_model_view_projection_current;
layout(location = 4) uniform mat4 u_model_view_projection_previous;
layout(location = 5) uniform bool u_exclude_from_motionblur;
layout(location = 6) uniform bool u_is_instanced;

layout(std430, binding = 0) buffer instanceBuffer
{
    mat4 b_world_matrices[];
};

void main()
{
    if (u_exclude_from_motionblur)
    {
        fs_current_position = u_model_view_projection_current * vec4(i_position, 1.0);
        fs_previous_position = fs_current_position;
    }
    else
    {
        fs_current_position = u_model_view_projection_current * vec4(i_position, 1.0);
        fs_previous_position = u_model_view_projection_previous * vec4(i_position, 1.0);
    }

    mat4 v_model_matrix = u_model;
    if (u_is_instanced)
    {
       v_model_matrix = b_world_matrices[gl_InstanceID];
    }

    const vec4 mpos = (u_view * v_model_matrix * vec4(i_position, 1.0));
    gl_Position = u_projection * mpos;

    fs_fragment_position = (v_model_matrix * vec4(i_position, 1.0)).xyz;
    fs_normal = i_normal;
    fs_uv = i_uvw.xy;
    fs_tangent = i_tangent;
    fs_model_matrix = v_model_matrix;
}
