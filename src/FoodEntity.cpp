#include "FoodEntity.h"
#include "Renderer.h"

FoodEntity::FoodEntity(int startX, int startY, int e) : Entity(startX, startY), energy(e) {}

void FoodEntity::render() {
    Renderer::renderDot(x, y, 5, {255, 0, 0, 0});
}

void FoodEntity::tick() {}