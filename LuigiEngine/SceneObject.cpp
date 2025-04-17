#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

extern bool* optimizeMVP;

#include "SceneObject.hpp"

void SceneObject::setMainCamera(SceneCamera* camera) {
	if (camera != mainCamera) {
		mainCamera = camera;
		mainCamera->viewProjChanged = true;
        mainCamera->justDefinedMain = true;
	}
}

void SceneObject::addChild(SceneObject* child)
{
	child->parent = this;
	children->push_back(child);
}

// TODO [TP03] Mise à jour des éléments de la scène (Transform)
void SceneObject::updateSelfAndChildren(double deltaTime)
{
	update(deltaTime);

	if (!*optimizeMVP || (!transform.isUpToDateLocal() || !transform.upToDateGlobal)) {
		if (parent)
			transform.computeGlobalModelMatrix(parent->transform.getGlobalModel());
		else
			transform.computeGlobalModelMatrix();

		for (SceneObject* child : *children)
			child->transform.upToDateGlobal = false;
	}

	for (SceneObject* child : *children)
		child->updateSelfAndChildren(deltaTime);
}

// TODO [TP03] Afficher les éléments de la scène
void SceneObject::renderSelfAndChildren()
{
	render();
	for (SceneObject* child : *children)
		child->renderSelfAndChildren();
}

void SceneObject::clearSelfAndChildren()
{
	clear();
	for (SceneObject* child : *children)
		child->clearSelfAndChildren();
}
