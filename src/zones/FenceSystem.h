#ifndef FENCESYSTEM_H
#define FENCESYSTEM_H

#include "../Flyweight.h"
#include "../../Shader.h"
#include <vector>
#include <glm/glm.hpp>

class FenceSystem {
public:
    void init(FlyweightFactory& factory);
    void render(Shader& shader);
private:
    MeshFlyweight* postFlyweight;
    MeshFlyweight* towerBaseFlyweight;
    MeshFlyweight* towerCabinFlyweight;
    MeshFlyweight* towerRoofFlyweight;
    MeshFlyweight* wireFlyweight;
    MeshFlyweight* sideGateFlyweight;
    MeshFlyweight* treeBranchFlyweight;

    std::vector<glm::vec3> postPositions;
    std::vector<glm::vec3> towerPositions;
    std::vector<glm::vec3> treePositions;

    void renderTower(Shader& shader, const glm::vec3& pos);
    void renderTree(Shader& shader, const glm::vec3& pos, float seed);
    void renderTreeBranch(Shader& shader, const glm::vec3& start, const glm::vec3& dir, float length, float radius, int depth, int maxDepth, float seed);
};

#endif
