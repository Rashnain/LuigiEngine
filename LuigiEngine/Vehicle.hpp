#ifndef VEHICLE_HPP
#define VEHICLE_HPP
#include "Physics.hpp"

struct VehicleComponent{

    RigidBodyComponent* Chassis;

    RigidBodyComponent* FLWheel;
    RigidBodyComponent* FRWheel;

    RigidBodyComponent* BLWheel;
    RigidBodyComponent* BRWheel;

};

#endif