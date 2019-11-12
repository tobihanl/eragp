#ifndef ERAGP_MAIMUC_EVO_2019_ENTITY_H
#define ERAGP_MAIMUC_EVO_2019_ENTITY_H

#include <SDL.h>
#include <cmath>

class Entity {
private:
    static int idCounter;

    friend bool operator==(const Entity &lhs, const Entity &rhs) { return lhs.id == rhs.id; }

    friend bool operator!=(const Entity &lhs, const Entity &rhs) { return !(lhs == rhs); }

protected:
    SDL_Texture *texture;
    int id;

public:
    int x, y;

    Entity(int x, int y, const SDL_Color &color, int radius);

    Entity(int id, int x, int y, const SDL_Color &color, int radius);

    Entity(int id, int x, int y, const SDL_Color &color, float size);

    virtual ~Entity();

    virtual void render() = 0;

    virtual void tick() = 0;

    virtual int serializedSize() = 0;

    virtual void serialize(void *&ptr) = 0;

    /**
     * Calculates distance to a given position
     * @param px the x coordinate of the position
     * @param py the y coordinate of the position
     * @return the distance
     */
    float getDistance(int px, int py) {
        return std::sqrt((x - px) * (x - px) + (y - py) * (y - py));
    }

    /**
     * Calculates the squared distance to a given position.
     * Might be used when for example only comparing two distances is important.
     * More efficient because no sqrt() and no conversion to float is required.
     * @param px the x coordinate of the position
     * @param py the y coordinate of the position
     * @return the squared distance
     */
    int getSquaredDistance(int px, int py) {
        return (x - px) * (x - px) + (y - py) * (y - py);
    }

    int getId() {
        return id;
    }
};

#endif //ERAGP_MAIMUC_EVO_2019_ENTITY_H
