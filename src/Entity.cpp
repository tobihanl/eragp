#include "Entity.h"
#include "Renderer.h"
#include "World.h"

int Entity::idCounter = 0;

Entity::Entity(int startX, int startY, const SDL_Color &color, int radius) :
        x(startX),
        y(startY),
        texture(Renderer::renderDot(radius, color)),
        id((idCounter++ * World::getMPINodes()) + World::getMPIRank()) {

}

Entity::Entity(int id, int x, int y, const SDL_Color &color, int radius) : Entity(x, y, color, radius) {
    this->id = id;
}

Entity::Entity(int i, int ix, int iy, const SDL_Color &color, float size) :
        texture(Renderer::renderDot((int) ((1.0f + size) * TILE_SIZE / 2), color)),
        id(i),
        x(ix),
        y(iy) {

}

Entity::~Entity() {
    Renderer::cleanup(texture);
}
