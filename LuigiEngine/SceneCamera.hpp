#pragma once

#include <glm/glm.hpp>
#include "ECS.h"
#include "Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>


struct CameraComponent {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 viewProj{1.0f};

    glm::vec3 target{0, 0, -1};
    glm::vec3 up{0, 1, 0};
    glm::vec3 right = glm::normalize(glm::cross(target, up));

    bool viewProjChanged = true;
    bool justDefinedMain = true;
    float speed = 1.0f;

    CameraComponent() = default;
    explicit CameraComponent(const glm::mat4& proj) : projection(proj) {}

    void onAttach(Registry & registry, Entity entity){};
    void onDetach(Registry & registry, Entity entity){};
};


class CameraSystem {
public:
    void update(Registry& registry, float deltaTime);
    void computeViewProj(Registry& registry);

private:
    static void updateView( Transform& transform, CameraComponent& camera);
};
