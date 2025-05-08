#include "Physics.hpp"
#include "LuigiEngine/ECS.h"
#include "LuigiEngine/Transform.hpp"
#include "LuigiEngine/ImGuiConsole.hpp"

void PhysicsSystem::recomputeAABB(Registry& registry){

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entity : view){

        RigidBodyComponent& rigidBody = view.get<RigidBodyComponent>(entity);
        Transform& transform = view.get<Transform>(entity);
        
        vec3 min = transform.getGlobalModel() * vec4(rigidBody.mesh->vertices[0], 1.0f);
        vec3 max = min;

        for(const vec3& vertex : rigidBody.mesh->vertices){
            vec3 globalVertex = transform.getGlobalModel() * vec4(vertex, 1.0f);

            min.x = std::min(min.x, globalVertex.x);
            min.y = std::min(min.y, globalVertex.y);
            min.z = std::min(min.z, globalVertex.z);

            max.x = std::max(max.x, globalVertex.x);
            max.y = std::max(max.y, globalVertex.y);
            max.z = std::max(max.z, globalVertex.z);
        }

        rigidBody.aabbCollider.min = min;
        rigidBody.aabbCollider.max = max;

    }

}

void PhysicsSystem::applyForces(Registry& registry, float deltaTime){

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entity : view){

        RigidBodyComponent& rigidBody = view.get<RigidBodyComponent>(entity);
        Transform& transform = view.get<Transform>(entity);
        
        if(rigidBody.bodyType == PhysicsType::STATIC){
            rigidBody.linearVelocity = vec3(0.0f);
            rigidBody.angularVelocity = vec3(0.0f);
            continue;
        }

        rigidBody.linearVelocity = vec3(0,-3,0);

        transform.addPos(rigidBody.linearVelocity * deltaTime);
    }

}

void PhysicsSystem::broadCollisionDetection(Registry& registry){

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entityA : view){

        RigidBodyComponent& rigidBodyA = view.get<RigidBodyComponent>(entityA);
        Transform& transformA = view.get<Transform>(entityA);

        for(Entity entityB : view){

            if(entityA == entityB) break;

            RigidBodyComponent& rigidBodyB = view.get<RigidBodyComponent>(entityB);
            Transform& transformB = view.get<Transform>(entityB);

            bool collisionX = rigidBodyA.aabbCollider.min.x <= rigidBodyB.aabbCollider.max.x &&
                               rigidBodyA.aabbCollider.max.x >= rigidBodyB.aabbCollider.min.x;

            bool collisionY = rigidBodyA.aabbCollider.min.y <= rigidBodyB.aabbCollider.max.y &&
                               rigidBodyA.aabbCollider.max.y >= rigidBodyB.aabbCollider.min.y;

            bool collisionZ = rigidBodyA.aabbCollider.min.z <= rigidBodyB.aabbCollider.max.z &&
                               rigidBodyA.aabbCollider.max.z >= rigidBodyB.aabbCollider.min.z;

            if (collisionX && collisionY && collisionZ) {
                Console::getInstance().addLog("Collision detected between Entity " + std::to_string(entityA) + 
                                  " and Entity " + std::to_string(entityB));

                narrowCollisionDetection(entityA, rigidBodyA, transformA,entityB, rigidBodyB, transformB);
            }else{

            }


        }

    }


}

void PhysicsSystem::narrowCollisionDetection(Entity entityA, RigidBodyComponent& rigidBodyA, Transform& transformA, Entity entityB ,RigidBodyComponent& rigidBodyB, Transform& transformB){

    CollisionInfo collisionInfo;

    // Compute overlap in each axis
    vec3 overlap;
    overlap.x = std::min(rigidBodyA.aabbCollider.max.x, rigidBodyB.aabbCollider.max.x) - 
                std::max(rigidBodyA.aabbCollider.min.x, rigidBodyB.aabbCollider.min.x);
    overlap.y = std::min(rigidBodyA.aabbCollider.max.y, rigidBodyB.aabbCollider.max.y) - 
                std::max(rigidBodyA.aabbCollider.min.y, rigidBodyB.aabbCollider.min.y);
    overlap.z = std::min(rigidBodyA.aabbCollider.max.z, rigidBodyB.aabbCollider.max.z) - 
                std::max(rigidBodyA.aabbCollider.min.z, rigidBodyB.aabbCollider.min.z);

    // Determine the axis of minimum penetration
    float minOverlap = std::min({overlap.x, overlap.y, overlap.z});
    vec3 collisionNormal;

    if (minOverlap == overlap.x) {
        collisionNormal = vec3((rigidBodyA.aabbCollider.min.x < rigidBodyB.aabbCollider.min.x) ? -1.0f : 1.0f, 0.0f, 0.0f);
    } else if (minOverlap == overlap.y) {
        collisionNormal = vec3(0.0f, (rigidBodyA.aabbCollider.min.y < rigidBodyB.aabbCollider.min.y) ? -1.0f : 1.0f, 0.0f);
    } else {
        collisionNormal = vec3(0.0f, 0.0f, (rigidBodyA.aabbCollider.min.z < rigidBodyB.aabbCollider.min.z) ? -1.0f : 1.0f);
    }

    // Populate collision info
    collisionInfo.normal = collisionNormal;
    collisionInfo.penetrationDepth = minOverlap;

    // Log collision details
    Console::getInstance().addLog("Collision Info: Normal (" + std::to_string(collisionNormal.x) + ", " +
                                  std::to_string(collisionNormal.y) + ", " + std::to_string(collisionNormal.z) +
                                  "), Depth: " + std::to_string(minOverlap));

    
    collisionInfo.entityA = entityA;
    collisionInfo.entityB = entityB;
    collisionInfo.isColliding = true;
    collisionList.push_back(collisionInfo);
}

void PhysicsSystem::collisionResolution(Registry& registry){

    for(CollisionInfo& collision : collisionList){

        RigidBodyComponent& rigidBodyA = registry.get<RigidBodyComponent>(collision.entityA);
        RigidBodyComponent& rigidBodyB = registry.get<RigidBodyComponent>(collision.entityB);

        Transform& transformA = registry.get<Transform>(collision.entityA);
        Transform& transformB = registry.get<Transform>(collision.entityB);

        //Correction de la position
        vec3 correction = collision.normal * collision.penetrationDepth * 0.5f;

        if (rigidBodyA.bodyType != PhysicsType::STATIC && rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction);
            transformB.addPos(-correction);
        } else if (rigidBodyA.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction * 2.0f);
        } else if (rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformB.addPos(-correction * 2.0f);
        }

    }

    collisionList.clear();

}

void PhysicsSystem::update(Registry& registry, float deltaTime) {

    recomputeAABB(registry);

    applyForces(registry, deltaTime);

    broadCollisionDetection(registry);

    collisionResolution(registry);

}

