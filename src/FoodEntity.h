#ifndef ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
#define ERAGP_MAIMUC_EVO_2019_FOODENTITY_H

#include "Entity.h"

#define AMOUNT_OF_FOOD_PARAMS 5
#define FOOD_EXPIRATION_TIME 180 * 4

class FoodEntity : public Entity {
private:
    int expire;

public:
    FoodEntity(int x, int y, int energy);

    explicit FoodEntity(void *&ptr);

    ~FoodEntity() override = default;

    struct RenderData getRenderData() override;

    void tick() override;

    int minimalSerializedSize() override { return fullSerializedSize(); }

    void minimalSerialize(void *&ptr) override { fullSerialize(ptr); }

    int fullSerializedSize() override {
        return AMOUNT_OF_FOOD_PARAMS * 4;
    }

    void fullSerialize(void *&ptr) override {
        ((int *) ptr)[0] = id;
        ((int *) ptr)[1] = x;
        ((int *) ptr)[2] = y;
        ((int *) ptr)[3] = energy;
        ((int *) ptr)[4] = expire;
        ptr = static_cast<int *>(ptr) + AMOUNT_OF_FOOD_PARAMS;
    }
};


#endif //ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
