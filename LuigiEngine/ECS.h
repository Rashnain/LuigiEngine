#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

using namespace std;

const uint32_t MAX_ENTITIES = 1024; //ne pas mettre valeur max uint32 utilser au maximum max(uint32) -1
const uint32_t INVALID = MAX_ENTITIES - 1; //permet de savoir si un composant est valide (contient un composant pour une certaine entity)

using Entity = uint32_t;

class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual const bool has(Entity entity) = 0;
    virtual void remove(Entity entity) = 0;
};

template<typename Component> //rend la classe générique et utilisable avec n'importe quelle struct Component
class ComponentStorage : public IComponentStorage{ //stocke les composants pour un unique type de composant 
    private:
    //utilise un sparse set https://www.geeksforgeeks.org/sparse-set/
    vector<uint32_t> sparse; // permet de lier les entity à leur composant en gardant les entites proches les unes des autres
    vector<Component> components; //contient les composants 
    vector<Entity> entities; //contient les entités qui ont ce composant  

    public:

    ComponentStorage(){
        sparse.resize(MAX_ENTITIES, INVALID);
        entities.reserve(MAX_ENTITIES); //reserve la memoire mais n'initialise rien (apres reserve la entities.size() = 0 toujours)
        components.reserve(MAX_ENTITIES);
    }

    inline const bool has(Entity entity){return sparse[entity] != INVALID;}

    inline Component & get(Entity entity) {return components[sparse[entity]];}

    inline const std::vector<Entity> & getEntities() {return entities;}

    void add(Entity entity, Component component){

        if(!has(entity)){ //si l'entity n'as pas deja ce composant
            sparse[entity] = entities.size();
            entities.push_back(entity); 
            components.push_back( move(component) ); //move permet de bouger completement le composant dans l'array pour ne pas qu'il soit supprimer si il sort du scope 

        }else{
            components[sparse[entity]] = std::move(component);
        }

    }

    void remove(Entity entity){
        assert(has(entity));

        uint32_t index = sparse[entity];

        Entity lastEntity = entities.back(); //on recup le dernier element pour le mettre a la place de celui qu'on supprime
        Component lastComponent = components.back();

        //on le remplace
        entities[index] = lastEntity; 
        components[index] = std::move(lastComponent);

        sparse[lastEntity] = index; //met a jour le lien
        
        entities.pop_back();
        components.pop_back();
        
        sparse[entity] = INVALID;
    }

};

template<typename... Components> //les ... indique qu'on peut passer plusieur composants View<Position, Velocity> 
class View{ //peut etre ameliorer

    tuple<ComponentStorage<Components>*...> storages;  //un tuple de storage 

public: 
    View( tuple<ComponentStorage<Components>*...>  storages_in){
        storages = storages_in;
    }

    template<typename Component>
    Component & get(Entity e) const {
        ComponentStorage<Component> * storage = std::get<ComponentStorage<Component>*>(storages);
        assert(storage->has(e));
        return storage->get(e);
    }

    template<typename function> 
    void each(function&& func){ // le && permet de garder le type de passage des arguments(reference ou copie) un peu comme std::forward

        auto storageSmallest = std::get<0>(storages); //trouver un moyen simple de choisir le plus petit des storages

        for (Entity e : storageSmallest->getEntities()) {
            if ((std::get<ComponentStorage<Components>*>(storages)->has(e) && ...)) { //si l entity a tous les composants

                func(e, std::get<ComponentStorage<Components>*>(storages)->get(e)...);

            }
        }
    }

    //bien aidé par chatGPT ici c etait galere
    class Iterator {
        public:
            using Iter = typename vector<Entity>::const_iterator;
        
            Iterator(Iter current, Iter end, const tuple<ComponentStorage<Components>*...>& storages): current(current), end(end), storages(storages) {
                skip_invalid();
            }
        
            Entity operator*() const { return *current; }
        
            Iterator& operator++() {
                ++current;
                skip_invalid();
                return *this;
            }
        
            bool operator!=(const Iterator& other) const { return current != other.current; }
        
        private:
            Iter current, end;
            const tuple<ComponentStorage<Components>*...> & storages;
        
            void skip_invalid() {
                while (current != end && !( std::get<ComponentStorage<Components>*>(storages)->has(*current) && ...)) {
                    ++current;
                }
            }
    };

    Iterator begin() const {
        const auto& entities = std::get<0>(storages)->getEntities();
        return Iterator(entities.begin(), entities.end(), storages);
    }

    Iterator end() const {
        const auto& entities = std::get<0>(storages)->getEntities();
        return Iterator(entities.end(), entities.end(), storages);
    }

};

/* exemple :
    auto view = registry.view<Position, Velocity>();

    view.each([](Entity e, const Position & pos, Velocity & vel) {

        std::cout << "Entity " << e << " has pos (" << pos.x << ", " << pos.y << ")\n";

    }); 
*/


class Registry{

private:
    Entity nextEntityID = 0;
    vector<Entity> FreeIDs; //les id des entites qu'on a detruit pour pouvoir les reutiliser

    vector<IComponentStorage *> componentStorages; //permet de savoir quelle composant sont utilisé    

    template<typename Component>
    ComponentStorage<Component> & getComponentStorage(){
        static ComponentStorage<Component> storage; //le static assure que ce n'est initialise qu'une fois et que donc on reutilise le meme storage chaque fois

        static bool isKnown = false; 

        if(!isKnown){ //permet de savoir quelle type de composant on utilise
            isKnown = true;
            componentStorages.push_back(&storage);
        }

        return storage;
    }

public:
    inline Entity create() {
        if(FreeIDs.empty()){
            return nextEntityID++;
        }else{
            Entity newID = FreeIDs.back(); //on recup l'ID du dernier element detruit
            FreeIDs.pop_back();
            return newID;
        }
    }

    //on itere a travers tous les types de composant pour essayer de supprimer l entity
    void destroy(Entity entity){

        for (IComponentStorage* storage : componentStorages) {
            if (storage->has(entity)) {
                storage->remove(entity);
            }
        }

        FreeIDs.push_back(entity);

    }

    template<typename Component>
    inline bool has(Entity entity){
        return getComponentStorage<Component>().has(entity);
    }

    template<typename Component>
    inline Component & get(Entity entity){
        return getComponentStorage<Component>().get(entity);
    }

    template<typename Component>
    inline void remove(Entity entity){
        return getComponentStorage<Component>().remove(entity);
    }

    template<typename Component, typename... Arguments>
    Component & emplace(Entity entity, Arguments &&... arguments){ //ajoute un composant a une entite 
        ComponentStorage<Component> & storage = getComponentStorage<Component>();

        storage.add(entity , Component{ forward<Arguments>(arguments)... } ); //Component{ args } permet d'initialiser le composant
                                                                           //forward permet de conserver les arguments (si ils sont passe par reference, on les passe par reference)
                                                                           // si ils sont passés par copie on utilise les copies

        return storage.get(entity);
    }


    template<typename... Components>
    View<Components...> view() {
        return View<Components...>( make_tuple( &getComponentStorage<Components>()...) );
    }




};



    