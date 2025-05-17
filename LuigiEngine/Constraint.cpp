#include "Constraint.hpp"
#include "ImGuiConsole.hpp"

void ConstraintSystem::updateSprings(Registry & registry, float deltaTime) {

}

void ConstraintSystem::updateSuspensions(Registry& registry, float deltaTime) {
    auto view = registry.view<RigidBodyComponent, Transform>();

    for (SuspensionConstraint& suspension : suspensionConstraints) {
        RigidBodyComponent& suspensionBody = registry.get<RigidBodyComponent>(suspension.entity);
        Transform& suspensionTransform = registry.get<Transform>(suspension.entity);

        vec3 worldAnchor = suspensionTransform.getGlobalModel() * vec4(suspension.localAnchor, 1.0f);
        vec3 globalDirection = normalize(suspensionTransform.getGlobalModel() * vec4(suspension.direction, 0.0f));

        vec3 endPointWorld = worldAnchor + globalDirection * suspension.initialLength;
        suspension.startPointWorld = worldAnchor;
        suspension.endPointWorld = endPointWorld;
        suspension.isColliding = false;

        // Closest collision tracking
        float minDistance = suspension.initialLength;
        vec3 closestCollisionPoint = vec3(0.0f);
        Entity closestEntity = INVALID;
        bool foundCollision = false;

        Ray globalRay(worldAnchor, globalDirection, suspension.initialLength);

        for (Entity entity : view) {
            if (entity == suspension.entity) continue;

            RigidBodyComponent& otherBody = registry.get<RigidBodyComponent>(entity);
            Transform& otherTransform = registry.get<Transform>(entity);

            if (suspensionBody.bodyType == PhysicsType::STATIC && otherBody.bodyType == PhysicsType::STATIC)
                continue;

            for (Collider* collider : otherBody.colliders) {
                if (!collider || collider->type != ColliderType::OBB) continue;

                const OBBCollider& obb = static_cast<const OBBCollider&>(*collider);
                CollisionInfo collisionInfo;

                CollisionDetection::collision_ray_obb(
                    suspension.entity, globalRay, suspensionTransform,
                    entity, obb, otherTransform, collisionInfo
                );

                if (!collisionInfo.isColliding) continue;

                float distance = length(collisionInfo.collisionPointA - worldAnchor);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestCollisionPoint = collisionInfo.collisionPointA;
                    closestEntity = entity;
                    foundCollision = true;
                }
            }
        }

        if (!foundCollision) continue;

        suspension.isColliding = true;
        suspension.collisionPoint = closestCollisionPoint;

        float currentLength = length(closestCollisionPoint - worldAnchor);
        float compression = suspension.initialLength - currentLength;
        if (compression < 0.0f) compression = 0.0f;

        vec3 forceDir = globalDirection;

        //spring
        vec3 force = forceDir * (compression * suspension.stiffness);

        //damping
        vec3 velocity = suspensionBody.getVelocityAt(worldAnchor);
        float dampingStrength = dot(velocity, forceDir);
        vec3 damping = forceDir * (dampingStrength * suspension.damping);
        vec3 totalForce = force + damping;

        if (totalForce.length() > suspension.maxForce) {
            totalForce = normalize(totalForce) * suspension.maxForce;
        }

        Console::getInstance().addLog("Suspension force: (" +
            std::to_string(force.x) + ", " +
            std::to_string(force.y) + ", " +
            std::to_string(force.z) + ")");

        if (suspensionBody.bodyType == PhysicsType::DYNAMIC) {
            suspensionBody.addForceAt(-totalForce, worldAnchor);
        }

        RigidBodyComponent& otherBody = registry.get<RigidBodyComponent>(closestEntity);
        if (otherBody.bodyType == PhysicsType::DYNAMIC) {
            otherBody.addForceAt(totalForce, closestCollisionPoint);
        }
    }
}



void ConstraintSystem::update(Registry& registry, float deltaTime){

    updateSprings(registry, deltaTime);

    updateSuspensions(registry, deltaTime);

}