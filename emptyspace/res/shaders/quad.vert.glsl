#version 450

out gl_PerVertex{ vec4 gl_Position; };

out vec2 texCoord;

void main()
{
	gl_Position = vec4(float(gl_VertexID / 2) * 4.0 - 1.0, float(gl_VertexID % 2) * 4.0 - 1.0, 0.0, 1.0);
	texCoord = vec2(float(gl_VertexID / 2) * 2.0, float(gl_VertexID % 2) * 2.0);
}