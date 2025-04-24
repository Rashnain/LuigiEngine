#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>             // Core GLM functionality
#include <glm/gtc/matrix_transform.hpp>  // For transformations
#include <glm/gtc/quaternion.hpp>  // For quaternion rotation

using namespace glm;

class Transform
{
    bool upToDateLocal = false;
    vec3 localPos{0.0f, 0.0f, 0.0f};
    vec3 localEulerRot{0.0f, 0.0f, 0.0f};
    mat4 localRot;
    vec3 localScale{1.0f, 1.0f, 1.0f};
    mat4 localModel{1.0f};
    mat4 globalModel{1.0f};

    void computeLocalModelMatrix();

public:
    bool upToDateGlobal = false;
    bool changed = true;

    bool isUpToDateLocal() { return upToDateLocal; }
    vec3 getPos() { return localPos; }
    void addPos(const vec3& vec);
    void setPos(vec3 vec);
    vec3 getEulerRot() { return localEulerRot; }
    void addEulerRot(const vec3& vec);
    void setEulerRot(vec3 vec);
    mat4 getRot() { return localRot; }
    void setRot(const quat& quatRot);
    vec3 getScale() { return localScale; }
    void addScale(const vec3& vec);
    void setScale(vec3 vec);
    mat4 getGlobalModel() { return globalModel; }
    void computeGlobalModelMatrix();
    void computeGlobalModelMatrix(const mat4& parentModel);
};

#endif //TRANSFORM_H
