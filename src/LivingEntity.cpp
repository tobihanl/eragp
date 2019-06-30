
#include "LivingEntity.h"
#include "Renderer.h"

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float s) : Entity(startX, startY), color(c), speed(s) {}

void LivingEntity::render() {
    Renderer::renderDot(x, y, 5, color);
}

void LivingEntity::tick() {
    //TODO
}