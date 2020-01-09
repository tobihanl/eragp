#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"
#include "FoodEntity.h"
#include "Brain.h"
#include "Tile.h"
#include "Lfsr.h"

#define AMOUNT_OF_LIVING_PARAMS 12 // 10 params (4 byte), 1 param (8 byte)

class LivingEntity : public Entity {
private:
    // when adding new properties, adapt squaredDifference() as well
    float speed;
    float size;
    float waterAgility;
    float rotation;

    ThinkResult thoughts;
    int nextX, nextY;
    FoodEntity *nearestFood;
    LivingEntity *nearestEnemy;
    LivingEntity *nearestMate;


    LFSR random;

    int cooldown;

    float energyLossBase;

    friend std::ostream &operator<<(std::ostream &strm, const LivingEntity &e);

    static float energyLossPerTick(float size) {
        return (int) round(size * 4 + 1);
    }

    float calculateDanger();

public:
    bool toBeRemoved = false;
    Brain *brain;

    LivingEntity(int x, int y, Color color, float speed, float size, float waterAgility, Brain *brain, uint32_t seed);

    LivingEntity(void *&ptr, bool minimal);

    ~LivingEntity() override;

    struct RenderData getRenderData() override;

    LivingEntity *breed();

    void think();

    void updateMovement();

    void tick() override;

    float squaredDifference(const LivingEntity &e) {
        return ((float) (e.color.r - color.r) / 255.f) * ((float) (e.color.r - color.r) / 255.f)
               + ((float) (e.color.g - color.g) / 255.f) * ((float) (e.color.g - color.g) / 255.f)
               + ((float) (e.color.b - color.b) / 255.f) * ((float) (e.color.b - color.b) / 255.f)
               + (e.speed - speed) * (e.speed - speed)
               + (e.size - size) * (e.size - size)
               + (e.waterAgility - waterAgility) * (e.waterAgility - waterAgility);
    }

    void addEnergy(int energy);

    void minimalSerialize(void *&ptr) override;

    void fullSerialize(void *&ptr) override;

    int minimalSerializedSize() override {
        return AMOUNT_OF_LIVING_PARAMS * 4;
    }

    int fullSerializedSize() override {
        if (brain == nullptr)
            return minimalSerializedSize();

        return AMOUNT_OF_LIVING_PARAMS * 4 + brain->serializedSized();
    }

    bool visibleOn(Tile *tile) {
        return (color.r - tile->color.r) * (color.r - tile->color.r)
               + (color.g - tile->color.g) * (color.g - tile->color.g)
               + (color.b - tile->color.b) * (color.b - tile->color.b) >= 200;
    }

};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
