#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

#include "SceneMeshPhong.hpp"

void SceneMeshPhong::render()
{
	glUseProgram(programID);
	vec3 light = vec3(0.0f, 0.0f, 0.0f); // TODO [Phong] SceneMeshPhong::addLight
	vec3 lightColor = vec3(1.0f, 1.0f, 0.9f);
	vec3 camera = vec3(mainCamera->transform.getGlobalModel()[3]);

	mat3 globalRot = mat3(transform.getGlobalModel());
	globalRot[0] = normalize(globalRot[0]);
	globalRot[1] = normalize(globalRot[1]);
	globalRot[2] = normalize(globalRot[2]);

	// TODO [Phong] SceneMesh::setUniforms() virtual
	glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &transform.getGlobalModel()[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(programID, "rotation"), 1, GL_FALSE, &globalRot[0][0]);
	glUniform3f(glGetUniformLocation(programID, "light"), light[0], light[1], light[2]);
	glUniform3f(glGetUniformLocation(programID, "camera"), camera[0], camera[1], camera[2]);
	glUniform3f(glGetUniformLocation(programID, "light_color"), lightColor[0], lightColor[1], lightColor[2]);
	glUniform1f(glGetUniformLocation(programID, "k_ambiante"), material.ambiant);
	glUniform1f(glGetUniformLocation(programID, "k_diffuse"), material.diffuse);
	glUniform1f(glGetUniformLocation(programID, "k_specular"), material.specular);
	glUniform1i(glGetUniformLocation(programID, "alpha"), material.alpha);
	SceneMesh::render();
}

SceneMeshPhong::SceneMeshPhong(const vector<pair<double, Mesh*>>& meshes, const string& texFile,
	const string& texUniform, GLuint programID, Material material
	):SceneMesh(meshes, vector{texFile}, vector{texUniform}, programID)
{
	this->material = material;
}
