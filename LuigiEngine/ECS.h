#include <iostream>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>

using Entity = uint32_t;

struct IComponentStorage {
    virtual void remove(Entity entity) = 0;
    virtual ~IComponentStorage() = default;
};

//globalement c'est juste une hashmap qui contient des pairs (entity, composant) pour un certain type de composant
template<typename Component>
class ComponentStorage : public IComponentStorage {
public:
    void add(Entity entity, Component component) {
        m_data[entity] = std::move(component);
    }

    void remove(Entity entity) {
        m_data.erase(entity);
    }

    bool has(Entity entity) const{
        return m_data.find(entity) != m_data.end();
    }

    Component & get(Entity entity) {
        return m_data.at(entity);
    }

    const std::unordered_map<Entity, Component> & data() const {
        return m_data;
    }

private:
    std::unordered_map<Entity, Component> m_data;
};

//gestionnaire d'entity
class Registry {
public:
    Entity create() {
        return m_nextEntity++;
    }

    //permet d'ajouter un composant a une entity avec emplace<NomComposant>(entityid, args composant)
    template<typename Component, typename... Args>
    Component& emplace(Entity entity, Args&&... args) {
        auto& storage = getStorage<Component>();
        storage.add(entity, Component{ std::forward<Args>(args)... });

        auto& component = storage.get(entity);
        component.onAttach(entity, *this);

        return component;
    }


    //retire un composant avec remove<NomComposant>
    template<typename Component>
    void remove(Entity entity) {
        auto& storage = getStorage<Component>();

        auto& component = storage.get(entity);
        component.onDetach(entity, *this);

        storage.remove(entity);
    }

    //get<Composant>(entityid)
    template<typename Component>
    Component& get(Entity entity) {
        auto& storage = getStorage<Component>();
        return storage.get(entity);
    }

    //has<Composant>(entityid)
    template<typename Component>
    bool has(Entity entity) const {
        return getStorage<Component>().has(entity);
    }

    //permet d'obtenir toutes les entitity avec les composants demandes
    //registry.view<Position, Velocity>([](Entity e, Position& pos, Velocity& vel){ boucle sur toutes les entités avec position et velocity }
    template<typename... Components, typename Func>
    void view(Func&& func) {

        const auto& storage = getStorage<std::tuple_element_t<0, std::tuple<Components...>>>().data();
        for (const auto& [entity, comp] : storage) {
            if ((getStorage<Components>().has(entity) && ...)) {
                func(entity, getStorage<Components>().get(entity)...);
            }
        }
    }


    //comme view mais permet d'itérer sur les paires uniques d'entités 
    /*
    registry.viewPairs<Position>([](Entity a, Entity b, Position& posA, Position& posB) {
        float dist = glm::distance(posA.pos, posB.pos);
        if (dist < 1.0f) {
            std::cout << "Entities " << a << " and " << b << " are too close!" << std::endl;
        }
    }); 
    */
    template<typename... Components, typename Func>
    void viewPairs(Func&& fn) {
        auto view = view<Components...>();
        auto begin = view.begin();
        auto end = view.end();

        for (auto itA = begin; itA != end; ++itA) {
            Entity a = *itA;
            auto& compA = std::forward_as_tuple(get<Components>(*itA)...);

            for (auto itB = std::next(itA); itB != end; ++itB) {
                Entity b = *itB;
                auto& compB = std::forward_as_tuple(get<Components>(*itB)...);

                std::apply([&](auto&... compsA) {
                    std::apply([&](auto&... compsB) {
                        fn(a, b, compsA..., compsB...);
                    }, compB);
                }, compA);
            }
        }
    }


private:
    Entity m_nextEntity = 0;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_componentPools;

    //renvoie le storage
    template<typename Component>
    ComponentStorage<Component>& getStorage() const {
        auto typeId = std::type_index(typeid(Component));
        auto it = m_componentPools.find(typeId);
        assert(it != m_componentPools.end());
        return *static_cast<ComponentStorage<Component>*>(it->second.get());
    }

    //crée le storage pour le composant si il n'existe pas 
    template<typename Component>
    ComponentStorage<Component>& getStorage() {
        auto typeId = std::type_index(typeid(Component));
        auto& ptr = m_componentPools[typeId];
        if (!ptr) {
            ptr = std::make_unique<ComponentStorage<Component>>();
        }
        return *static_cast<ComponentStorage<Component>*>(ptr.get());
    }
};

//un composant doit avoir la fonction onAttach, onDetach meme si vide
struct Position {
    float x, y;
    void onAttach(Entity entity, Registry & registry) {}
    void onDetach(Entity entity, Registry & registry) {}
};

struct Velocity {
    float dx, dy;

    void onAttach(Entity entity, Registry & registry) {
        std::cout << "Velocity attached to " << entity << "\n";
    }
    void onDetach(Entity entity, Registry & registry) {}
};

class MovementSystem {
    public:
        void update(Registry & registry){
            //boucle sur toutes les entity avec les composants position et velocity
            registry.view<Position, Velocity>([](Entity e, Position & pos, Velocity & vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
                std::cout << "Entity " << e << " moved to (" << pos.x << ", " << pos.y << ")\n";
            });
        }
};