
#ifndef ERAGP_MAIMUC_EVO_2019_ENTITY_H
#define ERAGP_MAIMUC_EVO_2019_ENTITY_H

#include <SDL.h>
class Entity {
private:
    static int idCounter;

    friend bool operator==(const Entity &lhs, const Entity &rhs);

    friend bool operator!=(const Entity &lhs, const Entity &rhs);
protected:
    SDL_Texture *texture;
    int id;
public:
    int x, y;

    Entity(int x, int y, const SDL_Color &color, int radius);

    Entity(int id, int x, int y, const SDL_Color &color, float size);

    virtual ~Entity();

    virtual void render() = 0;
    virtual void tick() = 0;

    int getId();

    /**
     * Calculates distance to a given position
     * @param x the x coordinate of the position
     * @param y the y coordinate of the position
     * @return the distance
     */
    float getDistance(int x, int y);

    /**
     * Calculates the squared distance to a given position.
     * Might be used when for example only comparing two distances is important.
     * More efficient because no sqrt() and no conversion to float is required.
     * @param x the x coordinate of the position
     * @param y the y coordinate of the position
     * @return the squared distance
     */
    int getSquaredDistance(int x, int y);
};


#endif //ERAGP_MAIMUC_EVO_2019_ENTITY_H
