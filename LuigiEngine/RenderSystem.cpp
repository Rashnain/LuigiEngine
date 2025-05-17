#include "RenderSystem.hpp"
#include "LuigiEngine/ECS.h"
#include "LuigiEngine/SceneCamera.hpp"
#include "LuigiEngine/Physics.hpp"
#include <glm/gtc/type_ptr.hpp>
using namespace glm;




void RenderSystem::setupMeshRendering(const MeshComponent &meshComp,
                                      GLuint programID) {

  glUseProgram(programID);
  glUniformMatrix4fv(glGetUniformLocation(programID, "mvp"), 1, GL_FALSE,
                     &meshComp.mvp[0][0][0]);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, meshComp.vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, meshComp.normalbuffer);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glEnableVertexAttribArray(3);
  glBindBuffer(GL_ARRAY_BUFFER, meshComp.uvbuffer);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void RenderSystem::bindTextureUniforms(const MeshComponent &meshComp,
                                       const TextureComponent &textures) {
  for (int i = 0; i < textures.textureIDs.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, textures.textureIDs[i]);
    glUniform1i(glGetUniformLocation(meshComp.programID,
                                     textures.texUniforms[i].c_str()),
                i);
  }
}

void RenderSystem::renderMesh(const MeshComponent &meshComp, GLuint programID,
                              const TextureComponent *textures) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshComp.elementbuffer);
  glDrawElements(GL_TRIANGLES, meshComp.activeMesh->triangles.size(),
                 GL_UNSIGNED_INT, nullptr);
}

void RenderSystem::render(Registry &registry) {

  CameraComponent &camera = registry.get<CameraComponent>(activeCamera);
  Transform &cameraTransform = registry.get<Transform>(activeCamera);

  vec3 cameraPos = cameraTransform.getPos();

  for (auto entity : registry.view<MeshComponent, Transform>()) {

    // std::cout << "rendering entity " << entity << std::endl;
    MeshComponent &meshComp = registry.get<MeshComponent>(entity);
    Transform &transform = registry.get<Transform>(entity);

    vec3 entityPos = vec3(transform.getGlobalModel()[3]);

    if (camera.viewProjChanged || transform.changed) {
      meshComp.mvp = new mat4(camera.viewProj * transform.getGlobalModel());
      transform.changed = false;
      meshComp.checkLOD(cameraPos, entityPos);
    }

    setupMeshRendering(meshComp, meshComp.programID);

    if (registry.has<TextureComponent>(entity)) {
      TextureComponent &textures = registry.get<TextureComponent>(entity);
      bindTextureUniforms(meshComp, textures);
    }

    renderMesh(meshComp, meshComp.programID,
               registry.has<TextureComponent>(entity)
                   ? &registry.get<TextureComponent>(entity)
                   : nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    
    if(registry.has<RigidBodyComponent>(entity)){
        const RigidBodyComponent &rigidBody = registry.get<RigidBodyComponent>(entity);

        for (const Collider* collider : rigidBody.colliders) {
            if (collider->type == ColliderType::OBB) {
              const OBBCollider& obb = *(const OBBCollider*)(collider);

              WorldOBB wobb;
              CollisionDetection::getWorldOBB(obb, transform, wobb);

              glm::vec3 vertices[8];
              wobb.getVertices(vertices);

              GLuint lines;
              glGenBuffers(1, &lines);
              glBindBuffer(GL_ARRAY_BUFFER, lines);
              glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
              glEnableVertexAttribArray(0);
              glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

              mat4 obbMVP = camera.viewProj;
              GLuint currentProgram = 0;
              glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&currentProgram);
              GLint mvpLoc = glGetUniformLocation(currentProgram, "mvp");
              if (mvpLoc != -1) {
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(obbMVP));
              }

              GLuint indices[] = {
                0,1, 1,3, 3,2, 2,0,
                4,5, 5,7, 7,6, 6,4,
                0,4, 1,5, 2,6, 3,7
              };
              GLuint lineEBO;
              glGenBuffers(1, &lineEBO);
              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
              glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

              glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);

              glDeleteBuffers(1, &lines);
              glDeleteBuffers(1, &lineEBO);
            }
          }
    }

    
  }

  

}
