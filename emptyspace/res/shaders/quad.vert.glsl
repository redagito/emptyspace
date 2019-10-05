#version 450

out gl_PerVertex{ vec4 gl_Position; };

out out_block
{
	vec2 uv;
} o;

void main()
{
	//const uint id = gl_VertexID;

	//o.uv = vec2((id << 1) & 2, id & 2);
	//gl_Position = vec4(o.uv * vec2(2, -2) + vec2(-1, 1), 0, 1);

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

	const vec2 position = vertices[indices[gl_VertexID]];
	const vec2 uv = uvs[indices[gl_VertexID]];

	o.uv = uv;
	gl_Position = vec4(position, 0.0, 1.0);
}