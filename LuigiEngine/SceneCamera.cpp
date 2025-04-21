#include "SceneCamera.hpp"

extern int* nbViewProjUpdate;
extern bool* optimizeMVP;

void CameraSystem::update(Registry& registry) {
    auto view = registry.view<CameraComponent, Transform>();

    for (auto entity : view) {
        auto& camera = registry.get<CameraComponent>(entity);
        auto& transform = registry.get<Transform>(entity);

        if (!camera.justDefinedMain)
            camera.viewProjChanged = false;
        else
            camera.justDefinedMain = false;

        if (!transform.upToDateGlobal)
            camera.viewProjChanged = true;
    }
}

void CameraSystem::computeViewProj(Registry& registry) {
    auto view = registry.view<CameraComponent, Transform>();

    for (auto entity : view) {
        auto& camera = registry.get<CameraComponent>(entity);
        auto& transform = registry.get<Transform>(entity);

        if (!*optimizeMVP || transform.changed) {
            updateView(transform, camera);
            camera.viewProj = camera.projection * camera.view;
            camera.viewProjChanged = true;
            transform.changed = false;
            ++(*nbViewProjUpdate);
        }
    }
}

void CameraSystem::updateView( Transform& transform, CameraComponent& camera) {
    glm::vec3 globalPos = glm::vec3(transform.getGlobalModel()[3]);

    glm::mat3 globalRot = glm::mat3(transform.getGlobalModel());
    globalRot[0] = glm::normalize(globalRot[0]);
    globalRot[1] = glm::normalize(globalRot[1]);
    globalRot[2] = glm::normalize(globalRot[2]);

    glm::vec3 globalTarget = glm::normalize(globalRot * camera.target);
    glm::vec3 globalUp = glm::normalize(globalRot * camera.up);

    camera.view = glm::lookAt(globalPos, globalPos + globalTarget, globalUp);
}
