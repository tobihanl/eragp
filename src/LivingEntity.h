#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"
#include "Brain.h"
#include "Tile.h"
#include "Lfsr.h"

#define AMOUNT_32_BIT_LIVING_PARAMS 6
#define AMOUNT_64_BIT_LIVING_PARAMS 1
#define AMOUNT_FLOAT_LIVING_PARAMS 4

class LivingEntity : public Entity {
private:
    // when adding new properties, adapt squaredDifference() as well
    float speed;
    float size;
    float waterAgility;
    float rotation;

    LFSR random;

    int cooldown;

    float energyLossBase;

    friend std::ostream &operator<<(std::ostream &strm, const LivingEntity &e);

    static float energyLossPerTick(float size) {
        return (int) round(size * 4 + 1);
    }

    float calculateDanger();

public:
    Brain *brain;

    LivingEntity(int x, int y, Color color, float speed, float size, float waterAgility, Brain *brain, uint32_t seed);

    LivingEntity(void *&ptr, bool minimal);

    ~LivingEntity() override;

    struct RenderData getRenderData() override;

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
        return AMOUNT_32_BIT_LIVING_PARAMS * (int) sizeof(uint32_t) +
               AMOUNT_64_BIT_LIVING_PARAMS * (int) sizeof(uint64_t) +
               AMOUNT_FLOAT_LIVING_PARAMS * (int) sizeof(float);
    }

    int fullSerializedSize() override {
        return minimalSerializedSize() + ((brain != nullptr) ? brain->serializedSized() : 0);
    }

    bool visibleOn(Tile *tile) {
        return (color.r - tile->color.r) * (color.r - tile->color.r)
               + (color.g - tile->color.g) * (color.g - tile->color.g)
               + (color.b - tile->color.b) * (color.b - tile->color.b) >= 200;
    }

};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
