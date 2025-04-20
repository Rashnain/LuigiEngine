#ifndef MESHLOADER_HPP
#define MESHLOADER_HPP

#include <vector>
#include <string>
#include "glm/glm.hpp" 
#include "external/OBJ_Loader.h"
#include <cstdio>
namespace objl {
    class Loader;
}

using namespace std;
using namespace glm;


void loadOBJ(const char* fileName, vector<vec3>& vertices, vector<vec3>& normals,
             vector<vec2>& uvs, vector<unsigned int>& triangles);


struct Mesh {
    vector<vec3> vertices;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<unsigned int> triangles;

 
    Mesh() = default;


    explicit Mesh(const string& objFile);
};

#endif // MESHLOADER_HPP
