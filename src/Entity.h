
#ifndef ERAGP_MAIMUC_EVO_2019_ENTITY_H
#define ERAGP_MAIMUC_EVO_2019_ENTITY_H

#include <SDL.h>
class Entity {
private:
    static int currentId;
    int id;
protected:
    SDL_Texture* texture;
public:
    int x, y;
    Entity(int x, int y, const SDL_Color &color, int radius);
    virtual ~Entity();

    virtual void render() = 0;
    virtual void tick() = 0;

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
    bool operator==(const Entity &other) const;
    bool operator!=(const Entity &other) const;
};


#endif //ERAGP_MAIMUC_EVO_2019_ENTITY_H
