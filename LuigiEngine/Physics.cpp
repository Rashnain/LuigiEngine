#include "Physics.hpp"
#include "LuigiEngine/ECS.h"
#include "LuigiEngine/Transform.hpp"
#include "LuigiEngine/ImGuiConsole.hpp"
#include "LuigiEngine/PhysicsUtils.hpp"

void RigidBodyComponent::addCollider(Collider* collider){
    assert(collider != nullptr);
    colliders.push_back(collider);

    localCentroid = vec3(0.0f);
    for(Collider* colptr : colliders){
        const Collider& col = *colptr;

        localCentroid += col.mass * col.localCentroid;

        mass += col.mass;
    }

    inverseMass = 1.0f / mass;

    localCentroid *= inverseMass;

    localInertiaTensor = mat3(0.0f);

    for(Collider* colptr : colliders){
        const Collider& col = *colptr;

        const vec3 r = localCentroid - col.localCentroid;
        const float rDotR = dot(r,r);
        const mat3 rOutR = outerProduct(r,r);

        localInertiaTensor += col.localInertiaTensor + col.mass * (rDotR * mat3(1.0f) - rOutR);
    }

    localInverseInertiaTensor = inverse(localInertiaTensor);
}

//recalcule l'aabb a partir de la liste des colliders de l'objet
void PhysicsSystem::recomputeAABB(Registry& registry){

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entity : view){

        RigidBodyComponent& rigidBody = view.get<RigidBodyComponent>(entity);
        Transform& transform = view.get<Transform>(entity);

        vec3 min = vec3(std::numeric_limits<float>::max());
        vec3 max = vec3(std::numeric_limits<float>::lowest());

        for(Collider* colptr : rigidBody.colliders){

            const Collider& collider = *colptr;

            if(collider.type == ColliderType::OBB){
                const OBBCollider& obb = (const OBBCollider&) collider;
                vec3 vertices[8];
                vec3 globalPos = transform.getGlobalModel() * vec4(obb.localCentroid,1.0f);
                obb.getVertices(globalPos, transform.getRight(), transform.getUp(), transform.getFront(),vertices);

                for (const vec3& vertex : vertices) {
                    min.x = std::min(min.x, vertex.x);
                    min.y = std::min(min.y, vertex.y);
                    min.z = std::min(min.z, vertex.z);

                    max.x = std::max(max.x, vertex.x);
                    max.y = std::max(max.y, vertex.y);
                    max.z = std::max(max.z, vertex.z);
                }

                float offset = 0.5f; //on veut que l'aabb soit un peu plus grande
                rigidBody.aabbCollider.min = min - vec3(offset);
                rigidBody.aabbCollider.max = max + vec3(offset);
            }else if(collider.type == ColliderType::SPHERE){

                const SphereCollider& sphere = (const SphereCollider&) collider;

                vec3 sphereCenter = transform.getGlobalModel() * vec4(sphere.localCentroid, 1.0f); 
                float sphereRadius = sphere.radius *transform.getScale().x;
                min.x = std::min(min.x, sphereCenter.x - sphereRadius);
                min.y = std::min(min.y, sphereCenter.y - sphereRadius);
                min.z = std::min(min.z, sphereCenter.z - sphereRadius);

                max.x = std::max(max.x, sphereCenter.x + sphereRadius);
                max.y = std::max(max.y, sphereCenter.y + sphereRadius);
                max.z = std::max(max.z, sphereCenter.z + sphereRadius);

                rigidBody.aabbCollider.min = min;
                rigidBody.aabbCollider.max = max;

            }else if(collider.type == ColliderType::PLANE){

                const PlaneCollider& plane = (const PlaneCollider&) collider;
                vec3 planePosition = transform.getGlobalModel() * vec4(plane.localCentroid, 1.0f);
                vec3 planeNormal = transform.getGlobalModel() * vec4(plane.normal, 0.0f);
                planeNormal = normalize(planeNormal);

                vec3 maxValue = vec3(std::numeric_limits<float>::max());
                vec3 minValue = vec3(std::numeric_limits<float>::lowest());

                auto perpendicular = [&](const vec3& normal){
                    if(abs(normal.x) <= abs(normal.y) && abs(normal.x) <= abs(normal.z)){
                        return vec3(0, -normal.z, normal.y);
                    }else if (abs(normal.y <= abs(normal.z))){
                        return vec3(-normal.z, 0, normal.x);
                    }else{
                        return vec3(-normal.y, normal.x, 0);
                    }
                };

                vec3 tangent = normalize(perpendicular(planeNormal));
                vec3 bitangent = normalize(cross(planeNormal, tangent));

                vec3 corner1 = planePosition + tangent * maxValue + bitangent * maxValue;
                vec3 corner2 = planePosition + tangent * maxValue + bitangent * minValue;
                vec3 corner3 = planePosition + tangent * minValue + bitangent * maxValue;
                vec3 corner4 = planePosition + tangent * minValue + bitangent * minValue;

                vec3 corners[4] = { corner1, corner2, corner3, corner4 };

                for (const vec3& corner : corners) {
                    min.x = std::min(min.x, corner.x);
                    min.y = std::min(min.y, corner.y);
                    min.z = std::min(min.z, corner.z);

                    max.x = std::max(max.x, corner.x);
                    max.y = std::max(max.y, corner.y);
                    max.z = std::max(max.z, corner.z);
                }
 
                 rigidBody.aabbCollider.min = min;
                 rigidBody.aabbCollider.max = max;

            }else if(collider.type == ColliderType::CYLINDER){

                const CylinderCollider& cylinderCollider = (const CylinderCollider&) collider;

                vec3 worldAxis = transform.getGlobalModel() * vec4(cylinderCollider.axis,0.0f);
                worldAxis = normalize(worldAxis);
                vec3 worldPos = transform.getGlobalModel() * vec4(cylinderCollider.localCentroid, 1.0f);
                float worldRadius = cylinderCollider.radius * std::max(std::max(transform.getScale().x, transform.getScale().y), transform.getScale().z);
                float worldHeight = cylinderCollider.halfSize * std::max(std::max(transform.getScale().x, transform.getScale().y), transform.getScale().z);

                vec3 start = worldPos + worldAxis * worldHeight;
                vec3 end = worldPos - worldAxis * worldHeight;

                min.x = std::min(start.x, end.x) - worldRadius;
                min.y = std::min(min.y, std::min(start.y, end.y) - worldRadius);
                min.z = std::min(min.z, std::min(start.z, end.z) - worldRadius);

                max.x = std::max(max.x, std::max(start.x, end.x) + worldRadius);
                max.y = std::max(max.y, std::max(start.y, end.y) + worldRadius);
                max.z = std::max(max.z, std::max(start.z, end.z) + worldRadius);

                rigidBody.aabbCollider.min = min;
                rigidBody.aabbCollider.max = max;



            }

        }

    }

}

