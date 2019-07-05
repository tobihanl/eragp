
#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"

class LivingEntity : public Entity {
private:

public:
    LivingEntity(int x, int y);
    void render() override;
    void tick() override;

    double distanceToPoint(int x, int y);
};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
