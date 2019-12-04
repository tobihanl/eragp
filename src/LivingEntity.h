#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"
#include "Brain.h"
#include "Tile.h"

#define AMOUNT_OF_LIVING_PARAMS 10

class LivingEntity : public Entity {
private:
    float speed;
    float size;
    float waterAgility;
    float rotation;

    int energy;
    int cooldown;

    int energyLossWithMove, energyLossWithoutMove;

    friend std::ostream &operator<<(std::ostream &strm, const LivingEntity &e);

    static int energyLossPerTick(bool move, float speed, float size) {
        return (int) round((move ? speed * 8 : 0) + size * 4 + 1);
    }

public:
    Brain *brain;

    LivingEntity(int x, int y, Color color, float speed, float size, float waterAgility, Brain *brain);

    explicit LivingEntity(void *&ptr);

    ~LivingEntity() override;

    struct RenderData getRenderData() override;

    void tick();

    //TODO consider new properties when added
    float squaredDifference(const LivingEntity &e) {
        return ((float) (e.color.r - color.r) / 255.f) * ((float) (e.color.r - color.r) / 255.f)
               + ((float) (e.color.g - color.g) / 255.f) * ((float) (e.color.g - color.g) / 255.f)
               + ((float) (e.color.b - color.b) / 255.f) * ((float) (e.color.b - color.b) / 255.f)
               + (e.speed - speed) * (e.speed - speed)
               + (e.size - size) * (e.size - size)
               + (e.waterAgility - waterAgility) * (e.waterAgility - waterAgility);//TODO consider brain
    }

    void addEnergy(int energy);

    void serialize(void *&ptr) override;

    int serializedSize() override {
        return AMOUNT_OF_LIVING_PARAMS * 4 + brain->serializedSized();
    }

    bool visibleOn(Tile *tile) {
        return (color.r - tile->color.r) * (color.r - tile->color.r)
               + (color.g - tile->color.g) * (color.g - tile->color.g)
               + (color.b - tile->color.b) * (color.b - tile->color.b) >= 200;
    }

};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
