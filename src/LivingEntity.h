#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#define ENERGY_FONT_SIZE 12

#include "Entity.h"
#include <SDL.h>
#include "Brain.h"
#include <random>
#include <SDL.h>
#include "Tile.h"

class LivingEntity : public Entity {
private:
    static std::mt19937 randomGenerator;
    static std::normal_distribution<float> normalDistribution;

    SDL_Color color;
    float speed;
    float size;
    float waterAgility;
    float rotation;

    int energy;
    int cooldown;

    int energyLossWithMove, energyLossWithoutMove;
    static int energyLossPerTick(bool move, float speed, float size);

    friend std::ostream &operator<<(std::ostream &strm, const LivingEntity &e);
public:
    static SDL_Texture *digits[];

    Brain *brain;

    LivingEntity(int x, int y, SDL_Color color, float speed, float size, float waterAgility, Brain *brain);

    explicit LivingEntity(void *&ptr);

    ~LivingEntity() override;
    void render() override;
    void tick() override;

    bool visibleOn(Tile *tile);
    float difference(const LivingEntity &e);

    int serializedSize() override;

    void serialize(void *&ptr) override;

};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
