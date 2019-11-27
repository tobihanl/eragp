#include "FoodEntity.h"
#include "World.h"
#include "Renderer.h"

FoodEntity::FoodEntity(int startX, int startY, int e) :
        Entity(startX, startY, {255, 0, 0, 255}, 4),
        energy(e) {

}

FoodEntity::FoodEntity(void *&ptr) :
        Entity(((int *) ptr)[0],
               ((int *) ptr)[1],
               ((int *) ptr)[2],
               {255, 0, 0, 255},
               4),
        energy(((int *) ptr)[3]) {
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_FOOD_PARAMS;
}

void FoodEntity::render() {
    Renderer::copy(texture, x - World::getWorldDim().p.x - 4, y - World::getWorldDim().p.y - 4);
}
