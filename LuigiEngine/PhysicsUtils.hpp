#ifndef PHYSICS_UTILS_HPP
#define PHYSICS_UTILS_HPP

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "ECS.h"
#include "glm/detail/type_vec.hpp"

using namespace glm;

struct CollisionInfo{
    Entity entityA;
    Entity entityB;

    bool isColliding = false;

    vec3 point;
    vec3 collisionPointA;
    vec3 collisionPointB;
    vec3 normal;
    float penetrationDepth;

    vec3 localPointA; //le point de collision en coord local sur A
    vec3 localPointB; //idem pour B

    float restitution;

};

enum class ColliderType
{
    SPHERE,
    CylinderCollider,
    AABB,
    OBB,
    PLANE,
    CONVEX
};


struct Collider {
    ColliderType type;
    float mass = 1.0f;
    mat3 localInertiaTensor;
    vec3 localCentroid = vec3(0.0f);

    vec3 support(const vec3 & direction) const; //donne un point de support pour l'algo GJK implementation plus tard

    virtual ~Collider() = default;
};

struct SphereCollider : public Collider {
    float radius;

    SphereCollider(float r) : radius(r) {
        type = ColliderType::SPHERE;
    }
};

struct CylinderCollider : public Collider {
    float radius;
    float height;
    vec3 axis = vec3(0.0,1.0,0.0);

    CylinderCollider(float r, float h) : radius(r), height(h) {
        type = ColliderType::CylinderCollider;
    }
};

struct AABBCollider : public Collider {
    vec3 min;
    vec3 max;

    AABBCollider() = default;

    AABBCollider(vec3 min, vec3 max) : min(min), max(max) {
        type = ColliderType::AABB;
    }
};

struct OBBCollider : public Collider {
    vec3 halfSize; 
    vec3 rotation;
    
    OBBCollider(const vec3& halfSize) : halfSize(halfSize){
        type = ColliderType::OBB;
        rotation = vec3(0.0f);
    }

    //un peu moche mais c'est le mieux
    void getVertices(const vec3& center, const vec3& right, const vec3& up, const vec3& front, vec3 vertices[8]) const {
        vertices[0] = center + right * halfSize.x + up * halfSize.y + front * halfSize.z;
        vertices[1] = center + right * halfSize.x + up * halfSize.y - front * halfSize.z;
        vertices[2] = center + right * halfSize.x - up * halfSize.y + front * halfSize.z;
        vertices[3] = center + right * halfSize.x - up * halfSize.y - front * halfSize.z;
        vertices[4] = center - right * halfSize.x + up * halfSize.y + front * halfSize.z;
        vertices[5] = center - right * halfSize.x + up * halfSize.y - front * halfSize.z;
        vertices[6] = center - right * halfSize.x - up * halfSize.y + front * halfSize.z;
        vertices[7] = center - right * halfSize.x - up * halfSize.y - front * halfSize.z;
    }

};

struct PlaneCollider : public Collider {
    vec3 normal;
    float d; 

    PlaneCollider( vec3& normal, float d) : normal(normal), d(d) {
        type = ColliderType::PLANE;
    }
};


struct ConvexCollider : public Collider {
    std::vector<vec3> points;
    std::vector<ivec2> edges; //aretes par indices
    std::vector<ivec3> faces; //faces par indices

    ConvexCollider(const std::vector<vec3>& points, const std::vector<ivec2>& edges, const std::vector<ivec3>& faces): points(points), edges(edges), faces(faces) {
        type = ColliderType::CONVEX;
    }
};

//Pour le double dispatch
struct Collider;
struct CollisionInfo;
struct Transform;
using CollisionFn = void(*)(const Entity, const Collider&, const Transform&, const Entity,const Collider&, const Transform&, CollisionInfo&);

class CollisionDetection {

    //attribue automatiquement la bonne fonction selon les types de colliders
    static constexpr int NUM_COLLIDER_TYPES = 6;
    static CollisionFn collisionDispatchTable[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES];

public:

    static void collision_sphere_sphere(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB ,const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_sphere_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_sphere_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_sphere_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_sphere_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);

    static void collision_cylinder_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_cylinder_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_cylinder_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_cylinder_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);

    static void collision_aabb_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_aabb_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_aabb_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);

    static void collision_obb_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void collision_obb_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);

    static void collision_plane_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);


    //dispatch automatiquement vers la bonne fonction
    static void testCollision(
        const Entity entityA, const Collider& colliderA, const Transform& transformA,
        const Entity entityB, const Collider& colliderB, const Transform& transformB,
        CollisionInfo& collisionInfo);
    
};

#endif