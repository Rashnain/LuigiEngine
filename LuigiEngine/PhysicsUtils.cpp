#include "PhysicsUtils.hpp"
#include "LuigiEngine/Transform.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtx/euler_angles.hpp"

CollisionFn CollisionDetection::collisionDispatchTable[CollisionDetection::NUM_COLLIDER_TYPES][CollisionDetection::NUM_COLLIDER_TYPES] = {
    {collision_sphere_sphere, collision_sphere_cylinder, collision_sphere_aabb, collision_sphere_obb, collision_sphere_plane, nullptr},
    {nullptr, collision_cylinder_cylinder, collision_cylinder_aabb, collision_cylinder_obb, collision_cylinder_plane, nullptr},
    {nullptr, nullptr, collision_aabb_aabb, collision_aabb_obb, collision_aabb_plane, nullptr},
    {nullptr, nullptr, nullptr, collision_obb_obb, collision_obb_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, collision_plane_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
};

//nos obb sont definit en local, on veut en global
struct WorldOBB{
    vec3 globalCentroid;
    vec3 halfSize;
    vec3 axes[3]; // la nouvelle base
};

void getWorldOBB(const OBBCollider& collider, const Transform& transform, WorldOBB& worldOBB){
    mat4 transformModel = transform.getGlobalModel();

    mat4 colliderRotation = yawPitchRoll(collider.rotation.y, collider.rotation.x, collider.rotation.z);

    mat4 newModel = transformModel * colliderRotation;

    worldOBB.globalCentroid = newModel * vec4(collider.localCentroid, 1.0f);

    worldOBB.axes[0] = glm::normalize(glm::vec3(newModel[0]));
    worldOBB.axes[1] = glm::normalize(glm::vec3(newModel[1]));
    worldOBB.axes[2] = glm::normalize(glm::vec3(newModel[2]));
    
    worldOBB.halfSize = collider.halfSize * transform.getScale();
}

void CollisionDetection::collision_obb_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo){

    collisionInfo.isColliding = false;

    mat4 modelA = transformA.getGlobalModel();
    mat4 modelB = transformB.getGlobalModel();

    const OBBCollider& obbA = (const OBBCollider&) colliderA;
    const OBBCollider& obbB = (const OBBCollider&) colliderB;
    
    WorldOBB wobbA;
    WorldOBB wobbB;
    getWorldOBB(obbA, transformA, wobbA);
    getWorldOBB(obbB, transformB, wobbB);

    float minOverlap = std::numeric_limits<float>::max();
    vec3 minAxis;

    //fonction lambda pour tester un axe, algo SAT
    auto testOverlap = [&](const vec3 axis) {
        if(length(axis) < 0.0001f) return true;
        float projectionA = abs(dot(axis, wobbA.axes[0])) * wobbA.halfSize.x + abs(dot(axis, wobbA.axes[1])) * wobbA.halfSize.y+ abs(dot(axis, wobbA.axes[2])) * wobbA.halfSize.z;
        float projectionB = abs(dot(axis, wobbB.axes[0])) * wobbB.halfSize.x + abs(dot(axis, wobbB.axes[1])) * wobbB.halfSize.y + abs(dot(axis, wobbB.axes[2])) * wobbB.halfSize.z;

        float dist = abs(dot( wobbB.globalCentroid - wobbA.globalCentroid , axis ));

        float overlap = projectionA + projectionB - dist;

        if(overlap < 0) return false;

        if(overlap < minOverlap){
            minOverlap = overlap;
            if(dot(wobbB.globalCentroid - wobbA.globalCentroid, axis ) < 0){ //on s'assure que la normal pointent toujours de A vers B
                minAxis = axis;
            }else{
                minAxis = - axis;
            }
        }

        return true;
    };

    //test des 15 axes
    for(int i = 0; i < 3; i++){ if(!testOverlap(wobbA.axes[i])) return; }
    for(int i = 0; i < 3; i++){ if(!testOverlap(wobbB.axes[i])) return; }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){ 
            vec3 newAxis = cross(wobbA.axes[i], wobbB.axes[j]); 
            newAxis = normalize(newAxis);
            if(!testOverlap(newAxis)) return;
        }
    }

    collisionInfo.isColliding = true;
    collisionInfo.normal = minAxis;
    collisionInfo.penetrationDepth = minOverlap;


    //calculer point de contact
    
}

