extern int* nbLocalMatrixUpdate;
extern int* nbGlobalMatrixUpdate;
extern bool* optimizeMVP;

#include "Transform.hpp"

using namespace glm;
using namespace std;



void Transform::computeLocalModelMatrix()
{
	if (*optimizeMVP && upToDateLocal) return;

	nbLocalMatrixUpdate = new int(*nbLocalMatrixUpdate + 1);

	// Si on a pas donné de rotation en quaternion alors on calcule la rotation avec les angles d'Euler
	if (localRot == mat4(1.0f))
		localRot = toMat4(quat(radians(localEulerRot)));

	// translation * rotation * scale (also know as TRS matrix)
	localModel = translate(mat4(1.0f), localPos) * localRot * scale(mat4(1.0f), localScale);

	upToDateLocal = true;
	changed = true;
}

void Transform::addPos(const vec3& vec) {
	localPos += vec;
	upToDateLocal = false;
}

void Transform::setPos(vec3 vec) {
	localPos = vec;
	upToDateLocal = false;
}

void Transform::addEulerRot(const vec3& vec) {
	localEulerRot += vec;
	upToDateLocal = false;
}

void Transform::setEulerRot(vec3 vec) {
	localEulerRot = vec;
	upToDateLocal = false;
}

void Transform::setRot(const quat& quatRot) {
	localRot = toMat4(quatRot);
	upToDateLocal = false;
}

void Transform::addScale(const vec3& vec) {
	localScale = vec;
	upToDateLocal = false;
}

void Transform::setScale(vec3 vec) {
	localScale = vec;
	upToDateLocal = false;
}

void Transform::computeGlobalModelMatrix() {
	computeLocalModelMatrix();
	globalModel = localModel;
	nbGlobalMatrixUpdate = new int(*nbGlobalMatrixUpdate + 1);
	upToDateGlobal = true;
	changed = true;
}

void Transform::computeGlobalModelMatrix(const mat4& parentModel) {
	computeGlobalModelMatrix();
	globalModel = parentModel * globalModel;
}


void TransformSystem::update(Registry & registry) {
    for (Entity entity : registry.view<Transform>()) {
        if (!registry.has<Hierarchy>(entity)) {
            computeGlobalTransform(entity, registry, glm::mat4(1.0f));

        } else {
            const auto & hierarchy = registry.get<Hierarchy>(entity);
            if (hierarchy.parent == INVALID) {
                computeGlobalTransform(entity, registry, glm::mat4(1.0f));
            }
        }
    }
}


void TransformSystem::computeGlobalTransform(Entity entity, Registry & registry, const glm::mat4 & parentModel) {
    auto& transform = registry.get<Transform>(entity);
    transform.computeGlobalModelMatrix(parentModel);

    if (registry.has<Hierarchy>(entity)) {
        auto & hierarchy = registry.get<Hierarchy>(entity);
        for (Entity child : hierarchy.children) {
            computeGlobalTransform(child, registry, transform.getGlobalModel());
        }
    }
}
