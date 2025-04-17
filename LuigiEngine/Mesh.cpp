#include "external/OBJ_Loader.h"

// TODO [TP03] Load OBJ
void loadOBJ(const char* fileName, vector<vec3>& vertices, vector<vec3>& normals,
	vector<vec2>& uvs, vector<unsigned int>& triangles)
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	triangles.clear();

	objl::Loader OBJLoader;
	if (OBJLoader.LoadFile(fileName))
	{
		objl::Mesh mesh = OBJLoader.LoadedMeshes[0];

		for (objl::Vertex& vertice : mesh.Vertices)
		{
			vertices.emplace_back(vertice.Position.X, vertice.Position.Y, vertice.Position.Z);
			normals.emplace_back(vertice.Normal.X, vertice.Normal.Y, vertice.Normal.Z);
			uvs.emplace_back(vertice.TextureCoordinate.X, vertice.TextureCoordinate.Y);
		}

		for (int i = 0; i < mesh.Indices.size(); i += 3)
		{
			triangles.emplace_back(mesh.Indices[i]);
			triangles.emplace_back(mesh.Indices[i+1]);
			triangles.emplace_back(mesh.Indices[i+2]);
		}
	}
	else
	{
		printf("Failed to load OBJ file %s.\n", fileName);
	}
}

struct Mesh {
	vector<vec3> vertices{};
	vector<vec3> normals{};
	vector<vec2> uvs{};
	vector<unsigned int> triangles;

	Mesh() = default;

	explicit Mesh(const string& objFile)
	{
		loadOBJ(objFile.c_str(), vertices, normals, uvs, triangles);
	}
};
