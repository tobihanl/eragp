
#include "World.h"

World::World() {}

void World::render() {
    //TODO render terrain
    for(auto &f : food) {
        f.render();
    }
    for(auto &e : living) {
        e.render();
    }
}

void World::tick() {
    for(auto &f : food) {
        f.tick();
    }
    for(auto &e : living) {
        e.tick();
    }
}
/*FoodEntity& World::findNearestFood(int x, int y) {
    if(living.size() == 0) return NULL;
    FoodEntity f = living[0];
    int dist = f.getSquaredDistance(x, y);
    for(auto &e : living) {
        int tempDist = e.getSquaredDistance(x, y);
        if(tempDist < dist) {
            f = e;
        }
    }
}*/