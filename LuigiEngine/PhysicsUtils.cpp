#include "PhysicsUtils.hpp"
#include "LuigiEngine/Transform.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "LuigiEngine/ImGuiConsole.hpp"

CollisionFn CollisionDetection::collisionDispatchTable[CollisionDetection::NUM_COLLIDER_TYPES][CollisionDetection::NUM_COLLIDER_TYPES] = {
    {collision_sphere_sphere, collision_sphere_cylinder, collision_sphere_aabb, collision_sphere_obb, collision_sphere_plane, nullptr},
    {nullptr, collision_cylinder_cylinder, collision_cylinder_aabb, collision_cylinder_obb, collision_cylinder_plane, nullptr},
    {nullptr, nullptr, collision_aabb_aabb, collision_aabb_obb, collision_aabb_plane, nullptr},
    {nullptr, nullptr, nullptr, collision_obb_obb, collision_obb_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, collision_plane_plane, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
};


void CollisionDetection::getWorldOBB(const OBBCollider& collider, const Transform& transform, WorldOBB& worldOBB){
    mat4 transformModel = transform.getGlobalModel();

    mat4 colliderRotation = yawPitchRoll(collider.rotation.y, collider.rotation.x, collider.rotation.z);

    mat4 newModel = transformModel * colliderRotation;

    worldOBB.globalCentroid = newModel * vec4(collider.localCentroid, 1.0f);

    worldOBB.axes[0] = normalize(vec3(newModel[0]));
    worldOBB.axes[1] = normalize(vec3(newModel[1]));
    worldOBB.axes[2] = normalize(vec3(newModel[2]));
    
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

    float minOverlap = numeric_limits<float>::max();
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

    //ContactPointDetection::contact_obb_obb(entityA, wobbA, transformA, entityB, wobbB, transformB, collisionInfo);

    
}

void CollisionDetection::collision_sphere_sphere(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    collisionInfo.isColliding = false;

    //std::cout << entityA << " and " << entityB << std::endl;

    const SphereCollider& sphereA = (const SphereCollider&) colliderA;
    const SphereCollider& sphereB = (const SphereCollider&) colliderB;

    vec3 centerA = vec3(transformA.getGlobalModel() * vec4(sphereA.localCentroid, 1.0f));
    vec3 centerB = vec3(transformB.getGlobalModel() * vec4(sphereB.localCentroid, 1.0f));

    float radiusA = sphereA.radius * std::max(std::max(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);
    float radiusB = sphereB.radius * std::max(std::max(transformB.getScale().x, transformB.getScale().y), transformB.getScale().z);

    vec3 diff = centerB - centerA;
    float distSquared = dot(diff, diff);
    float radiusSum = radiusA + radiusB;

    if (distSquared <= radiusSum * radiusSum) {
        collisionInfo.isColliding = true;
        float dist = sqrt(distSquared);
        collisionInfo.normal = dist > 0.0f ? -diff / dist : vec3(1.0f, 0.0f, 0.0f);
        collisionInfo.penetrationDepth = radiusSum - dist;
        collisionInfo.collisionPointA = centerA + diff / dist * radiusA;
        collisionInfo.collisionPointB = centerB - diff / dist * radiusB;
    }
}

void CollisionDetection::collision_sphere_cylinder(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    const SphereCollider& sphere = (const SphereCollider&)colliderA;
    const CylinderCollider& cylinder = (const CylinderCollider&)colliderB;

    vec3 sphereCenter = transformA.getGlobalModel() * vec4(sphere.localCentroid, 1.0f); 
    float sphereRadius = sphere.radius * std::max(std::max(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);
 
    vec3 cylinderPos = transformB.getGlobalModel() * vec4(cylinder.localCentroid, 1.0f);
    vec3 cylinderAxis = normalize(transformB.getGlobalModel() * vec4(cylinder.axis, 0.0f));
    float cylinderRadius = cylinder.radius * transformB.getScale().x;
    float cylinderHeight = cylinder.halfSize * std::min(std::min(transformB.getScale().x, transformB.getScale().y), transformB.getScale().z);

    vec3 axis = sphereCenter - cylinderPos;
    float t = dot(axis, cylinderAxis);

    vec3 radial = axis - t * cylinderAxis;
    float radialDist = length(radial);

    float axialOff = abs(t) - cylinderHeight;
    float radialOff = radialDist - cylinderRadius;

    float dist;
    vec3 normal;

    //cote du cylindre
    if (axialOff <= 0.0f && radialOff <= 0.0f) {
        dist = fabs(radialOff);
        normal = (radialDist > 1e-6f ? radial / radialDist : vec3(1.0f, 0.0f, 0.0f));
    } else if (axialOff > 0.0f && radialOff <= 0.0f) {//face plane du cylindre
        dist = axialOff;
        normal = (t > 0.0f ? cylinderAxis : -cylinderAxis);
    } else {//sur un coin
        dist = sqrt(axialOff*axialOff + radialOff*radialOff);
        vec3 dirRadial = (radialDist > 1e-6f ? radial / radialDist : vec3(1.0f, 0.0f, 0.0f));
        vec3 rimDir = normalize(dirRadial * cylinderRadius + (t > 0.0f ? cylinderAxis : -cylinderAxis) * cylinderHeight);
        normal = rimDir;
    }


    float penetration = sphereRadius - dist;
    if (penetration > 0.0f) {
        collisionInfo.isColliding = true;
        collisionInfo.normal = normal;
        collisionInfo.penetrationDepth = penetration;
        collisionInfo.collisionPointA = sphereCenter - normal * sphereRadius;

        if (axialOff <= 0.0f) {
            vec3 sidePoint = cylinderPos + t * cylinderAxis + normal * cylinderRadius;
            collisionInfo.collisionPointB = sidePoint;
        } else {
            vec3 capCenter = cylinderPos + (t > 0.0f ? cylinderAxis : -cylinderAxis) * cylinderHeight;
            collisionInfo.collisionPointB = capCenter + normal * cylinderRadius;
        }
    }

    
}

void CollisionDetection::collision_sphere_aabb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_sphere_obb(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    collisionInfo.isColliding = false;

    const SphereCollider& sphere = (const SphereCollider&) colliderA;
    const OBBCollider& obb = (const OBBCollider&) colliderB;

    vec3 sphereCenter = transformA.getGlobalModel() * vec4(sphere.localCentroid, 1.0f); 
    float sphereRadius = sphere.radius * std::max(std::max(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);

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
    collisionInfo.isColliding = false;

    const CylinderCollider& cylinder = (const CylinderCollider&) colliderA;
    const OBBCollider& obb = (const OBBCollider&) colliderB;

    vec3 cylinderPos = transformA.getGlobalModel() * vec4(cylinder.localCentroid, 1.0f);
    vec3 cylinderAxis = normalize(transformA.getGlobalModel() * vec4(cylinder.axis, 0.0f));
    float cylinderRadius = cylinder.radius * transformA.getScale().x;
    float cylinderHeight = cylinder.halfSize * std::min(std::min(transformA.getScale().x, transformA.getScale().y), transformA.getScale().z);

    WorldOBB wobb;
    getWorldOBB(obb, transformB, wobb);

    vec3 start = cylinderPos + cylinderAxis * cylinderHeight;
    vec3 end = cylinderPos - cylinderAxis * cylinderHeight;

    float minOverlap = numeric_limits<float>::max();
    vec3 minAxis;

    auto testOverlap = [&](const vec3& axis){ //penser a normaliser l'axe
        //float projCylinder = abs(dot(axis, cylinderAxis)) * cylinderHeight + cylinderRadius;
        float projOBB = abs(dot(axis, wobb.axes[0])) * wobb.halfSize.x + abs(dot(axis, wobb.axes[1])) * wobb.halfSize.y + abs(dot(axis, wobb.axes[2])) * wobb.halfSize.z;

        //besoin de faire ca sinon le cylindre agit comme une capsule

        float d = dot(axis,cylinderAxis);
        
        float projHeight = cylinderHeight * abs(d);
        float projRadius = cylinderRadius * sqrt(std::max(0.0f,1.0f - abs(d)*abs(d))); //chatgpt
        
        float projCylinder = projHeight + projRadius;

        float dist = abs(dot(wobb.globalCentroid - cylinderPos, axis));

        float overlap = projCylinder + projOBB - dist;

        if (overlap < 0) return false;

        if (overlap < minOverlap) {
            minOverlap = overlap;

            if (dot(wobb.globalCentroid - cylinderPos,axis) < 0) {
                minAxis = axis;
            } else {
                minAxis = -axis;
            }
        }

        return true;
    };

        if (!testOverlap(cylinderAxis)) {return;} //axe du cylindre   
        for (int i = 0; i < 3; ++i) {if (!testOverlap(wobb.axes[i])) return;} //les axes de l'obb

        //cross product de tous les axes precedents
        for (int i = 0; i < 3; ++i) {
            vec3 newAxis = normalize(cross(cylinderAxis, wobb.axes[i]));
            if ( !testOverlap(newAxis)) return;
        }

        collisionInfo.isColliding = true;
        collisionInfo.normal = minAxis;
        collisionInfo.penetrationDepth = minOverlap;


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

    float distance = dot(wobb.globalCentroid - planePosition, planeNormal);

    float projection = 0.0f;
    for (int i = 0; i < 3; ++i) {
        projection += abs(dot(wobb.axes[i], planeNormal)) * wobb.halfSize[i];
    }


    if (distance <= projection) {
        collisionInfo.isColliding = true;
        collisionInfo.penetrationDepth = projection - abs(distance);
        collisionInfo.normal = planeNormal;

        vec3 closestPoint = wobb.globalCentroid - distance * planeNormal;
        collisionInfo.collisionPointA = closestPoint;
        collisionInfo.collisionPointB = closestPoint;
    }

}

void CollisionDetection::collision_plane_plane(const Entity entityA, const Collider& colliderA, const Transform& transformA, const Entity entityB, const Collider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void CollisionDetection::collision_ray_obb(const Entity entityA, const Ray& ray ,const Transform& transformA, const Entity entityB, const OBBCollider& obb, const Transform& transformB, CollisionInfo& collisionInfo){

    collisionInfo.isColliding = false;

    WorldOBB wobb;
    getWorldOBB(obb, transformB, wobb);

    mat4 obbModel = mat4(1.0f);
    obbModel[0] = vec4(wobb.axes[0], 0.0f);
    obbModel[1] = vec4(wobb.axes[1], 0.0f);
    obbModel[2] = vec4(wobb.axes[2], 0.0f);
    obbModel[3] = vec4(wobb.globalCentroid, 1.0f);

    mat4 invObbModel = inverse(obbModel);

    vec3 localOrigin = vec3(invObbModel * vec4(ray.origin, 1.0f));
    vec3 localDir = normalize(vec3(invObbModel * vec4(ray.direction, 0.0f)));

    vec3 minB = -wobb.halfSize;
    vec3 maxB = wobb.halfSize;

    float tmin = numeric_limits<float>::lowest();
    float tmax = numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        if (abs(localDir[i]) < 1e-8f) {//ray parallele
            if (localOrigin[i] < minB[i] || localOrigin[i] > maxB[i])
                return;
        } else {
            float ood = 1.0f / localDir[i];
            float t1 = (minB[i] - localOrigin[i]) * ood;
            float t2 = (maxB[i] - localOrigin[i]) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax)
                return;
        }
    }

    if (tmax < 0.0f) return;

    float tHit = tmin >= 0.0f ? tmin : tmax;
    vec3 localHit = localOrigin + localDir * tHit;
    vec3 globalHit = vec3(obbModel * vec4(localHit, 1.0f));

    if (tHit < 0.0f || tHit > ray.length) return; 

    collisionInfo.isColliding = true;
    collisionInfo.penetrationDepth = tHit;
    collisionInfo.collisionPointA = ray.origin + ray.direction * tHit;
    collisionInfo.collisionPointB = globalHit;

    vec3 normalLocal(0.0f);
    for (int i = 0; i < 3; ++i) {
        if (abs(localHit[i] - minB[i]) < 0.00001f)
            normalLocal[i] = -1.0f;
        else if (abs(localHit[i] - maxB[i]) < 0.00001f)
            normalLocal[i] = 1.0f;
    }

    collisionInfo.normal = normalize(vec3(obbModel * vec4(normalLocal, 0.0f)));

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
            //std::swap(collisionInfo.localPointA, collisionInfo.localPointB);
            //collisionInfo.normal = - collisionInfo.normal; //apparemment pas necessaire ?
            std::swap(collisionInfo.entityA, collisionInfo.entityB);
        }
    } else {
        collisionInfo.isColliding = false;
        std::cout << "Pas de fonction de collision definit !!!" << std::endl;
    }
}                                        

vector<vec3> clipToPlane(const vector<vec3>& poly, const vec3& planePoint, const vec3& planeNormal) {
    vector<vec3> output;
    int n = poly.size();
    for (int i = 0; i < n; ++i) {
        vec3 A = poly[i];
        vec3 B = poly[(i + 1) % n];
        float dA = glm::dot(A - planePoint, planeNormal);
        float dB = glm::dot(B - planePoint, planeNormal);
        if (dA >= 0) output.push_back(A);
        if ((dA >= 0) ^ (dB >= 0)) {
            float t = dA / (dA - dB);
            output.push_back(A + t * (B - A));
        }
    }
    return output;
}

vector<vec3> clipToFace(const vector<vec3>& incidentVertices, const WorldOBB& referenceBox, int faceIndex){

    vec3 N = referenceBox.axes[faceIndex];
    vec3 R = referenceBox.axes[(faceIndex + 1) % 3];
    vec3 U = referenceBox.axes[(faceIndex + 2) % 3];
    vec3 C = referenceBox.globalCentroid + N * referenceBox.halfSize[faceIndex];
    float rExtent = referenceBox.halfSize[(faceIndex + 1) % 3];
    float uExtent = referenceBox.halfSize[(faceIndex + 2) % 3];

    vector<vec3> poly = clipToPlane(incidentVertices, C +  R * rExtent, -R);
    poly = clipToPlane(poly, C -  R * rExtent,  R);
    poly = clipToPlane(poly, C +  U * uExtent, -U);
    poly = clipToPlane(poly, C -  U * uExtent,  U);
    return poly;

}



//https://github.com/DallinClark/3d-physics-engine/blob/main/src/physics/collisions/collisions.cpp
//renvoie l'indice du point le plus eloigne dans la direction donne
int supportOBB(vec3 vertices[8], const vec3& direction) {
    float maxDot = numeric_limits<float>::lowest();
    int maxIndex = -1;
    for (int i = 0; i < 8; ++i) {
        float d = dot(vertices[i], direction);
        if (d > maxDot) {
            maxDot = d;
            maxIndex = i;
        }
    }
    return maxIndex;
}

//on cherche la face qui contient le vertex minIndex et qui a la normal la plus aligne avec axis
Face bestFace(vec3 vertices[8], vector<Face>& faces, int minIndex, vec3 axis){

    Face best;
    float bestAlignement = numeric_limits<float>::lowest();

    for(Face &f : faces) {

        bool containsTarget = false;
        vector<int> faceIndices;

        for (auto faceIndice : f.indices) {
            if (faceIndice == minIndex){ containsTarget = true;}
        }
        if(!containsTarget){continue;}

        float alignement = dot(f.normal, axis);
        if(alignement > bestAlignement){
            bestAlignement = alignement;
            best = f;
        }

    }
    return best;

}


//algo de Sutherland Hodgman pour le polygon clipping
void ContactPointDetection::contact_obb_obb(const Entity entityA, const WorldOBB& wobbA, const Transform& transformA, const Entity entityB, const WorldOBB& wobbB, const Transform& transformB, CollisionInfo& collisionInfo){

    //on determine la face de reference et la face incidente

    //collisionInfo.normal = normalize(-collisionInfo.normal);

    vec3 verticesA[8];
    vec3 verticesB[8];

    wobbA.getVertices(verticesA);
    wobbB.getVertices(verticesB);

    vector<Face> facesA;
    vector<Face> facesB;

    wobbA.getFaces(verticesA, facesA);
    wobbB.getFaces(verticesB, facesB);

    /* for (size_t i = 0; i < facesA.size(); ++i) {
        const Face& f = facesA[i];
        Console::getInstance().addLog("facesA[" + std::to_string(i) + "] normal: (" +
            std::to_string(f.normal.x) + ", " +
            std::to_string(f.normal.y) + ", " +
            std::to_string(f.normal.z) + ")");
    }
    for (size_t i = 0; i < facesB.size(); ++i) {
        const Face& f = facesB[i];
        Console::getInstance().addLog("facesB[" + std::to_string(i) + "] normal: (" +
            std::to_string(f.normal.x) + ", " +
            std::to_string(f.normal.y) + ", " +
            std::to_string(f.normal.z) + ")");
    } */


    int minIndexA = supportOBB(verticesA, -collisionInfo.normal);
    int minIndexB = supportOBB(verticesB, collisionInfo.normal);

    Console::getInstance().addLog("minIndexA: " + std::to_string(minIndexA));
    Console::getInstance().addLog("minIndexB: " + std::to_string(minIndexB));

    Face faceA = bestFace(verticesA, facesA, minIndexA, -collisionInfo.normal);
    Face faceB = bestFace(verticesB, facesB, minIndexB, collisionInfo.normal);

    Console::getInstance().addLog("FaceA normal: (" + std::to_string(faceA.normal.x) + ", " + std::to_string(faceA.normal.y) + ", " + std::to_string(faceA.normal.z) + ")");
    Console::getInstance().addLog("FaceB normal: (" + std::to_string(faceB.normal.x) + ", " + std::to_string(faceB.normal.y) + ", " + std::to_string(faceB.normal.z) + ")");

    float alignementA = dot(faceA.normal, -collisionInfo.normal);
    float alignementB = dot(faceB.normal, collisionInfo.normal);

    Console::getInstance().addLog(("entityA: " + std::to_string(entityA) + ", entityB: " + std::to_string(entityB)).c_str());
    Console::getInstance().addLog(("alignementA: " + std::to_string(alignementA) + ", alignementB: " + std::to_string(alignementB)).c_str());
    Console::getInstance().addLog(("Collision normal: " + std::to_string(collisionInfo.normal.x) + ", " + std::to_string(collisionInfo.normal.y) + ", " + std::to_string(collisionInfo.normal.z)).c_str());

    Face referenceFace;
    Face incidentFace;

    vector<Face> referenceOBBFaces;
    vector<vec3> referenceOBBVertices;
    vec3 newClippingNormal;

 
    float volumeA = 8.0f * wobbA.halfSize.x * wobbA.halfSize.y * wobbA.halfSize.z;
    float volumeB = 8.0f * wobbB.halfSize.x * wobbB.halfSize.y * wobbB.halfSize.z;

    if (volumeA < volumeB) {
        referenceFace = faceA;
        incidentFace = faceB;
        referenceOBBVertices = vector<vec3>(verticesA, verticesA + 8);
        referenceOBBFaces = facesA;
        newClippingNormal = -collisionInfo.normal;
        Console::getInstance().addLog("Reference: A (smaller), Incident: B");
    } else {
        referenceFace = faceB;
        incidentFace = faceA;
        referenceOBBVertices = vector<vec3>(verticesB, verticesB + 8);
        referenceOBBFaces = facesB;
        newClippingNormal = collisionInfo.normal;
        Console::getInstance().addLog("Reference: B (smaller), Incident: A");
    }


    vec3 u = normalize(referenceFace.vertices[1] - referenceFace.vertices[0]);
    vec3 v = normalize(referenceFace.vertices[3] - referenceFace.vertices[0]);

    vec3 faceCenter = (referenceFace.vertices[0] + referenceFace.vertices[1] + referenceFace.vertices[2] + referenceFace.vertices[3]) * 0.25f;

    vector<pair<vec3, vec3>> clipPlanes = {
        {v, faceCenter},
        {-v, faceCenter},
        {u, faceCenter},
        {-u, faceCenter}
    };

    vector<vec3> clipped = incidentFace.vertices;
    Console::getInstance().addLog(("start clipped " + std::to_string(clipped.size())).c_str());
 

    for(auto& [n,p] : clipPlanes){

        vec3 planeNormal = n;
        vec3 planePoint = p;

        if(dot(planeNormal, referenceFace.normal) < 0.0f){
            planeNormal = -planeNormal;
        }

        clipped = clipToPlane(clipped, planePoint, planeNormal);

        Console::getInstance().addLog(("clipped " + std::to_string(clipped.size())).c_str());
 

        if(clipped.empty()) break;
    }

    //clipping


    vector<vec3> manifold;
    float D = dot(referenceFace.normal, referenceFace.vertices[0]);
    for (auto& q : clipped) {
        float d = dot(referenceFace.normal, q) - D;
        vec3 pa = q - referenceFace.normal * d;
        manifold.push_back(pa);
    }

    collisionInfo.collisionPoints = manifold;

    Console::getInstance().addLog(("final Contact points: " + std::to_string(collisionInfo.collisionPoints.size())).c_str());
 
}


void ContactPointDetection::contact_sphere_sphere(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const SphereCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    //pas besoin on le fait deja dans test sphere vs sphere
}

void ContactPointDetection::contact_sphere_obb(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const OBBCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void ContactPointDetection::contact_obb_plane(const Entity entityA, const OBBCollider& colliderA, const Transform& transformA, const Entity entityB, const PlaneCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}

void ContactPointDetection::contact_sphere_plane(const Entity entityA, const SphereCollider& colliderA, const Transform& transformA, const Entity entityB, const PlaneCollider& colliderB, const Transform& transformB, CollisionInfo& collisionInfo) {
    // Empty implementation
}