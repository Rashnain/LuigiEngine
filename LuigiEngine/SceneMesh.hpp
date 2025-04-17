#ifndef SCENEMESH_H
#define SCENEMESH_H

#include "SceneObject.hpp"
#include "Mesh.cpp"

class SceneMesh : public SceneObject
{
	vector<pair<double, Mesh*>> meshes;
	Mesh* activeMesh;
	GLuint vertexbuffer;
	GLuint normalbuffer;
	GLuint uvbuffer;
	GLuint elementbuffer;
	vector<GLuint> textureIDs;
	vector<string> uniforms;
	mat4* mvp;

	void createVBO();

	void clearVBO();

	void checkLOD();

protected:
	GLuint programID;

	void render() override;

	void clear() override;

public:
	SceneMesh(const vector<pair<double, Mesh*>>& meshes, const vector<string>& texFiles,
		const vector<string>& texUniforms, GLuint programID);
};

#endif //SCENEMESH_H