void CollisionDetection::collision_sphere_sphere(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    collisionInfo.isColliding = false;

    const SphereCollider& sphereA = (const SphereCollider&) colliderA;
    const SphereCollider& sphereB = (const SphereCollider&) colliderB;

    vec3 centerA = vec3(transformA.getGlobalModel() * vec4(sphereA.localCentroid, 1.0f));
    vec3 centerB = vec3(transformB.getGlobalModel() * vec4(sphereB.localCentroid, 1.0f));

    float radiusA = sphereA.radius * glm::max(glm::max(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);
    float radiusB = sphereB.radius * glm::max(glm::max(transformB.getScale().x, transformB.getScale().y), transformB.getScale().z);

    vec3 diff = centerB - centerA;
    float distSquared = dot(diff, diff);
    float radiusSum = radiusA + radiusB;

    if (distSquared <= radiusSum * radiusSum) {
        collisionInfo.isColliding = true;
        float dist = sqrt(distSquared);
        collisionInfo.normal = dist > 0.0f ? -diff / dist : vec3(1.0f, 0.0f, 0.0f);
        collisionInfo.penetrationDepth = radiusSum - dist;
        collisionInfo.collisionPointA = centerA + collisionInfo.normal * radiusA;
        collisionInfo.collisionPointB = centerB - collisionInfo.normal * radiusB;
    }
}

void CollisionDetection::collision_sphere_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    collisionInfo.isColliding = false;

    const SphereCollider& sphere = (const SphereCollider&) colliderA;
    const OBBCollider& obb = (const OBBCollider&) colliderB;

    vec3 sphereCenter = transformA.getGlobalModel() * vec4(sphere.localCentroid, 1.0f); 
    float sphereRadius = sphere.radius * glm::max(glm::max(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);

    WorldOBB wobb;
    getWorldOBB(obb, transformB, wobb);

    vec3 dir = sphereCenter - wobb.globalCentroid;

    
    //on cherche le point sur l'obb le plus proche de la sphere
    vec3 closestPoint = wobb.globalCentroid;

    for (int i = 0; i < 3; ++i) {
        float projection = dot(dir, wobb.axes[i]);
        float clamped = glm::clamp(projection, -wobb.halfSize[i], wobb.halfSize[i]);
        closestPoint += clamped * wobb.axes[i];
    }

    vec3 diff = sphereCenter - closestPoint;
    float distSquared = dot(diff, diff);

    if (distSquared <= sphereRadius * sphereRadius) {
        float distance = sqrt(distSquared);

        collisionInfo.isColliding = true;
        collisionInfo.normal = distance > 0.0f ? diff / distance : vec3(1.0f, 0.0f, 0.0f);
        collisionInfo.penetrationDepth = sphereRadius - distance;
        collisionInfo.collisionPointA = sphereCenter - collisionInfo.normal * sphereRadius;
        collisionInfo.collisionPointB = closestPoint;
    }
}

void CollisionDetection::collision_sphere_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    collisionInfo.isColliding = false;

    const SphereCollider& sphere = (const SphereCollider&) colliderA;
    const PlaneCollider& plane = (const PlaneCollider&) colliderB;

    vec3 sphereCenter = transformA.getGlobalModel() * vec4(sphere.localCentroid, 1.0f); 
    float sphereRadius = sphere.radius * transformA.getScale().x;

    vec3 planePosition = transformB.getGlobalModel() * vec4(plane.localCentroid, 1.0f);
    vec3 planeNormal = transformB.getGlobalModel() * vec4(plane.normal, 0.0f);
    planeNormal = normalize(planeNormal);

    float distance = dot(sphereCenter - planePosition, planeNormal);

    if (distance < sphereRadius) {
        collisionInfo.isColliding = true;
        collisionInfo.penetrationDepth = sphereRadius - abs(distance);
        collisionInfo.normal = distance > 0.0f ? planeNormal : -planeNormal;
        collisionInfo.collisionPointA = sphereCenter - collisionInfo.normal * sphereRadius;
        collisionInfo.collisionPointB = sphereCenter - collisionInfo.normal * distance;
    }
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
    collisionInfo.isColliding = false;
    const OBBCollider& obb = (const OBBCollider&) colliderA;
    const PlaneCollider& plane = (const PlaneCollider&) colliderB;

    WorldOBB wobb;
    getWorldOBB(obb, transformA, wobb);

    vec3 planePosition = transformB.getGlobalModel() * vec4(plane.localCentroid, 1.0f);
    vec3 planeNormal = normalize(transformB.getGlobalModel() * vec4(plane.normal, 0.0f));

    // le point sur l'obb le plus proche du plan
    vec3 closestPoint = wobb.globalCentroid;

    for (int i = 0; i < 3; ++i) {
        float projection = dot(-planeNormal, wobb.axes[i]);
        float clamped = glm::clamp(projection, -wobb.halfSize[i], wobb.halfSize[i]);
        closestPoint += clamped * wobb.axes[i];
    }

    float distance = dot(closestPoint - planePosition, planeNormal);

    if (distance < 0) {
        collisionInfo.isColliding = true;
        collisionInfo.penetrationDepth = -distance;
        collisionInfo.normal = planeNormal;
        collisionInfo.collisionPointA = closestPoint;
        collisionInfo.collisionPointB = closestPoint;
    }

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
            //collisionInfo.normal = - collisionInfo.normal; //apparemment pas necessaire ?
            std::swap(collisionInfo.entityA, collisionInfo.entityB);
        }
    } else {
        collisionInfo.isColliding = false;
        std::cout << "Pas de fonction de collision definit !!!" << std::endl;
    }
}                                        