
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