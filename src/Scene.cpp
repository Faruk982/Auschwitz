#include "Scene.h"

void Scene::init() {
    environment.init(factory);
    barrackGrid.init(factory);
    fenceSystem.init(factory);
    entranceZone.init(factory);
    crematoryZone.init(factory);
}

void Scene::updateTrainMovement(float forwardInput, float lateralInput, float deltaTime) {
    entranceZone.updateTrainMovement(forwardInput, lateralInput, deltaTime);
}

void Scene::setBarrackDoorsOpen(bool open) {
    barrackGrid.setDoorsOpen(open);
    crematoryZone.setDoorsOpen(open);
}

void Scene::setInteriorLightsOn(bool on) {
    barrackGrid.setInteriorLightsOn(on);
    crematoryZone.setInteriorLightsOn(on);
}

void Scene::render(Shader& shader) {
    environment.render(shader);
    barrackGrid.render(shader);
    fenceSystem.render(shader);
    entranceZone.render(shader);
    crematoryZone.render(shader);
}
