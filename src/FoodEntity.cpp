#include "FoodEntity.h"
#include "World.h"

FoodEntity::FoodEntity(int startX, int startY, int e) :
        Entity(startX, startY, {255, 0, 0, 255}, 4, e),
        expire(FOOD_EXPIRATION_TIME),
        beer(false) {

}

FoodEntity::FoodEntity(void *&ptr) :
        Entity(((int32_t *) ptr)[0],
               ((int32_t *) ptr)[1],
               ((int32_t *) ptr)[2],
               {255, 0, 0, 255},
               4,
               ((int32_t *) ptr)[3]),
        expire(((int32_t *) ptr)[4]),
        beer(false) {
    ptr = static_cast<int32_t *>(ptr) + AMOUNT_32_BIT_FOOD_PARAMS;
}

struct RenderData FoodEntity::getRenderData() {
    return {World::getWorldDim(), radius, color, x, y, 0, false};
}

void FoodEntity::tick() {
    if (--expire < 0)
        World::removeFoodEntity(this, false);
}
