#version 450

layout(location = 1) in vec3 fs_fragment_position;
layout(location = 2) in vec3 fs_normal;
layout(location = 3) in vec2 fs_uv;
layout(location = 4) in vec4 fs_tangent;
layout(location = 5) in flat int fs_material_id;
layout(location = 6) in smooth vec4 fs_current_position;
layout(location = 7) in smooth vec4 fs_previous_position;
layout(location = 8) in flat mat4 fs_model_matrix;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_albedo;
layout(location = 3) out vec2 out_velocity;
layout(location = 4) out vec4 out_emission;

layout(binding = 0) uniform sampler2D t_diffuse;
layout(binding = 1) uniform sampler2D t_specular;
layout(binding = 2) uniform sampler2D t_normal;

void main()
{
    vec3 v_diffuse = texture(t_diffuse, fs_uv).rgb;
    vec3 v_specular = texture(t_specular, fs_uv).rgb;
    vec3 v_normal = texture(t_normal, fs_uv).rgb * 2.0 - 1.0;

    vec3 v_bitangent = cross(fs_normal, fs_tangent.xyz) * fs_tangent.w;
    mat3 v_tbn = mat3(fs_tangent.xyz, v_bitangent, normalize(fs_normal));
    v_normal = normalize(v_tbn * v_normal);
    v_normal = (fs_model_matrix * vec4(v_normal, 0.0)).xyz;

    out_position = vec4(fs_fragment_position, v_specular.r);
    out_normal = normalize(v_normal);
    out_albedo.rgb = v_diffuse;
    out_velocity = ((fs_current_position.xy / fs_current_position.w) * 0.5 + 0.5) - ((fs_previous_position.xy / fs_previous_position.w) * 0.5 + 0.5);
    out_emission = vec4(v_specular, 1.0f) * 1.2f;
}