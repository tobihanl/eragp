#ifndef ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
#define ERAGP_MAIMUC_EVO_2019_FOODENTITY_H

#include "Entity.h"

#define AMOUNT_32_BIT_FOOD_PARAMS 6
#define FOOD_EXPIRATION_TIME (180 * 4)

class FoodEntity : public Entity {
private:
    int expire;

public:
    bool beer;
    FoodEntity(int x, int y, int energy);

    explicit FoodEntity(void *&ptr);

    ~FoodEntity() override = default;

    struct RenderData getRenderData() override;

    void tick() override;

    int minimalSerializedSize() override { return fullSerializedSize(); }

    void minimalSerialize(void *&ptr) override { fullSerialize(ptr); }

    int fullSerializedSize() override {
        return AMOUNT_32_BIT_FOOD_PARAMS * sizeof(int32_t);
    }

    void fullSerialize(void *&ptr) override {
        ((int32_t *) ptr)[0] = (int32_t) id;
        ((int32_t *) ptr)[1] = (int32_t) x;
        ((int32_t *) ptr)[2] = (int32_t) y;
        ((int32_t *) ptr)[3] = (int32_t) energy;
        ((int32_t *) ptr)[4] = (int32_t) expire;
        ((int32_t *) ptr)[5] = (beer) ? -1 : 0;
        ptr = static_cast<int32_t *>(ptr) + AMOUNT_32_BIT_FOOD_PARAMS;
    }
};


#endif //ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
