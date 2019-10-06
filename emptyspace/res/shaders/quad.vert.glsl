#version 450

out gl_PerVertex{ vec4 gl_Position; };

out vec2 uv;

void main()
{
	//vec2 vertices[] =
	//{
	//	vec2(-1.0f, 1.0f),
	//	vec2(1.0f, 1.0f),
	//	vec2(1.0f,-1.0f),
	//	vec2(-1.0f,-1.0f)
	//};

	//vec2 uvs[] =
	//{
	//	vec2(0.0f, 1.0f),
	//	vec2(1.0f, 1.0f),
	//	vec2(1.0f, 0.0f),
	//	vec2(0.0f, 0.0f)

	//};
	//uint indices[] = { 0, 3, 2, 2, 1, 0 };

	//const vec2 position = vertices[indices[gl_VertexID]];
	//const vec2 texcoord = uvs[indices[gl_VertexID]];

	//o.uv = uv;
	//gl_Position = vec4(position, 0.0, 1.0);

	gl_Position = vec4(float(gl_VertexID / 2) * 4.0 - 1.0, float(gl_VertexID % 2) * 4.0 - 1.0, 0.0, 1.0);
	uv = vec2(float(gl_VertexID / 2) * 2.0, 1.0 - float(gl_VertexID % 2) * 2.0);
}