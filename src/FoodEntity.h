#ifndef ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
#define ERAGP_MAIMUC_EVO_2019_FOODENTITY_H

#include "Entity.h"

#define AMOUNT_OF_FOOD_PARAMS 4

class FoodEntity : public Entity {
private:

public:
    int energy;

    FoodEntity(int x, int y, int energy);

    explicit FoodEntity(void *&ptr);

    struct RenderData getRenderData() override;

    int serializedSize() override {
        return AMOUNT_OF_FOOD_PARAMS * 4;
    }

    void serialize(void *&ptr) override {
        ((int *) ptr)[0] = id;
        ((int *) ptr)[1] = x;
        ((int *) ptr)[2] = y;
        ((int *) ptr)[3] = energy;
        ptr = static_cast<int *>(ptr) + AMOUNT_OF_FOOD_PARAMS;
    }
};


#endif //ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
