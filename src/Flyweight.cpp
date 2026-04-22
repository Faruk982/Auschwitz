#include "Flyweight.h"
#include "../Shader.h"
#include <glad/glad.h>

void MeshFlyweight::draw(Shader& shader) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    mesh.Draw();
}

FlyweightFactory::~FlyweightFactory() {
    for (auto& pair : pool) {
        delete pair.second;
    }
    pool.clear();
}

MeshFlyweight* FlyweightFactory::get(const std::string& key) {
    if (pool.find(key) != pool.end()) {
        return pool[key];
    }
    return nullptr;
}

void FlyweightFactory::add(const std::string& key, const Mesh& mesh, unsigned int textureID) {
    if (pool.find(key) == pool.end()) {
        pool[key] = new MeshFlyweight(mesh, textureID);
    }
}