void PhysicsSystem::integrate(Registry& registry, float deltaTime){

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entity : view){

        RigidBodyComponent& rigidBody = view.get<RigidBodyComponent>(entity);
        Transform& transform = view.get<Transform>(entity);

        //-----
        mat3 rotation = mat3(transform.getRot());
        rigidBody.globalInverseInertiaTensor = rotation * rigidBody.localInverseInertiaTensor * transpose(rotation);         
        rigidBody.globalCentroid = transform.getGlobalModel() * vec4(rigidBody.localCentroid, 1.0f);
        //-----
        
        if(rigidBody.isPaused) continue;

        if(rigidBody.bodyType == PhysicsType::STATIC){
            rigidBody.linearVelocity = vec3(0.0f);
            rigidBody.angularVelocity = vec3(0.0f);
            continue;
        }

        rigidBody.linearVelocity *= (1.0f - rigidBody.linearDamping * deltaTime);
        vec3 acceleration = rigidBody.forceAccumulator * rigidBody.inverseMass;
        rigidBody.linearVelocity += acceleration * deltaTime;
        rigidBody.linearVelocity += gravity * deltaTime;

        if(length(rigidBody.linearVelocity) > 0.01){ //permet de limiter les tremblements
            transform.addPos(rigidBody.linearVelocity * deltaTime);
        }

        vec3 angularAcceleration = rigidBody.globalInverseInertiaTensor * rigidBody.torqueAccumulator;
        rigidBody.angularVelocity *= (1.0f - rigidBody.angularDamping * deltaTime);
        rigidBody.angularVelocity += angularAcceleration * deltaTime;

        if (length(rigidBody.angularVelocity) > 0.00f) {
            //continue;
            float angle = length(rigidBody.angularVelocity) * deltaTime;
            vec3 axis = normalize(rigidBody.angularVelocity);
            quat deltaRotation = angleAxis(angle, axis);
            quat currentRotation = transform.getRot();
            quat newRotation = currentRotation * deltaRotation;
            transform.setRot(normalize(newRotation));
        }
        

        rigidBody.forceAccumulator = vec3(0.0f);
        rigidBody.torqueAccumulator = vec3(0.0f);
    }

}

