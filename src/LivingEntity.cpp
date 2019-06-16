
#include "LivingEntity.h"
#include "Renderer.h"

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c) : Entity(startX, startY), color(c) {}

void LivingEntity::render() {
    Renderer::renderDot(x, y, 5, color);
}

void LivingEntity::tick() {
    //TODO
}