
#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"
#include <SDL.h>
#include "Brain.h"

class LivingEntity : public Entity {
private:
    SDL_Color color;
    float speed;
    float size;
    Brain* brain;

    int energy;
    int cooldown;
public:
    LivingEntity(int x, int y, SDL_Color color, float speed, float size, Brain* brain);
    void render() override;
    void tick() override;
    virtual ~LivingEntity();
};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
