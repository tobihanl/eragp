#include "FoodEntity.h"
#include "Renderer.h"
#include "World.h"

#define AMOUNT_OF_PARAMS 4

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
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_PARAMS;
}

void FoodEntity::render() {
    Renderer::copy(texture, x - World::getWorldDim().p.x - 4, y - World::getWorldDim().p.y - 4);
}

void FoodEntity::tick() {}

int FoodEntity::serializedSize() {
    return AMOUNT_OF_PARAMS * 4;
}

void FoodEntity::serialize(void *&ptr) {
    ((int *) ptr)[0] = id;
    ((int *) ptr)[1] = x;
    ((int *) ptr)[2] = y;
    ((int *) ptr)[3] = energy;
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_PARAMS;
}
