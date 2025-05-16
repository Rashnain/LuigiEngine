#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include <GL/glew.h>

//#include "Texture.hpp"
#include "Mesh.hpp"

#include <unordered_map>
#include <string>
#include <vector>


class RessourceManager {
public:
    static RessourceManager& getInstance();

    enum class MeshType {
        SPHERE,
        TERRAIN,
        MESH
    };


    Mesh* addMesh(std::string meshId);
    Mesh* getMesh(std::string meshId);
    std::vector<const char*> getMeshesNames();


private:
    RessourceManager(){};
    std::unordered_map<std::string, Mesh> meshes;
};

#endif // RESSOURCES_MANAGER_HPP