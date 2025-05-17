#ifndef SCENEMESH_H
#define SCENEMESH_H

#include "ECS.h"
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Mesh.hpp"

#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <iostream>


#include "external/stb_image.h"

using namespace std;

struct MeshComponent {
    vector<pair<double, Mesh*>> meshes;
    Mesh* activeMesh;

    GLuint vertexbuffer;
    GLuint normalbuffer;
    GLuint uvbuffer;
    GLuint elementbuffer;
	vector<string> texFiles;
    vector<string> texUniforms;

    GLuint programID;
    mat4* mvp;
    string material = "";
	GLuint cubeMapID = 0;

	MeshComponent() = default;

	MeshComponent(
        const vector<pair<double, Mesh*>>& meshes,
        GLuint programID,
        const vector<string>& texFiles_in = {},
        const vector<string>& texUniforms_in = {},
        const string& material = "",
        const string& cubeMap = ""
    ) : meshes(meshes), activeMesh(meshes.empty() ? nullptr : meshes[0].second), programID(programID), mvp(nullptr) {

			createVBO();
			if (material.empty()) {
				texFiles = texFiles_in;
				texUniforms = texUniforms_in;
			} else {
				this->material = material;
				texFiles = {material+"/albedo.png", material+"/ao.png", material+"/metallic.png",
								material+"/normal.png", material+"/roughness.png"};
				texUniforms = {"albedoMap", "aoMap", "metallicMap", "normal", "roughnessMap"};
				if (!cubeMap.empty())
					cubeMapID = loadCubemap("textures/cubemap/"+cubeMap+"/");
			}
    }


    void createVBO();
    void clearVBO();
    void checkLOD(const vec3& cameraPos, const vec3& entityPos);
	static GLuint loadCubemap(string folder);

	void onAttach(Registry& registry, Entity entity);
    void onDetach(Registry& registry, Entity entity){};
};


struct TextureComponent {
    vector<string> texFiles;
    vector<string> texUniforms;
    vector<GLuint> textureIDs;

	TextureComponent(
        const vector<string>& texFiles = {},
        const vector<string>& texUniforms = {}
    )
        : texFiles(texFiles), texUniforms(texUniforms) {
        loadTextures();
    }

    void loadTextures();

	void onAttach(Registry& registry, Entity entity){};
    void onDetach(Registry& registry, Entity entity){};
};



#endif // SCENEMESH_H
