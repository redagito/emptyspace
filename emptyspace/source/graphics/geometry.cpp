#include <emptyspace/graphics/geometry.hpp>

Geometry* Geometry::CreateEmpty()
{
	return new Geometry();
}

Geometry* Geometry::CreateCube(f32 width, f32 height, f32 depth)
{
	std::vector<VertexPositionColorNormalUv> const vertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),

		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
	};

	std::vector<u8> const indices =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,

		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	const auto vertexCount = vertices.size();
	const auto indexCount = indices.size();

	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
	};

	return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
}

Geometry* Geometry::CreatePlane(f32 width, f32 height)
{
	std::vector<VertexPositionColorNormalUv> const vertices =
	{
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		VertexPositionColorNormalUv(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		VertexPositionColorNormalUv(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
	};

	std::vector<u8> const indices =
	{
		0, 1, 2, 2, 3, 0,
	};

	const auto vertexCount = vertices.size();
	const auto indexCount = indices.size();

	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
	};

	return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
}

Geometry* Geometry::CreatePlainFromFile(const std::filesystem::path& filePath)
{
	std::vector<VertexPosition> vertices;
	std::vector<u8> indices;

	Assimp::Importer importer;
	const auto scene = importer.ReadFile(filePath.string(), /*aiProcess_JoinIdenticalVertices |*/ aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData);
	const auto mesh = scene->mMeshes[0];

	for (u32 f = 0; f < mesh->mNumFaces; ++f)
	{
		const auto face = mesh->mFaces[f];
		if (face.mNumIndices != 3)
		{
			continue;
		}

		for (u32 i = 0; i < 3; ++i)
		{
			const auto index = face.mIndices[i];

			glm::vec3 position;
			position.x = mesh->mVertices[index].x;
			position.y = mesh->mVertices[index].y;
			position.z = mesh->mVertices[index].z;

			VertexPosition vertex(position);
			vertices.push_back(vertex);

			indices.push_back(index);
		}
	}

	const auto vertexCount = vertices.size();
	const auto indexCount = indices.size();

	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPosition, Position))
	};

	return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
}

Geometry* Geometry::CreateFromFile(const std::filesystem::path& filePath)
{
	std::vector<VertexPositionColorNormalUv> vertices;
	std::vector<u8> indices;

	Assimp::Importer importer;
	const auto scene = importer.ReadFile(filePath.string(), /*aiProcess_JoinIdenticalVertices |*/ aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FindInvalidData);
	const auto mesh = scene->mMeshes[0];

	for (u32 f = 0; f < mesh->mNumFaces; ++f)
	{
		const auto face = mesh->mFaces[f];
		if (face.mNumIndices != 3)
		{
			continue;
		}

		for (u32 i = 0; i < 3; ++i)
		{
			const auto index = face.mIndices[i];

			glm::vec3 position;
			position.x = mesh->mVertices[index].x;
			position.y = mesh->mVertices[index].y;
			position.z = mesh->mVertices[index].z;

			glm::vec3 normal;
			if (mesh->HasNormals())
			{
				normal.x = mesh->mNormals[index].x;
				normal.y = mesh->mNormals[index].y;
				normal.z = mesh->mNormals[index].z;
			}

			glm::vec2 uv(0.0f, 0.0f);

			VertexPositionColorNormalUv vertex(position, normal, normal, uv);
			vertices.push_back(vertex);
		}
	}

	const auto vertexCount = vertices.size();
	const auto indexCount = indices.size();

	std::vector<AttributeFormat> const vertexFormat =
	{
		CreateAttributeFormat<glm::vec3>(0, offsetof(VertexPositionColorNormalUv, Position)),
		CreateAttributeFormat<glm::vec3>(1, offsetof(VertexPositionColorNormalUv, Color)),
		CreateAttributeFormat<glm::vec3>(2, offsetof(VertexPositionColorNormalUv, Normal)),
		CreateAttributeFormat<glm::vec2>(3, offsetof(VertexPositionColorNormalUv, Texcoord))
	};

	return new Geometry(vertices, indices, u32(vertexCount), u32(indexCount), vertexFormat);
}

void Geometry::Bind() const
{
	glBindVertexArray(_vao);
}

void Geometry::DrawArrays() const
{
	glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
}

void Geometry::DrawElements() const
{
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_BYTE, nullptr);
}

Geometry::Geometry()
{
	glCreateVertexArrays(1, &_vao);
}

Geometry::~Geometry()
{
	delete _ibo;
	delete _vbo;
	glDeleteVertexArrays(1, &_vao);
}