//utilise les aabb classique
void PhysicsSystem::broadCollisionDetection(Registry& registry){

    collisionList.clear();
    Console::getInstance().clearLogs();

    auto view = registry.view<RigidBodyComponent, Transform>();

    for(Entity entityA : view){

        RigidBodyComponent& rigidBodyA = view.get<RigidBodyComponent>(entityA);
        Transform& transformA = view.get<Transform>(entityA);

        for(Entity entityB : view){

            if(entityA == entityB) break;

            RigidBodyComponent& rigidBodyB = view.get<RigidBodyComponent>(entityB);
            Transform& transformB = view.get<Transform>(entityB);

            if (rigidBodyA.bodyType == PhysicsType::STATIC && rigidBodyB.bodyType == PhysicsType::STATIC){
                //Console::getInstance().addLog("skipping");
                continue;
            }

            bool collisionX = rigidBodyA.aabbCollider.min.x <= rigidBodyB.aabbCollider.max.x &&
                               rigidBodyA.aabbCollider.max.x >= rigidBodyB.aabbCollider.min.x;

            bool collisionY = rigidBodyA.aabbCollider.min.y <= rigidBodyB.aabbCollider.max.y &&
                               rigidBodyA.aabbCollider.max.y >= rigidBodyB.aabbCollider.min.y;

            bool collisionZ = rigidBodyA.aabbCollider.min.z <= rigidBodyB.aabbCollider.max.z &&
                               rigidBodyA.aabbCollider.max.z >= rigidBodyB.aabbCollider.min.z;

            if (collisionX && collisionY && collisionZ) {
                //Console::getInstance().addLog("broad phase collision e " + std::to_string(entityA) + " e " + std::to_string(entityB));

                narrowCollisionDetection(entityA, rigidBodyA, transformA,entityB, rigidBodyB, transformB);
            }else{
                //Console::getInstance().addLog("no broad Collision");
            }


        }

    }


}

void PhysicsSystem::narrowCollisionDetection(Entity entityA, RigidBodyComponent& rigidBodyA, Transform& transformA, Entity entityB ,RigidBodyComponent& rigidBodyB, Transform& transformB){

    for(const Collider* colliderAptr : rigidBodyA.colliders){
        for(const Collider* colliderBptr : rigidBodyB.colliders){
            CollisionInfo collisionInfo;
            collisionInfo.entityA = entityA;
            collisionInfo.entityB = entityB;

            const Collider& colliderA = *colliderAptr;
            const Collider& colliderB = *colliderBptr;

            CollisionDetection::testCollision(entityA, colliderA, transformA, entityB, colliderB, transformB, collisionInfo);
            if(collisionInfo.isColliding){
                collisionList.push_back(collisionInfo);
                //Console::getInstance().addLog("collision narrow phase e " + std::to_string(entityA) + " e " + std::to_string(entityB));

                //Console::getInstance().addLog("collision info normal = (" + std::to_string(collisionInfo.normal.x) + ", " + std::to_string(collisionInfo.normal.y) + ", " + std::to_string(collisionInfo.normal.z) + "), pen depth = " + std::to_string(collisionInfo.penetrationDepth));
            }else{
                //Console::getInstance().addLog("no narrow collision");
            }
        }
    }

}

