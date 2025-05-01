#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

#include "SceneMeshPBR.hpp"

void SceneMeshPBR::render()
{
	glUseProgram(programID);
	vector<vec3> lights = {{0.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, -10.0f, 0.0f}};
	vector<vec3> lightColors = {{255.0f, 0.0f, 0.0f}, {0.0f, 255.0f, 0.0f}, {0.0f, 0.0f, 255.0f}};
	vec3 cameraPos = vec3(mainCamera->transform.getGlobalModel()[3]);

	mat3 globalRot = mat3(transform.getGlobalModel());
	globalRot[0] = normalize(globalRot[0]);
	globalRot[1] = normalize(globalRot[1]);
	globalRot[2] = normalize(globalRot[2]);

	glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &transform.getGlobalModel()[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(programID, "rotation"), 1, GL_FALSE, &globalRot[0][0]);
	glUniform3fv(glGetUniformLocation(programID, "lightPositions"), lights.size(), &lights[0][0]);
	glUniform3fv(glGetUniformLocation(programID, "lightColors"), lightColors.size(), &lightColors[0][0]);
	glUniform3f(glGetUniformLocation(programID, "camPos"), cameraPos[0], cameraPos[1], cameraPos[2]);
	SceneMesh::render();
}

SceneMeshPBR::SceneMeshPBR(const vector<pair<double, Mesh*>>& meshes, const string& maps_folder,
		GLuint programID
		):SceneMesh(meshes, {maps_folder+"/albedo.png", maps_folder+"/ao.png", maps_folder+"/metallic.png",
			maps_folder+"/normal.png", maps_folder+"/roughness.png"}, {"albedoMap", "aoMap",
				"metallicMap", "normalMap", "roughnessMap"}, programID)
{
}
