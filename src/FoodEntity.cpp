#include "FoodEntity.h"
#include "Renderer.h"

FoodEntity::FoodEntity(int startX, int startY) : Entity(startX, startY) {}

void FoodEntity::render() {
    Renderer::renderDot(x, y, 5, {255, 0, 0, 0});
}

void FoodEntity::tick() {
    //TODO
}