#include "Constraint.hpp"
#include "ImGuiConsole.hpp"

void ConstraintSystem::updateSprings(Registry & registry, float deltaTime) {

}

void ConstraintSystem::updateSuspensions(Registry& registry, float deltaTime){

    auto view = registry.view<RigidBodyComponent,Transform>();

    for( const SuspensionConstraint& suspension : suspensionConstraints){

        RigidBodyComponent& suspensionBody = registry.get<RigidBodyComponent>(suspension.entity);
        Transform& suspensionTransform = registry.get<Transform>(suspension.entity);

        for(Entity entity : view){

            if(entity == suspension.entity) continue;

            RigidBodyComponent& otherBody = registry.get<RigidBodyComponent>(entity);
            Transform& otherTransform = registry.get<Transform>(entity);

            if(suspensionBody.bodyType == PhysicsType::STATIC && otherBody.bodyType == PhysicsType::STATIC) continue;

            for(Collider* collider : otherBody.colliders){

                const Collider& col = *collider;

                if(col.type == ColliderType::OBB){

                    const OBBCollider& obb = (const OBBCollider&) col;

                    CollisionInfo collisionInfo;

                    vec3 globalPos = suspensionTransform.getGlobalModel() * vec4(suspension.localAnchor, 1.0f);
                    vec3 GlobalDirection = normalize( suspensionTransform.getGlobalModel() * vec4(suspension.direction, 0.0f) );
                    
                    Ray globalRay(globalPos, GlobalDirection, suspension.initialLength);

                    CollisionDetection::collision_ray_obb(suspension.entity, globalRay, suspensionTransform, entity, obb, otherTransform, collisionInfo);

                    if(collisionInfo.isColliding){

                        //Console::getInstance().addLog("Suspension collision detected between " + std::to_string(suspension.entity) + " and " + std::to_string(entity));

                        vec3 collisionPoint = collisionInfo.collisionPointA;

                        vec3 worldAnchor = suspensionTransform.getGlobalModel() * vec4(suspension.localAnchor, 1.0f);

                        float currentLength = length(collisionPoint - worldAnchor);
                        float compression = suspension.initialLength - currentLength;
                        if (compression < 0.0f) compression = 0.0f;

                        vec3 forceDir = normalize(suspension.direction);
                        vec3 force = forceDir * (compression * suspension.stiffness);

                        Console::getInstance().addLog("Suspension force: (" + std::to_string(force.x) + ", " + std::to_string(force.y) + ", " + std::to_string(force.z) + ")");

                        if(suspensionBody.bodyType == PhysicsType::DYNAMIC){
                            suspensionBody.addForceAt(-force, worldAnchor);
                        }

                        if(otherBody.bodyType == PhysicsType::DYNAMIC){
                            otherBody.addForceAt(force, collisionPoint);
                        }

                    }
                }
            }


        }

    }

}

void ConstraintSystem::update(Registry& registry, float deltaTime){

    updateSprings(registry, deltaTime);

    updateSuspensions(registry, deltaTime);

}