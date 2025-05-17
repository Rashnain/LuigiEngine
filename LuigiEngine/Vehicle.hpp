#ifndef VEHICLE_HPP
#define VEHICLE_HPP
#include "Physics.hpp"
#include "Constraint.hpp"


struct VehicleComponent {
    Entity chassis = INVALID; // le chassis

    Entity FLWheel, FRWheel, BLWheel, BRWheel; // les roues

    SuspensionConstraint *FLSuspension = nullptr, *FRSuspension = nullptr, *BLSuspension = nullptr, *BRSuspension = nullptr; // les suspensions

    OBBCollider* chassisCollider; // pointeur unique pour tester

    void onAttach(Registry& registry, Entity entity) {
        chassis = entity;
    }

    void onDetach(Registry& registry, Entity entity) {
        chassis = INVALID;
        FLWheel = FRWheel = BLWheel = BRWheel = INVALID;
        FLSuspension = FRSuspension = BLSuspension = BRSuspension = nullptr;
        if (chassisCollider) {
            delete chassisCollider;
            chassisCollider = nullptr;
        }
    }


    void setup(Registry& registry, ConstraintSystem& constraintSystem, MeshComponent& chassisMesh, MeshComponent& wheelMesh, vec3 position, vec3 scale = vec3(1.0f), float wheelRadius = 0.5f, float wheelWidth = 0.5f) {
        // chassis
        Transform& transform = registry.emplace<Transform>(chassis);
        transform.setPos(position);
        transform.setScale(scale);
        registry.emplace<MeshComponent>(chassis, chassisMesh);
        registry.emplace<Hierarchy>(chassis, std::vector<Entity>{}).name = "Chassis";


        // collision
        vec3 colliderSize = vec3(0.25f, 0.18f, 0.5f);
        chassisCollider = new OBBCollider(colliderSize);
        chassisCollider->localCentroid = vec3(0.0f, 0.28f, 0.0f);
        chassisCollider->mass = 100.0f;

        auto& rigidBody = registry.emplace<RigidBodyComponent>(chassis);
        rigidBody.addCollider(chassisCollider);

        // les roues
        auto createWheel = [&](Entity& wheelEntity, const std::string& name, vec3 offset) {
            wheelEntity = registry.create();
            Transform& wheelTransform = registry.emplace<Transform>(wheelEntity);
            wheelTransform.setPos(position + offset);
            wheelTransform.setScale(vec3(wheelWidth, wheelRadius, wheelRadius));
            registry.emplace<MeshComponent>(wheelEntity, wheelMesh);
            //registry.emplace<Hierarchy>(wheelEntity, std::vector<Entity>{}).name = name;
        };

        float stiffness = 2000.0f;
        float damping = 100.0f;
        float initialLength = 1.2;

        vec3 hs = chassisCollider->halfSize * scale * colliderSize;
        hs.z /= 2.75f;
        hs.y *= 2.0f;

        createWheel(FLWheel, "FLWheel", vec3(-hs.x, chassisCollider->halfSize.y * scale.y,  hs.z));
        createWheel(FRWheel, "FRWheel", vec3( hs.x, chassisCollider->halfSize.y * scale.y,  hs.z));
        createWheel(BLWheel, "BLWheel", vec3(-hs.x, chassisCollider->halfSize.y * scale.y, -hs.z));
        createWheel(BRWheel, "BRWheel", vec3( hs.x, chassisCollider->halfSize.y * scale.y, -hs.z));

        FLSuspension = constraintSystem.addSuspension(chassis, vec3(-hs.x, hs.y,  -hs.z), vec3(0.0f, -1.0f, 0.0f), initialLength);
        FLSuspension->stiffness = stiffness;
        FLSuspension->damping = damping;

        FRSuspension = constraintSystem.addSuspension(chassis, vec3( hs.x, hs.y,  -hs.z), vec3(0.0f, -1.0f, 0.0f), initialLength);
        FRSuspension->stiffness = stiffness;
        FRSuspension->damping = damping;

        BLSuspension = constraintSystem.addSuspension(chassis, vec3(-hs.x, hs.y, hs.z), vec3(0.0f, -1.0f, 0.0f), initialLength);
        BLSuspension->stiffness = stiffness;
        BLSuspension->damping = damping;

        BRSuspension = constraintSystem.addSuspension(chassis, vec3( hs.x, hs.y, hs.z), vec3(0.0f, -1.0f, 0.0f), initialLength);
        BRSuspension->stiffness = stiffness;
        BRSuspension->damping = damping;
    }

    void setStiffness(float stiffness) {
        FLSuspension->stiffness = stiffness;
        FRSuspension->stiffness = stiffness;
        BLSuspension->stiffness = stiffness;
        BRSuspension->stiffness = stiffness;
    }

    void setDamping(float damping) {
        FLSuspension->damping = damping;
        FRSuspension->damping = damping;
        BLSuspension->damping = damping;
        BRSuspension->damping = damping;
    }

    void setInitialLength(float length) {
        FLSuspension->initialLength = length;
        FRSuspension->initialLength = length;
        BLSuspension->initialLength = length;
        BRSuspension->initialLength = length;
    }

    void setMaxForce(float force) {
        FLSuspension->maxForce = force;
        FRSuspension->maxForce = force;
        BLSuspension->maxForce = force;
        BRSuspension->maxForce = force;
    }
};


struct VehicleSystem{

    void update(Registry& registry, float deltaTime);



};

#endif