
#ifndef ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
#define ERAGP_MAIMUC_EVO_2019_FOODENTITY_H

#include "Entity.h"

class FoodEntity : public Entity {
private:

public:
    int energy;

    FoodEntity(int x, int y, int energy);

    explicit FoodEntity(void *&ptr);

    ~FoodEntity() override = default;
    void render() override;
    void tick() override;

    int serializedSize() override;

    void serialize(void *&ptr) override;
};


#endif //ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
