#include "RessourceManager.hpp"


RessourceManager& RessourceManager::getInstance() {
    static RessourceManager instance;
    return instance;
}



Mesh* RessourceManager::addMesh(std::string meshId) {
    auto it = meshes.find(meshId);
    if (it != meshes.end()) {
        return &it->second;
    } else {
        Mesh mesh;
        meshes.insert(std::pair<std::string, Mesh>(meshId, mesh));
        return &meshes.find(meshId)->second;
    }
}

Mesh* RessourceManager::getMesh(std::string meshId) {
    auto it = meshes.find(meshId);
    if (it != meshes.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}

std::vector<const char*> RessourceManager::getMeshesNames() {
    std::vector<const char*> meshNames;
    for (auto const& x : meshes) {
        meshNames.push_back(x.first.c_str());
    }
    return meshNames;
}


