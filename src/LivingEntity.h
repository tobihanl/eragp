
#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#include "Entity.h"
#include <SDL.h>

class LivingEntity : public Entity {
private:
    SDL_Color color;
    float speed;
    //Matrix& brain;
public:
    LivingEntity(int x, int y, SDL_Color color, float speed);
    void render() override;
    void tick() override;
};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
