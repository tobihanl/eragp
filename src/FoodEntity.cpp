#include "FoodEntity.h"
#include "World.h"

FoodEntity::FoodEntity(int startX, int startY, int e) :
        Entity(startX, startY, {255, 0, 0, 255}, 4, e),
        expire(FOOD_EXPIRATION_TIME) {

}

FoodEntity::FoodEntity(void *&ptr) :
        Entity(((int *) ptr)[0],
               ((int *) ptr)[1],
               ((int *) ptr)[2],
               {255, 0, 0, 255},
               4,
               ((int *) ptr)[3]),
        expire(((int *) ptr)[4]) {
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_FOOD_PARAMS;
}

struct RenderData FoodEntity::getRenderData() {
    return {World::getWorldDim(), radius, color, x, y, 0, false};
}

void FoodEntity::tick() {
    if (--expire < 0)
        World::removeFoodEntity(this, false);
}
