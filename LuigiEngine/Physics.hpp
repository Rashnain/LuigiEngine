#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "LuigiEngine/SceneMesh.hpp"
#include "Transform.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <utility>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include "ECS.h"
#include "glm/detail/type_vec.hpp"

#include "LuigiEngine/PhysicsUtils.hpp"

using namespace glm;



enum class PhysicsType
{
    STATIC,
    DYNAMIC
};

struct RigidBodyComponent {

    Transform * transform;
    Mesh * mesh; //a enlever

    vec3 linearVelocity;
    vec3 angularVelocity;

    vec3 forceAccumulator;
    vec3 torqueAccumulator;

    float mass;
    float inverseMass;

    mat3 localInertiaTensor; //utile seulement pour la rotation
    mat3 localInverseInertiaTensor;
    mat3 globalInverseInertiaTensor;

    vec3 globalCentroid;
    vec3 localCentroid;

    AABBCollider aabbCollider; //chaque objet a une aabb en coordonée globale

    std::vector<Collider*> colliders;

    PhysicsType bodyType = PhysicsType::DYNAMIC;
    bool isPaused = false;
    bool useGravity = true;

    float linearDamping = 0.0f;
    float angularDamping = 0.0f;

    float restitution = 1.0f;

    RigidBodyComponent(){
        mass = 1.0f;
        inverseMass = 1.0f;
        localInertiaTensor = glm::mat3(0.0f);
        localInverseInertiaTensor = glm::mat3(0.0f);
        globalInverseInertiaTensor = glm::mat3(0.0f);
        globalCentroid = glm::vec3(0.0f);
        localCentroid = glm::vec3(0.0f);
        linearVelocity = glm::vec3(0.0f);
        angularVelocity = glm::vec3(0.0f);
        forceAccumulator = glm::vec3(0.0f);
        torqueAccumulator = glm::vec3(0.0f);
    }   

    void onAttach(Registry & registry, Entity entity){
        Transform& t = registry.get<Transform>(entity);
        transform = &t;
    };
    void onDetach(Registry & registry, Entity entity){};

    void addCollider(Collider* collider);

};



class PhysicsSystem  {

    std::vector<std::pair<Entity, Entity>> collisionPairs;
    std::vector<CollisionInfo> collisionList;

    vec3 gravity = vec3(0,-9.8, 0);

    void recomputeAABB(Registry& registry);

    void integrate(Registry& registry, float deltaTime);

    void narrowCollisionDetection(Entity entityA, RigidBodyComponent& rigidBodyA, Transform& transformA, Entity entityB, RigidBodyComponent& rigidBodyB, Transform& transformB);
    void broadCollisionDetection(Registry& registry);

    void collisionResolution(Registry& registry);

public : 

    void update(Registry& registry, float deltaTime );
    

};


#endif