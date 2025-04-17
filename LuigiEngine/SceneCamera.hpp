#ifndef SCENECAMERA_H
#define SCENECAMERA_H

#pragma once

#include "SceneObject.hpp"

class SceneCamera : public SceneObject
{
    mat4 projection;
    mat4 view;
    mat4 viewProj;
    vec3 target{0, 0, -1};
    vec3 up{0, 1, 0};
    vec3 right;

    void updateView();

protected:
    void update(double deltaTime) override;

public:
    bool viewProjChanged = true;
    bool justDefinedMain = true;
    float speed = 1;

    explicit SceneCamera(mat4& projection);

    mat4 getViewProjMatrix();

    vec3 getLocalTarget();

    vec3 getLocalUp();

    vec3 getLocalRight();

    void setProjection(mat4& projection);
};

#endif //SCENECAMERA_H