void PhysicsSystem::collisionResolution(Registry& registry) {
    for (CollisionInfo& collision : collisionList) {
        RigidBodyComponent& rigidBodyA = registry.get<RigidBodyComponent>(collision.entityA);
        RigidBodyComponent& rigidBodyB = registry.get<RigidBodyComponent>(collision.entityB);

        Transform& transformA = registry.get<Transform>(collision.entityA);
        Transform& transformB = registry.get<Transform>(collision.entityB);

        vec3 correction = collision.normal * collision.penetrationDepth * 0.5f;

        if (rigidBodyA.bodyType != PhysicsType::STATIC && rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction);
            transformB.addPos(-correction);
        } else if (rigidBodyA.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction * 2.0f);
        } else if (rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformB.addPos(-correction * 2.0f);
        }

        vec3 rA = collision.collisionPointA - rigidBodyA.globalCentroid;
        vec3 rB = collision.collisionPointB - rigidBodyB.globalCentroid;

        vec3 vA = rigidBodyA.linearVelocity + cross(rigidBodyA.angularVelocity, rA);
        vec3 vB = rigidBodyB.linearVelocity + cross(rigidBodyB.angularVelocity, rB);
        vec3 relativeVelocity = vB - vA;
        float velocityAlongNormal = dot(relativeVelocity, collision.normal);

        if (velocityAlongNormal < 0)
            continue;

        vec3 raCrossN = cross(rA, collision.normal);
        vec3 rbCrossN = cross(rB, collision.normal);

        vec3 IA_raCrossN = rigidBodyA.globalInverseInertiaTensor * raCrossN;
        vec3 IB_rbCrossN = rigidBodyB.globalInverseInertiaTensor * rbCrossN;

        float denom = rigidBodyA.inverseMass + rigidBodyB.inverseMass + dot(cross(IA_raCrossN, rA) + cross(IB_rbCrossN, rB), collision.normal);

        float restitution = std::min(rigidBodyA.restitution, rigidBodyB.restitution);
        float impulseStrength = -(1.0f + restitution) * velocityAlongNormal / denom;

        vec3 impulse = impulseStrength * collision.normal;


        float linearDenom = rigidBodyA.inverseMass + rigidBodyB.inverseMass;
        float linearImpulseStrength = -(1.0f + restitution) * velocityAlongNormal / linearDenom;
        vec3 linearImpulse = linearImpulseStrength * collision.normal;
        impulse = linearImpulse;

        if (rigidBodyA.bodyType != PhysicsType::STATIC) {
            rigidBodyA.linearVelocity -= impulse * rigidBodyA.inverseMass;
            rigidBodyA.angularVelocity -= IA_raCrossN * impulseStrength;
        }
        if (rigidBodyB.bodyType != PhysicsType::STATIC) {
            rigidBodyB.linearVelocity += impulse * rigidBodyB.inverseMass;
            rigidBodyB.angularVelocity += IB_rbCrossN * impulseStrength;
        }

        vec3 tangent = relativeVelocity - dot(relativeVelocity, collision.normal) * collision.normal;
        if (length(tangent) > 0.1f) tangent = normalize(tangent);
        else continue;

        vec3 raCrossT = cross(rA, tangent);
        vec3 rbCrossT = cross(rB, tangent);

        vec3 IA_raCrossT = rigidBodyA.globalInverseInertiaTensor * raCrossT;
        vec3 IB_rbCrossT = rigidBodyB.globalInverseInertiaTensor * rbCrossT;

        float denomFriction = rigidBodyA.inverseMass + rigidBodyB.inverseMass + dot(cross(IA_raCrossT, rA) + cross(IB_rbCrossT, rB), tangent);

        float impulseFriction;
        vec3 frictionImpulse = vec3(0.0f);
        if (denomFriction > 0.0f) {
            //Console::getInstance().addLog("applying friction ");
            impulseFriction = -dot(relativeVelocity, tangent) / denomFriction;
            float mu = std::min(rigidBodyA.friction, rigidBodyB.friction);
            float maxFriction = impulseStrength * mu;
            impulseFriction = glm::clamp(impulseFriction, -maxFriction, maxFriction);
            frictionImpulse = impulseFriction * tangent;

            if (rigidBodyA.bodyType != PhysicsType::STATIC) {
                rigidBodyA.linearVelocity -= frictionImpulse * rigidBodyA.inverseMass;
                //Console::getInstance().addLog("Friction Impulse: (" + std::to_string(frictionImpulse.x) + ", " + std::to_string(frictionImpulse.y) + ", " + std::to_string(frictionImpulse.z) + ")");
                vec3 deltaAngularVelocity = IA_raCrossT * impulseFriction;
                rigidBodyA.angularVelocity -= deltaAngularVelocity;
                //Console::getInstance().addLog("Delta Angular Velocity: (" + std::to_string(deltaAngularVelocity.x) + ", " + std::to_string(deltaAngularVelocity.y) + ", " + std::to_string(deltaAngularVelocity.z) + ")");

            }
            if (rigidBodyB.bodyType != PhysicsType::STATIC) {
                rigidBodyB.linearVelocity += frictionImpulse * rigidBodyB.inverseMass;
                rigidBodyB.angularVelocity += IB_rbCrossT * impulseFriction;
            }
        }
        /* Console::getInstance().addLog("Collision detected between Entity " + std::to_string(collision.entityA) + " and Entity " + std::to_string(collision.entityB));
        Console::getInstance().addLog("Collision Point A: (" + std::to_string(collision.collisionPointA.x) + ", " + std::to_string(collision.collisionPointA.y) + ", " + std::to_string(collision.collisionPointA.z) + ")");
        Console::getInstance().addLog("Collision Point B: (" + std::to_string(collision.collisionPointB.x) + ", " + std::to_string(collision.collisionPointB.y) + ", " + std::to_string(collision.collisionPointB.z) + ")");
        Console::getInstance().addLog("Collision Normal: (" + std::to_string(collision.normal.x) + ", " + std::to_string(collision.normal.y) + ", " + std::to_string(collision.normal.z) + ")");
        Console::getInstance().addLog("Penetration Depth: " + std::to_string(collision.penetrationDepth));
        Console::getInstance().addLog("Impulse: (" + std::to_string(impulse.x) + ", " + std::to_string(impulse.y) + ", " + std::to_string(impulse.z) + ")");
        Console::getInstance().addLog("Friction Impulse: (" + std::to_string(frictionImpulse.x) + ", " + std::to_string(frictionImpulse.y) + ", " + std::to_string(frictionImpulse.z) + ")");
     */

    }
}

