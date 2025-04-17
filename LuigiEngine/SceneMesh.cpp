#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

extern int* nbMVPUpdate;
extern bool* optimizeMVP;

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "SceneMesh.hpp"

void SceneMesh::createVBO() {
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, activeMesh->vertices.size() * sizeof(vec3), &activeMesh->vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, activeMesh->normals.size() * sizeof(vec3), &activeMesh->normals[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, activeMesh->uvs.size() * sizeof(vec2), &activeMesh->uvs[0] , GL_STATIC_DRAW);
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, activeMesh->triangles.size() * sizeof(unsigned int), &activeMesh->triangles[0] , GL_STATIC_DRAW);
}

void SceneMesh::clearVBO() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &elementbuffer);
}

// TODO [TP04] Level Of Detail
void SceneMesh::checkLOD() {
	if (meshes.size() == 1) return;
	const double distanceToCamera = length(vec3(mainCamera->transform.getGlobalModel()[3]) - vec3(transform.getGlobalModel()[3]));
	for (unsigned long i = meshes.size(); i-- > 0;)
		if (distanceToCamera >= meshes[i].first) {
			if (activeMesh != meshes[i].second) {
				activeMesh = meshes[i].second;
				createVBO();
			}
			break;
		}
}

void SceneMesh::render()
{
	if (mainCamera) {
		if (!*optimizeMVP || (mainCamera->viewProjChanged || transform.changed)) {
			mvp = new mat4(mainCamera->getViewProjMatrix() * transform.getGlobalModel());
			nbMVPUpdate = new int(*nbMVPUpdate + 1);
			transform.changed = false;
			checkLOD();
		}
		glUseProgram(programID);
		glUniformMatrix4fv(glGetUniformLocation(programID, "mvp"), 1, GL_FALSE, &mvp[0][0][0]);
	} else {
		cerr << "You are trying to display a mesh, but no main camera has been set." << endl <<
			"Create one with SceneCamera, and activate it with SceneObject::setMainCamera(...)." << endl <<
			"Beware that the camera must be in your scene tree for the meshes to be displayed correctly." << endl;
	}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	// TODO [TP01] Texture UV buffer
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	for (int i = 0; i < textureIDs.size(); i++) {
		// TODO [TP01] Charge les textures dans le GPU
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glUniform1i(glGetUniformLocation(programID, uniforms[i].c_str()), i);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glDrawElements(GL_TRIANGLES, activeMesh->triangles.size(), GL_UNSIGNED_INT, nullptr);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

void SceneMesh::clear() {
	clearVBO();
	glDeleteProgram(programID);
}

// TODO [LOD] string& impostor, double impostorDist
SceneMesh::SceneMesh(const vector<pair<double, Mesh*>>& meshes, const vector<string>& texFiles,
	const vector<string>& texUniforms, GLuint programID)
{
	// mesh
	this->meshes = meshes;
	this->activeMesh = meshes[0].second;
	createVBO();

	// shaders
	textureIDs = vector<GLuint>(texFiles.size());
	uniforms = texUniforms;
	this->programID = programID;

	for (int i = 0; i < texFiles.size(); i++)
	{
		// TODO [TP01] Texture IMG
		int width, height, nrChannels;
		unsigned char* img = stbi_load(("textures/" + texFiles[i]).c_str(), &width, &height, &nrChannels, 0);
		glGenTextures(1, &textureIDs[i]);
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLint type = GL_RGB;
		if (nrChannels == 4)
			type = GL_RGBA;
		else if (nrChannels == 2)
			type = GL_RG;
		else if (nrChannels == 1)
			type = GL_R;
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(img);
	}
}
