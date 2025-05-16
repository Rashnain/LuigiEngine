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

struct MeshComponent {
    std::vector<std::pair<double, Mesh*>> meshes;
    Mesh* activeMesh;

    GLuint vertexbuffer;
    GLuint normalbuffer;
    GLuint uvbuffer;
    GLuint elementbuffer;
	std::vector<std::string> texFiles;
    std::vector<std::string> texUniforms;

    GLuint programID;
    mat4* mvp;

	MeshComponent() = default;

	MeshComponent(
        const std::vector<std::pair<double, Mesh*>>& meshes,
        GLuint programID,
        const std::vector<std::string>& texFiles_in = {},
        const std::vector<std::string>& texUniforms_in = {}
    ) : meshes(meshes), activeMesh(meshes.empty() ? nullptr : meshes[0].second), programID(programID), mvp(nullptr) {

			createVBO();

			texUniforms = texUniforms_in;
			texFiles = texFiles_in;
    }


    void createVBO();
    void clearVBO();
    void checkLOD(const vec3& cameraPos, const vec3& entityPos);

	void onAttach(Registry& registry, Entity entity);
    void onDetach(Registry& registry, Entity entity){};
};


struct TextureComponent {
    std::vector<std::string> texFiles;
    std::vector<std::string> texUniforms;
    std::vector<GLuint> textureIDs;

	TextureComponent(
        const std::vector<std::string>& texFiles = {},
        const std::vector<std::string>& texUniforms = {}
    )
        : texFiles(texFiles), texUniforms(texUniforms) {
        loadTextures();
    }

    void loadTextures();

	void onAttach(Registry& registry, Entity entity){};
    void onDetach(Registry& registry, Entity entity){};
};



#endif // SCENEMESH_H
