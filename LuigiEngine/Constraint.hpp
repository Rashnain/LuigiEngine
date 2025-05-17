#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP
#include "ECS.h"
#include "Transform.hpp"
#include "Physics.hpp"

//agit comme un ressort entre deux entites
struct SpringConstraint
{
    Entity entityA;
    Entity entityB;

    vec3 localAnchorA;
    vec3 localAnchorB;

    float initialLength;
    float stiffness;
    float damping;

    void onAttach(Registry & registry, Entity entity);
    void onDetach(Registry & registry, Entity entity);
};

//agit comme un ressort mais attache a une seule entite
struct SuspensionConstraint
{
    Entity entity;

    vec3 localAnchor;
    vec3 direction;

    float initialLength = 1.0f;
    float stiffness;
    float damping;

    float maxForce = 10000.0f;

    bool isColliding;
    vec3 collisionPoint;
    vec3 startPointWorld;
    vec3 endPointWorld;

    void onAttach(Registry& registry, Entity entity);
    void onDetach(Registry& registry, Entity entity);
};

class ConstraintSystem
{
public:

    vector<SpringConstraint> springConstraints;
    vector<SuspensionConstraint> suspensionConstraints;

    ConstraintSystem(){
        springConstraints.reserve(500);
        suspensionConstraints.reserve(500);
    }

    void update(Registry & registry, float deltaTime);

    void updateSprings(Registry& registry, float deltaTime);

    void updateSuspensions(Registry& registry, float deltaTime);

    SuspensionConstraint* addSuspension(Entity entity, vec3 localAnchor, vec3 direction, float initialLength)
    {
        SuspensionConstraint suspension;
        suspension.entity = entity;
        suspension.localAnchor = localAnchor;
        suspension.direction = direction;
        suspension.initialLength = initialLength;

        suspensionConstraints.push_back(suspension);
        return &(suspensionConstraints.back());
    }

    void clear()
    {
        springConstraints.clear();
        suspensionConstraints.clear();
    }
};



#endif