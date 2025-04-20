#include "SceneMesh.hpp"


extern bool* optimizeMVP;



void MeshComponent::createVBO() {
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, activeMesh->vertices.size() * sizeof(glm::vec3), &activeMesh->vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, activeMesh->normals.size() * sizeof(glm::vec3), &activeMesh->normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, activeMesh->uvs.size() * sizeof(glm::vec2), &activeMesh->uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, activeMesh->triangles.size() * sizeof(unsigned int), &activeMesh->triangles[0], GL_STATIC_DRAW);
}

void MeshComponent::clearVBO() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &elementbuffer);
}

void MeshComponent::checkLOD(const glm::vec3& cameraPos, const glm::vec3& entityPos) {
    if (meshes.size() == 1) return;

    const double distanceToCamera = glm::length(cameraPos - entityPos);
    for (unsigned long i = meshes.size(); i-- > 0;) {
        if (distanceToCamera >= meshes[i].first) {
            if (activeMesh != meshes[i].second) {
                activeMesh = meshes[i].second;
                createVBO();
            }
            break;
        }
    }
}

void MeshComponent::onAttach(Registry& registry, Entity entity){
	if (!texFiles.empty() && !texUniforms.empty()) {

		if(!registry.has<TextureComponent>(entity)){
			auto& textureComponent = registry.emplace<TextureComponent>(entity, texFiles, texUniforms);
			textureComponent.loadTextures();
		}

	}
};

void TextureComponent::loadTextures() {
    textureIDs.resize(texFiles.size());
    for (int i = 0; i < texFiles.size(); i++) {
        int width, height, nrChannels;
        unsigned char* img = stbi_load(("textures/" + texFiles[i]).c_str(), &width, &height, &nrChannels, 0);
        glGenTextures(1, &textureIDs[i]);
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLint type = GL_RGB;
        if (nrChannels == 4) type = GL_RGBA;
        else if (nrChannels == 2) type = GL_RG;
        else if (nrChannels == 1) type = GL_R;

        glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(img);
    }
}

