#ifndef SCENEMESHPHONG_H
#define SCENEMESHPHONG_H

#include "SceneMesh.hpp"
#include "Material.h"

class SceneMeshPhong : public SceneMesh
{
	Material material;

protected:
	void render() override;

public:
	SceneMeshPhong(const vector<pair<double, Mesh*>>& meshes, const string& texFile,
		const string& texUniform, GLuint programID, Material material);
};

#endif //SCENEMESHPHONG_H
