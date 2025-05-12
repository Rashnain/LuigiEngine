#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include "ECS.h"

using namespace glm;

class Transform
{
    bool upToDateLocal = false;
    vec3 localPos{0.0f, 0.0f, 0.0f};
    vec3 localEulerRot{0.0f, 0.0f, 0.0f};
    mat4 localRot;
    vec3 localScale{1.0f, 1.0f, 1.0f};
    mat4 localModel{1.0f};
    

    

public:

    void onAttach(Registry & registry, Entity entity){};
    void onDetach(Registry & registry, Entity entity){};

    bool upToDateGlobal = false;
    bool changed = true;

    void computeLocalModelMatrix();
    mat4 globalModel{1.0f};

    bool isUpToDateLocal() { return upToDateLocal; }
    vec3 getPos() { return localPos; }
    vec3 getPos() const { return localPos; }
    void addPos(const vec3& vec);
    void setPos(vec3 vec);
    vec3 getEulerRot() { return localEulerRot; }
    void addEulerRot(const vec3& vec);
    void setEulerRot(vec3 vec);
    mat4 getRot() { return localRot; }
    void setRot(const quat& quatRot);
    vec3 getScale() { return localScale; }
    vec3 getScale() const { return localScale; }
    void addScale(const vec3& vec);
    void setScale(vec3 vec);
    inline mat4 getGlobalModel() { return globalModel; }
    inline mat4 getGlobalModel() const { return globalModel; }
    void computeGlobalModelMatrix();
    void computeGlobalModelMatrix(const mat4& parentModel);

    vec3 getRight() const {
        return vec3(globalModel[0][0], globalModel[1][0], globalModel[2][0]);
    }
    vec3 getUp() const {
        return vec3(globalModel[0][1], globalModel[1][1], globalModel[2][1]);
    }
    vec3 getFront() const {
        return vec3(globalModel[0][2], globalModel[1][2], globalModel[2][2]);
    }
};


//pour l' heritage des transformations
struct Hierarchy {
    std::string name = "default_name";
    Entity parent = INVALID;
    std::vector<Entity> children;

    Hierarchy(Entity parent, std::vector<Entity>  children) : parent(parent), children(children){}

    Hierarchy(std::vector<Entity> children) : children(children){}

    Hierarchy() = default;

    void addChild(Entity child) {
        children.push_back(child);
    }

    void onAttach(Registry& registry, Entity entity) {
        name = "Entity " + std::to_string(entity);
        if (parent != INVALID) {
            if (registry.has<Hierarchy>(parent)) {
                registry.get<Hierarchy>(parent).children.push_back(entity);
            } else {
                registry.emplace<Hierarchy>(parent).children.push_back(entity);
            }
        }
    }

    void onDetach(Registry& registry, Entity entity) {
        if (parent != INVALID && registry.has<Hierarchy>(parent)) {
            auto& siblings = registry.get<Hierarchy>(parent).children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
        }
        parent = INVALID;
    }
};


class TransformSystem{
public:
    void update(Registry & registry);

private:
    void computeGlobalTransform(Entity entity, Registry & registry, const glm::mat4 & parentModel);
};

#endif //TRANSFORM_H
