#include <cmath>
#include <cassert>
#include "LivingEntity.h"
#include "World.h"
#include "FoodEntity.h"
#include "Rng.h"

#define PI 3.14159265
#define MAX_ENERGY 9999

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, Color c, float sp, float si, float wa, Brain *b) :
        Entity(startX, startY, c, (int) ((1.0f + si) * TILE_SIZE / 2)),
        speed(sp >= 0 ? sp : 0),
        size(si >= 0 ? si : 0),
        waterAgility(wa < 0 ? 0 : (wa > 1 ? 1 : wa)),
        brain(b),
        energy(60 * 2),
        cooldown(60),
        rotation(0.0f),
        energyLossWithMove(energyLossPerTick(true, sp, si)),
        energyLossWithoutMove(energyLossPerTick(false, sp, si)) {

}

LivingEntity::LivingEntity(void *&ptr) :
        Entity(((int *) ptr)[0],
               ((int *) ptr)[1],
               ((int *) ptr)[2],
               {
                       (uint8_t) (((uint32_t *) ptr)[3] >> 24u),
                       (uint8_t) (((uint32_t *) ptr)[3] >> 16u),
                       (uint8_t) (((uint32_t *) ptr)[3] >> 8u),
                       (uint8_t) ((uint32_t *) ptr)[3]
               },
               ((float *) ptr)[5]),
        speed(((float *) ptr)[4]),
        size(((float *) ptr)[5]),
        waterAgility(((float *) ptr)[6]),
        rotation(((float *) ptr)[7]),
        energy(((int *) ptr)[8]),
        cooldown(((int *) ptr)[9]),
        energyLossWithMove(energyLossPerTick(true, ((float *) ptr)[4], ((float *) ptr)[5])),
        energyLossWithoutMove(energyLossPerTick(false, ((float *) ptr)[4], ((float *) ptr)[5])) {
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_LIVING_PARAMS;
    brain = new Brain(ptr);
}

RenderData LivingEntity::getRenderData() {
    return {World::getWorldDim(), radius, color, x, y, energy, true};
}

