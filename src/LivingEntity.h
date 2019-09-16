
#ifndef ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
#define ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H

#define ENERGY_FONT_SIZE 12

#include "Entity.h"
#include <SDL.h>
#include "Brain.h"
#include <random>
#include <SDL.h>

class LivingEntity : public Entity {
private:
    static std::mt19937 randomGenerator;
    static std::normal_distribution<float> normalDistribution;


    float rotation; //TODO add to serialization

    SDL_Color color;
    float speed;
    float size;
    float waterAgility; //TODO add to serialization

    int energy;
    int cooldown;

    friend std::ostream &operator<<(std::ostream &strm, const LivingEntity &e);
public:
    static SDL_Texture *digits[];

    Brain *brain;

    LivingEntity(int x, int y, SDL_Color color, float speed, float size, float waterAgility, Brain *brain);

    LivingEntity(void *&ptr);

    ~LivingEntity() override;
    void render() override;
    void tick() override;

    float difference(const LivingEntity &e);

    int serializedSize();
    void serialize(void *&ptr);

};


#endif //ERAGP_MAIMUC_EVO_2019_LIVINGENTITY_H
