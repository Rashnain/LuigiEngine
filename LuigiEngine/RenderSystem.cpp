#include "RenderSystem.hpp"


using namespace glm;

void RenderSystem::setupMeshRendering(const MeshComponent& meshComp, GLuint programID) {
    glUseProgram(programID);
    glUniformMatrix4fv(glGetUniformLocation(programID, "mvp"), 1, GL_FALSE, &meshComp.mvp[0][0][0]);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, meshComp.vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, meshComp.normalbuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void RenderSystem::bindTextureUniforms(const MeshComponent& meshComp, const TextureComponent& textures) {
    for (int i = 0; i < textures.textureIDs.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures.textureIDs[i]);
        glUniform1i(glGetUniformLocation(meshComp.programID, textures.texUniforms[i].c_str()), i);
    }
}

void RenderSystem::renderMesh(const MeshComponent& meshComp, GLuint programID, const TextureComponent* textures) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshComp.elementbuffer);
    glDrawElements(GL_TRIANGLES, meshComp.activeMesh->triangles.size(), GL_UNSIGNED_INT, nullptr);
}

void RenderSystem::render(Registry& registry) {
    vec3 cameraPos = vec3(registry.get<Transform>(activeCamera).getGlobalModel()[3]);

    for (auto entity : registry.view<MeshComponent, Transform>()) {

        MeshComponent& meshComp = registry.get<MeshComponent>(entity);
        Transform& transform = registry.get<Transform>(entity);
        
        vec3 entityPos = vec3(transform.getGlobalModel()[3]);
        meshComp.checkLOD(cameraPos, entityPos);
        
        setupMeshRendering(meshComp, meshComp.programID);
        
        if (registry.has<TextureComponent>(entity)) {
            TextureComponent& textures = registry.get<TextureComponent>(entity);
            bindTextureUniforms(meshComp, textures);
        }
        
        renderMesh(meshComp, meshComp.programID, registry.has<TextureComponent>(entity) ? &registry.get<TextureComponent>(entity) : nullptr);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(2);
    }
}
