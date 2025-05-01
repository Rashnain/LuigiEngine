#ifndef SCENEMESHPBR_H
#define SCENEMESHPBR_H

#include "SceneMesh.hpp"

class SceneMeshPBR : public SceneMesh
{

protected:
	void render() override;

public:
	SceneMeshPBR(const vector<pair<double, Mesh*>>& meshes, const string& maps_folder,
		GLuint programID);
};

#endif //SCENEMESHPBR_H
