#include "Vehicle.hpp"
#include "ImGuiConsole.hpp"

void VehicleSystem::update(Registry& registry, float deltaTime){

    auto view = registry.view<VehicleComponent>();

    for(auto entity : view){
        VehicleComponent& vehicle = registry.get<VehicleComponent>(entity);

        float flWheelYScale = registry.get<Transform>(vehicle.FLWheel).getScale().y * 0.3f;
        vec3 flSuspensionDir = -normalize(vehicle.FLSuspension->direction);
        vec3 flOffset = flSuspensionDir * flWheelYScale * 0.5f;

        float frWheelYScale = registry.get<Transform>(vehicle.FRWheel).getScale().y * 0.3f;
        vec3 frSuspensionDir = -normalize(vehicle.FRSuspension->direction);
        vec3 frOffset = frSuspensionDir * frWheelYScale * 0.5f;

        float blWheelYScale = registry.get<Transform>(vehicle.BLWheel).getScale().y * 0.3f;
        vec3 blSuspensionDir = -normalize(vehicle.BLSuspension->direction);
        vec3 blOffset = blSuspensionDir * blWheelYScale * 0.5f;

        float brWheelYScale = registry.get<Transform>(vehicle.BRWheel).getScale().y * 0.3f;
        vec3 brSuspensionDir = -normalize(vehicle.BRSuspension->direction);
        vec3 brOffset = brSuspensionDir * brWheelYScale * 0.5f;

        if(!vehicle.FLSuspension->isColliding){
            registry.get<Transform>(vehicle.FLWheel).setPos(vehicle.FLSuspension->endPointWorld + flOffset);
        } else {
            registry.get<Transform>(vehicle.FLWheel).setPos(vehicle.FLSuspension->collisionPoint + flOffset);
        }
        

        if(!vehicle.FRSuspension->isColliding){
            registry.get<Transform>(vehicle.FRWheel).setPos(vehicle.FRSuspension->endPointWorld + frOffset);
        } else {
            registry.get<Transform>(vehicle.FRWheel).setPos(vehicle.FRSuspension->collisionPoint + frOffset);
        }

        if(!vehicle.BLSuspension->isColliding){
            registry.get<Transform>(vehicle.BLWheel).setPos(vehicle.BLSuspension->endPointWorld + blOffset);
        } else {
            registry.get<Transform>(vehicle.BLWheel).setPos(vehicle.BLSuspension->collisionPoint + blOffset);
        }

        if(!vehicle.BRSuspension->isColliding){
            registry.get<Transform>(vehicle.BRWheel).setPos(vehicle.BRSuspension->endPointWorld + brOffset);
        } else {
            registry.get<Transform>(vehicle.BRWheel).setPos(vehicle.BRSuspension->collisionPoint + brOffset);
        }

        registry.get<Transform>(vehicle.FLWheel).setRot(registry.get<Transform>(entity).getRot());
        registry.get<Transform>(vehicle.FRWheel).setRot(registry.get<Transform>(entity).getRot());
        registry.get<Transform>(vehicle.BLWheel).setRot(registry.get<Transform>(entity).getRot());
        registry.get<Transform>(vehicle.BRWheel).setRot(registry.get<Transform>(entity).getRot());


        std::string logMsg = "endPointWorld: (" +
            std::to_string(vehicle.FLSuspension->endPointWorld.x) + ", " +
            std::to_string(vehicle.FLSuspension->endPointWorld.y) + ", " +
            std::to_string(vehicle.FLSuspension->endPointWorld.z) + "), collisionPoint: (" +
            std::to_string(vehicle.FLSuspension->collisionPoint.x) + ", " +
            std::to_string(vehicle.FLSuspension->collisionPoint.y) + ", " +
            std::to_string(vehicle.FLSuspension->collisionPoint.z) + ")";
        //Console::getInstance().addLog(logMsg);
        
    }

}