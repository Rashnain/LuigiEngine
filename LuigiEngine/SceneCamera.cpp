#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

extern int* nbViewProjUpdate;
extern bool* optimizeMVP;

#include "SceneCamera.hpp"

void SceneCamera::updateView() {
    vec3 globalPos = vec3(transform.getGlobalModel()[3]);

    mat3 globalRot = mat3(transform.getGlobalModel());
    globalRot[0] = normalize(globalRot[0]);
    globalRot[1] = normalize(globalRot[1]);
    globalRot[2] = normalize(globalRot[2]);

    vec3 globalTarget = normalize(globalRot * target);
    vec3 globalUp = normalize(globalRot * up);

    // TODO [TP01] Camera - (Model) View (Projection)
    // View matrix : camera/view transformation lookat() utiliser position target up
    view = lookAt(globalPos, globalPos+globalTarget, globalUp);
}

void SceneCamera::update(double deltaTime) {
    if (!justDefinedMain)
        viewProjChanged = false;
    else
        justDefinedMain = false;

    if (!transform.upToDateGlobal)
        viewProjChanged = true;
}

SceneCamera::SceneCamera(mat4& projection) {
    right = cross(target, up);
    this->projection = projection;
}

mat4 SceneCamera::getViewProjMatrix() {
    if (!*optimizeMVP || transform.changed) {
        updateView();
        viewProj = projection * view;
        nbViewProjUpdate = new int(*nbViewProjUpdate + 1);
        viewProjChanged = true;
        transform.changed = false;
    }
    return viewProj;
}

vec3 SceneCamera::getLocalTarget() {
    return mat3(transform.getRot()) * target;
}

vec3 SceneCamera::getLocalUp() {
    return mat3(transform.getRot()) * up;
}

vec3 SceneCamera::getLocalRight() {
    return mat3(transform.getRot()) * right;
}

void SceneCamera::setProjection(mat4& projection) {
    this->projection = projection;
    viewProj = projection * view; // TODO [Camera] tester avec caméra orthogonale
    viewProjChanged = true;
}
