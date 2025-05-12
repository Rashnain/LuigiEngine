#include "PhysicsUtils.hpp"
#include "LuigiEngine/Transform.hpp"
#include "glm/detail/type_vec.hpp"

CollisionFn CollisionDetection::collisionDispatchTable[CollisionDetection::NUM_COLLIDER_TYPES][CollisionDetection::NUM_COLLIDER_TYPES] = {
    {collision_sphere_sphere, collision_sphere_cylinder, collision_sphere_aabb, collision_sphere_obb, collision_sphere_plane, nullptr},
    {nullptr, collision_cylinder_cylinder, collision_cylinder_aabb, collision_cylinder_obb, collision_cylinder_plane, nullptr},
    {nullptr, nullptr, collision_aabb_aabb, collision_aabb_obb, collision_aabb_plane, nullptr},
    {nullptr, nullptr, nullptr, collision_obb_obb, collision_obb_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, collision_plane_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
};

void CollisionDetection::collision_obb_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo){

    const OBBCollider& obbA = (const OBBCollider&) colliderA;
    const OBBCollider& obbB = (const OBBCollider&) colliderB;
    vec3 halfSizeA = obbA.halfSize;
    vec3 halfSizeB = obbB.halfSize;
    vec3 centerA = transformA.getPos();
    vec3 centerB = transformB.getPos();

    vec3 delta = centerB - centerA;

    vec3 rightA = transformA.getRight();
    vec3 upA = transformA.getUp();
    vec3 frontA = transformA.getFront();
    vec3 rightB = transformB.getRight();
    vec3 upB = transformB.getUp();
    vec3 frontB = transformB.getFront();
    vec3 axes[15] = {rightA, upA, frontA, rightB, upB, frontB,
                     cross(rightA, rightB), cross(rightA, upB), cross(rightA, frontB),
                     cross(upA, rightB), cross(upA, upB), cross(upA, frontB),
                     cross(frontA, rightB), cross(frontA, upB), cross(frontA, frontB)};

    float minOverlap = std::numeric_limits<float>::max();

    vec3 obbAVertices[8];
    vec3 obbBVertices[8];
    obbA.getVertices(centerA, rightA, upA, frontA, obbAVertices);
    obbB.getVertices(centerB, rightB, upB, frontB, obbBVertices);

    for(int i = 0; i < 15; i++){
        vec3 axis = axes[i];
        axis = normalize(axis);

        float minA = std::numeric_limits<float>::max(), maxA = -std::numeric_limits<float>::max();
        float minB = std::numeric_limits<float>::max(), maxB = -std::numeric_limits<float>::max();
        
        //on projette A
        for(int j = 0; j < 8; j++){
            float dotA = dot(obbAVertices[j], axis);
            minA = std::min(minA, dotA);
            maxA = std::max(maxA, dotA);
        }

        //on projette B
        for(int j = 0; j < 8; j++){
            float dotB = dot(obbBVertices[j], axis);
            minB = std::min(minB, dotB);
            maxB = std::max(maxB, dotB);
        }

        if(maxA < minB || maxB < minA) {
            //pas d'overlap donc pas de collision
            collisionInfo.isColliding = false;
            return;
        }

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
    
        if(overlap < minOverlap){
            minOverlap = overlap;
            collisionInfo.normal = axis;
        }
    }
    
    collisionInfo.penetrationDepth = minOverlap;

    collisionInfo.collisionPointA = centerA + collisionInfo.normal * halfSizeA.x;
    collisionInfo.collisionPointB = centerB - collisionInfo.normal * halfSizeB.x;

    collisionInfo.isColliding = true;
    collisionInfo.entityA = entityA;
    collisionInfo.entityB = entityB;
    
    //normal de A vers B
    if (dot(collisionInfo.normal, delta) < 0)
        collisionInfo.normal = -collisionInfo.normal;


    std::cout << "Collision Info:" << std::endl;
    std::cout << "Is Colliding: " << (collisionInfo.isColliding ? "Yes" : "No") << std::endl;
    if (collisionInfo.isColliding) {
        std::cout << "Penetration Depth: " << collisionInfo.penetrationDepth << std::endl;
    }
    std::cout << "Collision Normal: (" << collisionInfo.normal.x << ", " << collisionInfo.normal.y << ", " << collisionInfo.normal.z << ")" << std::endl;

    std::cout << "Collision Point A: (" << collisionInfo.collisionPointA.x << ", " << collisionInfo.collisionPointA.y << ", " << collisionInfo.collisionPointA.z << ")" << std::endl;
    std::cout << "Collision Point B: (" << collisionInfo.collisionPointB.x << ", " << collisionInfo.collisionPointB.y << ", " << collisionInfo.collisionPointB.z << ")" << std::endl;
    std::cout << "Entity A: " << collisionInfo.entityA << std::endl;
    std::cout << "Entity B: " << collisionInfo.entityB << std::endl;
}

void CollisionDetection::collision_sphere_sphere(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_cylinder_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_cylinder_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_cylinder_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_cylinder_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_aabb_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_aabb_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_aabb_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_obb_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_plane_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}
   

void CollisionDetection::testCollision(const Entity entityA, const Collider &colliderA, const Transform &transformA, const Entity entityB, const Collider &colliderB, const Transform &transformB, CollisionInfo &collisionInfo){
    int typeA = static_cast<int>(colliderA.type);
    int typeB = static_cast<int>(colliderB.type);

    bool swap = false;
    const Collider* cA = &colliderA;
    const Collider* cB = &colliderB;
    const Transform* tA = &transformA;
    const Transform* tB = &transformB;
    Entity eA = entityA;
    Entity eB = entityB;
    
    //on trie selon le type de collider
    if (typeB < typeA) {
        std::swap(typeA, typeB);
        std::swap(cA, cB);
        std::swap(tA, tB);
        std::swap(eA, eB);
        swap = true;
    }

    CollisionFn fn = CollisionDetection::collisionDispatchTable[typeA][typeB];
    if (fn) {
        fn(eA, *cA, *tA, eB, *cB, *tB, collisionInfo);
        
        //on swap le resultat pour toujours garder la collision de A vers B
        if (swap) {
            std::swap(collisionInfo.collisionPointA, collisionInfo.collisionPointB);
            std::swap(collisionInfo.localPointA, collisionInfo.localPointB);
            collisionInfo.normal = -collisionInfo.normal;
            std::swap(collisionInfo.entityA, collisionInfo.entityB);
        }
    } else {
        collisionInfo.isColliding = false;
        std::cout << "Pas de fonction de collision definit !!!" << std::endl;
    }
}                                        