void LivingEntity::tick() {
    //################################# Breed ################################# at the beginning, so spawning happens before move ->on the right node
    int tempEnergy = this->energy;
    if (cooldown > 0) cooldown--;
    if (cooldown == 0 && tempEnergy >= 60 * energyLossWithMove) {
        //tempEnergy -= 60; leaving out might give better results
        uint8_t nr = color.r + (int) std::round(getRandomFloatBetween(0, 2.55));
        nr = nr < 0 ? 0 : (nr > 255 ? 255 : nr);
        uint8_t ng = color.g + (int) std::round(getRandomFloatBetween(0, 2.55));
        ng = ng < 0 ? 0 : (ng > 255 ? 255 : ng);
        uint8_t nb = color.b + (int) std::round(getRandomFloatBetween(0, 2.55));
        nb = nb < 0 ? 0 : (nb > 255 ? 255 : nb);

        // Create children
        auto child = new LivingEntity(x, y, {nr, ng, nb, 255}, speed + getRandomFloatBetween(0, 0.01),
                                      size + getRandomFloatBetween(0, 0.01),
                                      waterAgility + getRandomFloatBetween(0, 0.01), brain->createMutatedCopy());
        if (!World::addLivingEntity(child, false)) // Not added?
            delete child;

        cooldown += 60;
    }
    //################################# Think #################################
    FoodEntity *nearestFood = World::findNearestFood(x, y, false);
    NearestLiving nearest = World::findNearestLiving(this, false);


    Matrix continuousIn(6, 1, {
            (nearestFood ? nearestFood->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.enemy ? nearest.enemy->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.mate ? nearest.mate->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (float) tempEnergy / MAX_ENERGY,
            (nearest.mate ? (float) nearest.mate->energy / MAX_ENERGY * 0.8f : 1.f),
            nearest.enemy ? (float) nearest.enemy->size : 0.f
    });
    Matrix normalizedIn(4, 1, {
            (float) (nearestFood ? std::atan2(nearestFood->x - x, nearestFood->y - y) / PI : rotation),
            (float) (nearest.enemy ? std::atan2(nearest.enemy->x - x, nearest.enemy->y - y) / PI : rotation),
            (float) (nearest.mate ? std::atan2(nearest.mate->x - x, nearest.mate->y - y) / PI : rotation),
            *World::tileAt(x + (int) std::round(std::cos(rotation * PI) * TILE_SIZE),
                           y + (int) std::round(std::sin(rotation * PI) * TILE_SIZE)) == Tile::WATER ? -1.f : 1.f
    });
    //std::cout << continuousIn << normalizedIn << std::endl;
    ThinkResult thoughts = brain->think(continuousIn, normalizedIn);
    rotation = thoughts.rotation;
    //################################# Move ##################################
    if (thoughts.move) {
        float agility = *World::tileAt(x, y) == Tile::WATER ? waterAgility : 1.f - waterAgility;
        int xTo = x + (int) std::round(TILE_SIZE * speed * agility * 2 * std::cos(rotation * PI));
        int yTo = y + (int) std::round(TILE_SIZE * speed * agility * 2 * std::sin(rotation * PI));
        if (xTo < 0) xTo = 0;
        if (yTo < 0) yTo = 0;
        if (xTo >= World::overallWidth) xTo = World::overallWidth - 1;
        if (yTo >= World::overallHeight) yTo = World::overallHeight - 1;
        if(brain->printThink) std::cout << "xDif: " << (xTo - x) << " yDif: " << (yTo-y) << std::endl;
        if ((*World::tileAt(xTo, yTo) == Tile::WATER && waterAgility >= 0.2)
            || (*World::tileAt(xTo, yTo) != Tile::WATER && waterAgility < 0.8)) {
            x = (xTo + World::overallWidth) % World::overallWidth;
            y = (yTo + World::overallHeight) % World::overallHeight;
        }
    }
    //################################## Attack ##################################
    if (thoughts.attack && nearest.enemy && nearest.enemy->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (World::toRemoveLiving(nearest.enemy)) {
            LivingEntity *temp = World::findNearestLiving(this, true).enemy;
            nearest.enemy = (temp && temp->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) ? temp
                                                                                            : nullptr; //TODO synchronize from here
        }
        if (nearest.enemy) {
            if (size > nearest.enemy->size) {
                World::removeLivingEntity(nearest.enemy); //don't forget to synchronize
                tempEnergy += nearest.enemy->energy;
            } else {
                World::removeLivingEntity(this);
                return;
            }
        }
    }
    //################################## Share ##################################
    if (thoughts.share && tempEnergy > 80 && nearest.mate &&
        nearest.mate->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (World::toRemoveLiving(nearest.mate)) {
            LivingEntity *temp = World::findNearestLiving(this, true).mate;
            nearest.mate = (temp && temp->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) ? temp
                                                                                           : nullptr; //TODO synchronize from here
        }
        if (nearest.mate) {
            nearest.mate->addEnergy(60);
            tempEnergy -= 60;
        }
    }
    //################################## Eat ##################################
    if (nearestFood &&
        nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {//nearest food also needed for input
        if (World::toRemoveFood(nearestFood)) {
            FoodEntity *temp = World::findNearestFood(x, y, true);
            nearestFood = (temp && temp->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) ? temp
                                                                                           : nullptr; //TODO synchronize from here
        }
        if (nearestFood) {
            World::removeFoodEntity(nearestFood, false); //don't forget to synchronize
            tempEnergy += nearestFood->energy;
        }
    }

    //################################# Energy ################################
    tempEnergy -= thoughts.move ? energyLossWithMove : energyLossWithoutMove;
    assert(thoughts.move ? energyLossWithMove : energyLossWithoutMove > 0 && "Entity not losing Energy");
    if (tempEnergy <= 0) World::removeLivingEntity(this);
    if (tempEnergy > MAX_ENERGY) {
        this->energy = MAX_ENERGY;
    } else {
        this->energy = tempEnergy;
    }
    brain->printThink = false; //TODO remove
}

void LivingEntity::addEnergy(int e) {
    if(energy + e > MAX_ENERGY) {
        energy = MAX_ENERGY;
    } else {
        energy += e;
    }
}

/**
 * Writes the data to the given point in memory and sets the pointer to point to the next free byte after the written data
 * Only works on platforms with sizeof(int) = sizeof(float) = 32 bit
 * @param ptr Where to write the data. Use serializedSize() before, to determine the required space for allocation
 */
void LivingEntity::serialize(void *&ptr) {
    //continuous counting only works due to sizeof(int) = sizeof(float)
    ((int *) ptr)[0] = id;
    ((int *) ptr)[1] = x;
    ((int *) ptr)[2] = y;
    ((uint32_t *) ptr)[3] =
            ((uint32_t) color.r) << 24u | ((uint32_t) color.g) << 16u | ((uint32_t) color.b) << 8u |
            ((uint32_t) color.a);
    ((float *) ptr)[4] = speed;
    ((float *) ptr)[5] = size;
    ((float *) ptr)[6] = waterAgility;
    ((float *) ptr)[7] = rotation;
    ((int *) ptr)[8] = energy;
    ((int *) ptr)[9] = cooldown;
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_LIVING_PARAMS;
    brain->serialize(ptr);
}

std::ostream &operator<<(std::ostream &strm, const LivingEntity &l) {
    strm << "Entity:[id: " << l.id << ", x: " << l.x << ", y: " << l.y << ", color:{r: " << ((int) l.color.r) << ", g: "
         << ((int) l.color.g) << ", b: " << ((int) l.color.b) << "}, speed: " << l.speed << ", size: " << l.size
         << ", waterAgility: " << l.waterAgility << ", brainLayers: " << l.brain->getNumLayers() << "]";
    strm << *l.brain;
    return strm;
}


LivingEntity::~LivingEntity() {
    delete brain;
}
