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

    std::vector<vec3> collisionPoints;
    std::vector<vec3> collisionPointsA;
    std::vector<vec3> collisionPointsB;
    vec3 collisionPointA;
    vec3 collisionPointB;
    vec3 normal;
    float penetrationDepth;


    float restitution;

};

enum class ColliderType
{
    SPHERE,
    CYLINDER,
    AABB,
    OBB,
    PLANE,
    CONVEX
};


struct Collider {
    ColliderType type;
    float mass = 100.0f;
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

    void computeInertiaTensor() {
        float I = (2.0f / 5.0f) * mass * radius * radius;
        localInertiaTensor = mat3(I, 0.0f, 0.0f,
                                  0.0f, I, 0.0f,
                                  0.0f, 0.0f, I);
    }
};

struct CylinderCollider : public Collider {
    float radius;
    float halfSize;
    vec3 axis;

    CylinderCollider(float radius, float halfSize, vec3 direction = vec3(0.0,1.0,0.0)) : radius(radius), halfSize(halfSize), axis(direction) {
        type = ColliderType::CYLINDER;
        computeInertiaTensor();
    }

    void computeInertiaTensor() {
        float Ix = (1.0f / 12.0f) * mass * (3 *radius*radius + 4 *halfSize * halfSize);
        float Iy = 0.5f * mass * radius * radius;
        float Iz = Ix;
        localInertiaTensor = mat3(Ix, 0.0f, 0.0f,
                                  0.0f, Iy, 0.0f,
                                  0.0f, 0.0f, Iz);
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
    
    OBBCollider(const vec3& halfSize = vec3(1.0f)) : halfSize(halfSize){
        type = ColliderType::OBB;
        rotation = vec3(0.0f);
        computeInertiaTensor();
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

    void computeInertiaTensor() {
        float w = 2.0f * halfSize.x;
        float h = 2.0f * halfSize.y;
        float d = 2.0f * halfSize.z;

        float Ix = (1.0f/12.0f) * mass * (h * h + d * d);
        float Iy = (1.0f/12.0f)* mass * (w * w + d * d);
        float Iz = (1.0f/12.0f) * mass * (w * w +h * h);

        localInertiaTensor = mat3(Ix, 0.0f, 0.0f,
                                  0.0f, Iy, 0.0f,
                                  0.0f, 0.0f, Iz);
    }

};

struct Ray{
    vec3 origin;
    vec3 direction;
    float length;

    Ray(const vec3& origin, const vec3& direction, float length) : origin(origin), direction(direction), length(length) {}
};

struct Face {
    std::vector<glm::vec3> vertices;
    std::vector<int> indices;
    glm::vec3 normal;
};

//utilisé pour le test de collision uniquement
//nos obb sont definit en local, on veut en global
struct WorldOBB {
    vec3 globalCentroid;
    vec3 halfSize;
    vec3 axes[3]; // la nouvelle base

    void getVertices(vec3 vertices[8]) const {
        vertices[0] = globalCentroid + axes[0] * halfSize.x + axes[1] * halfSize.y + axes[2] * halfSize.z;
        vertices[1] = globalCentroid + axes[0] * halfSize.x + axes[1] * halfSize.y - axes[2] * halfSize.z;
        vertices[2] = globalCentroid + axes[0] * halfSize.x - axes[1] * halfSize.y + axes[2] * halfSize.z;
        vertices[3] = globalCentroid + axes[0] * halfSize.x - axes[1] * halfSize.y - axes[2] * halfSize.z;
        vertices[4] = globalCentroid - axes[0] * halfSize.x + axes[1] * halfSize.y + axes[2] * halfSize.z;
        vertices[5] = globalCentroid - axes[0] * halfSize.x + axes[1] * halfSize.y - axes[2] * halfSize.z;
        vertices[6] = globalCentroid - axes[0] * halfSize.x - axes[1] * halfSize.y + axes[2] * halfSize.z;
        vertices[7] = globalCentroid - axes[0] * halfSize.x - axes[1] * halfSize.y - axes[2] * halfSize.z;
    }

    void getFaces(vec3 vertices[8],std::vector<Face>& facesOut) const {
        static const int faceDef[6][4] = {
            { 0, 1, 5, 4 },   // +Y
            { 2, 3, 7, 6 },   // -Y
            { 0, 2, 6, 4 },   // +X
            { 1, 3, 7, 5 },   // -X
            { 0, 1, 3, 2 },   // +Z
            { 4, 5, 7, 6 }    // -Z
        };

        static const vec3 localNormals[6] = {
            vec3( 0,  1,  0),
            vec3( 0, -1,  0),
            vec3( 1,  0,  0),
            vec3(-1,  0,  0),
            vec3( 0,  0,  1),
            vec3( 0,  0, -1)
        };


        facesOut.resize(6);
        for (int i = 0; i < 6; ++i) {
            Face& face = facesOut[i];
            face.indices = {faceDef[i][0], faceDef[i][1], faceDef[i][2], faceDef[i][3]};
            face.vertices.clear();
            for (int j = 0; j < 4; ++j) {
                face.vertices.push_back(vertices[faceDef[i][j]]);
            }

            const vec3& local = localNormals[i];
            face.normal = normalize(
                local.x * axes[0] +
                local.y * axes[1] +
                local.z * axes[2]
            );
        }
    }
};


struct PlaneCollider : public Collider {
    vec3 normal;

    PlaneCollider(const vec3& normal) : normal(normal){
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

    static void getWorldOBB(const OBBCollider& collider, const Transform& transform, WorldOBB& worldOBB);

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
    static void collision_ray_obb(const Entity entityA, const Ray& ray ,const Transform& transformA, const Entity entityB, const OBBCollider& obb, const Transform& transformB, CollisionInfo& collisionInfo);
    

    //dispatch automatiquement vers la bonne fonction
    static void testCollision(
        const Entity entityA, const Collider& colliderA, const Transform& transformA,
        const Entity entityB, const Collider& colliderB, const Transform& transformB,
        CollisionInfo& collisionInfo);
    
};

class ContactPointDetection{

public:

    static void contact_obb_obb(const Entity entityA, const WorldOBB& wobbA, const Transform& transformA, const Entity entityB, const WorldOBB& wobbB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void contact_sphere_sphere(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const SphereCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void contact_sphere_obb(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const OBBCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void contact_obb_plane(const Entity entityA, const OBBCollider& colliderA, const Transform& transformA, const Entity entityB, const PlaneCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
    static void contact_sphere_plane(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const PlaneCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo);
};

#endif