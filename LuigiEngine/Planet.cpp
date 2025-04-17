#include "SceneMesh.hpp"

// TODO [TP03] Planète
class Planet : public SceneMesh
{
    double rotationSpeed;
    double rotationTilt;
    double radius;
    double orbitSpeed;
    double orbitTilt;
    double orbitAngle = 0;

protected:
    void update(double deltaTime) override
    {
        // rotation
        transform.addEulerRot({0, degrees(rotationSpeed) * deltaTime, 0});
        quat tilt = angleAxis(rotationTilt, dvec3(0.0, 0.0, 1.0));
        quat rotation = angleAxis(radians(transform.getEulerRot().y), vec3(0, 1, 0));
        transform.setRot(tilt * rotation);

        // orbit
        orbitAngle += orbitSpeed * deltaTime;
        double x = cos(orbitAngle) * radius * cos(orbitTilt);
        double z = sin(orbitAngle) * radius;
        double y = sin(orbitTilt) * sin(orbitAngle) * radius;
        transform.setPos({x, y, z});
    }

public:
    Planet(const vector<pair<double, Mesh*>>& meshes, const vector<string>& texFiles, const vector<string>& texUniforms, GLuint programID,
        double rotationSpeed = 0, double rotationTilt = 0, double radius = 0, double orbitSpeed = 0,
        double orbitTilt = 0):
            SceneMesh(meshes, texFiles, texUniforms, programID) {
        this->rotationSpeed = rotationSpeed;
        this->rotationTilt = rotationTilt;
        this->radius = radius;
        this->orbitSpeed = orbitSpeed;
        this->orbitTilt = orbitTilt;
    }
};
