#include "FoodEntity.h"
#include "Renderer.h"
#include "World.h"

FoodEntity::FoodEntity(int startX, int startY, int e) : Entity(startX, startY, {255, 0, 0, 255}, 4), energy(e) {}

void FoodEntity::render() {
    Renderer::copy(texture, x - World::getWorldDim().x - 4, y - World::getWorldDim().y - 4);
}

void FoodEntity::tick() {}