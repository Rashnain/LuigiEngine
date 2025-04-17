#include "ECS.h"
#include <iostream>

/* int main() {
    Registry registry;
    MovementSystem move = MovementSystem();

    Entity e1 = registry.create();
    Entity e2 = registry.create();

    registry.emplace<Position>(e1, 10.f, 20.f);
    registry.emplace<Velocity>(e1, 1.f, 1.f);

    registry.emplace<Position>(e2, 50.f, 60.f);
    registry.emplace<Velocity>(e2, -1.f, 0.5f);

    //move.update(registry);

    Position & pos = registry.get<Position>(e1);

    move.update(registry);

    std::cout << "position is " << pos.x << ", " << pos.y << std::endl;
} */
