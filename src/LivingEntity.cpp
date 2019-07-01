
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float sp, float si) : Entity(startX, startY), color(c), speed(sp), size(si) {

}

void LivingEntity::render() {
    Renderer::renderDot(x, y, 2 + (size * TILE_SIZE / 2), color);
}

void LivingEntity::tick() {
    //FoodEntity* nearestFood = World::findNearestFood(x, y);
}