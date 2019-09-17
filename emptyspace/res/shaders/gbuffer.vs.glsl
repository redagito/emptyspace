#version 450

out gl_PerVertex { vec4 gl_Position; };

out out_block
{
	vec3 position;
	vec3 normal;
	vec2 uv;
	smooth vec4 curr_pos;
	smooth vec4 prev_pos;
} o;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;
layout (location = 3) uniform mat4 u_model_view_projection_current;
layout (location = 4) uniform mat4 u_model_view_projection_previous;
layout (location = 5) uniform bool u_exclude_from_motionblur;

layout (location = 6) uniform bool u_is_instanced;

layout(std430, binding = 0) buffer instanceBuffer
{
    mat4 u_models[];
};

void main()
{
	if (!u_exclude_from_motionblur)
	{
		o.curr_pos = u_model_view_projection_current * vec4(in_position, 1.0);
		o.prev_pos = u_model_view_projection_previous * vec4(in_position, 1.0);
	}
	else
	{
		o.curr_pos = u_model_view_projection_current * vec4(in_position, 1.0);
		o.prev_pos = o.curr_pos;
	}

	mat4 model = u_model;
	if (u_is_instanced)
	{
	   model = u_models[gl_InstanceID];
	}

	const vec4 mpos = (u_view * model * vec4(in_position, 1.0));
	o.position = (model * vec4(in_position, 1.0)).xyz;
	o.normal = mat3(transpose(inverse(model))) * in_normal;
	o.uv = in_uv;
	gl_Position = u_projection * mpos;
}