void PhysicsSystem::collisionResolutionLinear(Registry& registry) {
    for (CollisionInfo& collision : collisionList) {
        RigidBodyComponent& rigidBodyA = registry.get<RigidBodyComponent>(collision.entityA);
        RigidBodyComponent& rigidBodyB = registry.get<RigidBodyComponent>(collision.entityB);

        Transform& transformA = registry.get<Transform>(collision.entityA);
        Transform& transformB = registry.get<Transform>(collision.entityB);

        vec3 correction = collision.normal * collision.penetrationDepth * 0.5f;

        if (rigidBodyA.bodyType != PhysicsType::STATIC && rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction);
            transformB.addPos(-correction);
        } else if (rigidBodyA.bodyType != PhysicsType::STATIC) {
            transformA.addPos(correction * 2.0f);
        } else if (rigidBodyB.bodyType != PhysicsType::STATIC) {
            transformB.addPos(-correction * 2.0f);
        }

        vec3 vA = rigidBodyA.linearVelocity;
        vec3 vB = rigidBodyB.linearVelocity;
        vec3 relativeVelocity = vB - vA;
        float velocityAlongNormal = dot(relativeVelocity, collision.normal);

        if (velocityAlongNormal < 0)
            continue;

        float restitution = std::min(rigidBodyA.restitution, rigidBodyB.restitution);
        float denom = rigidBodyA.inverseMass + rigidBodyB.inverseMass;

        if (denom == 0.0f)
            continue;

        float impulseStrength = -(1.0f + restitution) * velocityAlongNormal / denom;
        vec3 impulse = impulseStrength * collision.normal;

        if (rigidBodyA.bodyType != PhysicsType::STATIC) {
            rigidBodyA.linearVelocity -= impulse * rigidBodyA.inverseMass;
        }
        if (rigidBodyB.bodyType != PhysicsType::STATIC) {
            rigidBodyB.linearVelocity += impulse * rigidBodyB.inverseMass;
        }
    }
}




void PhysicsSystem::update(Registry& registry, float deltaTime) {

    recomputeAABB(registry);

    this->deltaTime = deltaTime;

    integrate(registry, deltaTime);

    broadCollisionDetection(registry);

    collisionResolutionLinear(registry);

}
