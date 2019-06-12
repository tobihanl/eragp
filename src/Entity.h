
#ifndef ERAGP_MAIMUC_EVO_2019_ENTITY_H
#define ERAGP_MAIMUC_EVO_2019_ENTITY_H


class Entity {
private:

public:
    int x, y;
    Entity(int x, int y);

    virtual void render() = 0;
    virtual void tick() = 0;
};


#endif //ERAGP_MAIMUC_EVO_2019_ENTITY_H
