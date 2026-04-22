#ifndef FLYWEIGHT_H
#define FLYWEIGHT_H

#include "Mesh.h"
#include <unordered_map>
#include <string>

class MeshFlyweight {
public:
    Mesh mesh;
    unsigned int textureID;
    
    MeshFlyweight(const Mesh& mesh, unsigned int textureID) 
        : mesh(mesh), textureID(textureID) {}

    void draw(class Shader& shader) const;
};

class FlyweightFactory {
private:
    std::unordered_map<std::string, MeshFlyweight*> pool;

public:
    ~FlyweightFactory();
    
    MeshFlyweight* get(const std::string& key);
    void add(const std::string& key, const Mesh& mesh, unsigned int textureID);
};

#endif
