#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <vector> 
#include "Transform.hpp"

using namespace std;  // Add this line or use std::vector explicitly

class SceneCamera;


class SceneObject
{
	SceneObject* parent = nullptr;
	vector<SceneObject*>* children = new vector<SceneObject*>();

protected:
	inline static SceneCamera* mainCamera = nullptr;

	virtual void update(double deltaTime) {}
	virtual void render() {}
	virtual void clear() {}

public:
	Transform transform;

	virtual ~SceneObject() = default;

	static SceneCamera* getMainCamera() { return mainCamera; }

	/**
	 * Sets the scene tree main camera (the one used for displaying the scene)
	 *
	 * WARNING : the camera must be in your scene tree, otherwise it won't display correctly
	 */
	static void setMainCamera(SceneCamera* camera);

	void addChild(SceneObject *child);

	void updateSelfAndChildren(double deltaTime);

	void renderSelfAndChildren();

	void clearSelfAndChildren();
};

#endif //SCENEOBJECT_H
