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

    float initialLength;
    float stiffness;
    float damping;

    void onAttach(Registry& registry, Entity entity);
    void onDetach(Registry& registry, Entity entity);
};

class ConstraintSystem
{
public:

    vector<SpringConstraint> springConstraints;
    vector<SuspensionConstraint> suspensionConstraints;

    void update(Registry & registry, float deltaTime);

    void updateSprings(Registry& registry, float deltaTime);

    void updateSuspensions(Registry& registry, float deltaTime);

};



#endif