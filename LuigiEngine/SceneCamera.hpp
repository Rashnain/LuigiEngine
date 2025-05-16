#pragma once

#include <glm/glm.hpp>
#include "ECS.h"
#include "Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>


struct CameraComponent {
    mat4 projection{1.0f};
    mat4 view{1.0f};
    mat4 viewProj{1.0f};

    vec3 target{0, 0, -1};
    vec3 up{0, 1, 0};
    vec3 right = normalize(cross(target, up));

    bool viewProjChanged = true;
    bool justDefinedMain = true;
    float speed = 1.0f;

    CameraComponent() = default;
    explicit CameraComponent(const mat4& proj) : projection(proj) {}

    void onAttach(Registry & registry, Entity entity){};
    void onDetach(Registry & registry, Entity entity){};
};


class CameraSystem {
public:
    void update(Registry& registry);
    void computeViewProj(Registry& registry);

private:
    static void updateView( Transform& transform, CameraComponent& camera);
};
