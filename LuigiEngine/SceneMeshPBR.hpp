#ifndef SCENEMESHPBR_H
#define SCENEMESHPBR_H

#include "SceneMesh.hpp"
#include "Material.h"

class SceneMeshPBR : public SceneMesh
{
	Material material;

protected:
	void render() override;

public:
	SceneMeshPBR(const vector<pair<double, Mesh*>>& meshes, const string& maps_folder,
		GLuint programID, Material material);
};

#endif //SCENEMESHPBR_H
