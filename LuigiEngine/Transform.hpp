#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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


//pour l' heritage des transformations
struct Hierarchy {
    Entity parent = INVALID;
    std::vector<Entity> children;

    Hierarchy(Entity parent, std::vector<Entity>  children) : parent(parent), children(children){}

    Hierarchy(std::vector<Entity> children) : children(children){}

    Hierarchy() = default;

    void addChild(Entity child) {
        children.push_back(child);
    }

    void onAttach(Registry& registry, Entity entity) {
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
