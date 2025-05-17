#include "RenderSystem.hpp"
#include "LuigiEngine/ECS.h"
#include "LuigiEngine/SceneCamera.hpp"

using namespace glm;

void RenderSystem::setupMeshRendering(const MeshComponent &meshComp, Transform &meshTransform, Transform &camTransform) {

  glUseProgram(meshComp.programID);
  glUniformMatrix4fv(glGetUniformLocation(meshComp.programID, "mvp"), 1, GL_FALSE,
                     &meshComp.mvp[0][0][0]);

  // PBR shader
  if (!meshComp.material.empty()) {
    vector<vec3> lights = {{0.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, -10.0f, 0.0f}};
    vector<vec3> lightColors = {{255.0f, 0.0f, 0.0f}, {0.0f, 255.0f, 0.0f}, {0.0f, 0.0f, 255.0f}};
    vec3 cameraPos = vec3(camTransform.getGlobalModel()[3]);

    mat3 globalRot = mat3(meshTransform.getGlobalModel());
    globalRot[0] = normalize(globalRot[0]);
    globalRot[1] = normalize(globalRot[1]);
    globalRot[2] = normalize(globalRot[2]);

    glUniformMatrix4fv(glGetUniformLocation(meshComp.programID, "model"), 1, GL_FALSE, &meshTransform.getGlobalModel()[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(meshComp.programID, "rotation"), 1, GL_FALSE, &globalRot[0][0]);
    glUniform3fv(glGetUniformLocation(meshComp.programID, "lightPositions"), lights.size(), &lights[0][0]);
    glUniform3fv(glGetUniformLocation(meshComp.programID, "lightColors"), lightColors.size(), &lightColors[0][0]);
    glUniform3f(glGetUniformLocation(meshComp.programID, "camPos"), cameraPos[0], cameraPos[1], cameraPos[2]);
  }

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

void RenderSystem::renderMesh(const MeshComponent &meshComp,
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

    setupMeshRendering(meshComp, transform, cameraTransform);

    if (registry.has<TextureComponent>(entity)) {
      TextureComponent &textures = registry.get<TextureComponent>(entity);
      bindTextureUniforms(meshComp, textures);
    }

    renderMesh(meshComp,
               registry.has<TextureComponent>(entity)
                   ? &registry.get<TextureComponent>(entity)
                   : nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
  }
}
