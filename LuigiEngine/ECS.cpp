#include "ECS.h"
#include <iostream>

/* 
struct Position {

    Position (){x = 0; y = 0;};

    Position(float xin, float yin){x = xin; y=yin;}

    float x, y;
};

struct Velocity {
    float dx, dy;
};

 int main() {
    Registry registry;

    //creer une entite 
    Entity e1 = registry.create();
    Entity e2 = registry.create();

    //attacher un composant
    registry.emplace<Position>(e1, 10.f, 20.f); //attache un composant position a l entite 1
    registry.emplace<Velocity>(e1, 1.f, 1.f); //pareil pour la velocite

    registry.emplace<Position>(e2);
    registry.emplace<Velocity>(e2, -1.f, 0.5f);

    float vitesse = 5.0f;

    //recuperer un composant sur un objet
    Position & pos = registry.get<Position>(e1);
    pos.x += 666;

    //recuperer toutes les entités avec les composants indiquer
    auto view = registry.view<Position, Velocity>(); //permet d iterer sur tous les composants qui ont une position et une velocite 
    
    //2 facons d'iterer sur cette view 

    //methode 1 : mettre les variables externe comme &vitesse avant
    view.each([&vitesse](Entity e, Position & pos, Velocity & vel) {

        std::cout << "entite " << e << " pos : (" << pos.x << ", " << pos.y << ")\n";
        pos.x += vitesse;

    }); 

    //methode 2 : plus propre 
    for (Entity e : view) {
        auto& pos = view.get<Position>(e);
        auto& vel = view.get<Velocity>(e);

        pos.x += 99999 * vitesse;

    }


    //verifie si une entite a un composant
    if(registry.has<Position>(e1)){ 
        std::cout << "l'entite 1 a un composant position" << std::endl;
    }


    auto all_positions = registry.view<Position>();
    for (Entity e : all_positions) {
        auto& pos = all_positions.get<Position>(e);
        std::cout << "Entity " << e << " Position: (" << pos.x << ", " << pos.y << ")\n";
    }
} 
 */