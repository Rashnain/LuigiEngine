#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ECS.h"

#include "SceneMesh.hpp"
#include "Transform.hpp"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

class RenderSystem {
public:
    Entity activeCamera = INVALID;

    void render(Registry& registry);

private:
    void setupMeshRendering(const MeshComponent& meshComp, GLuint programID);
    void bindTextureUniforms(const MeshComponent& meshComp, const TextureComponent& textures);
    void renderMesh(const MeshComponent& meshComp, GLuint programID, const TextureComponent* textures);
};

#endif // RENDERSYSTEM_H
