#include <cmath>
#include <cassert>
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"
#include "Rng.h"

#define PI 3.14159265
#define MAX_ENERGY 9999

SDL_Texture *LivingEntity::digits[10];

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float sp, float si, float wa, Brain *b) :
        Entity(startX, startY, c, (int) ((1.0f + si) * TILE_SIZE / 2)),
        color(c),
        speed(sp >= 0 ? sp : 0),
        size(si >= 0 ? si : 0),
        waterAgility(wa < 0 ? 0 : (wa > 1 ? 1 : wa)),
        brain(b),
        energy(60 * 2),
        cooldown(60),
        rotation(0.0f),
        energyLossBase(energyLossPerTick(si)) {

}

LivingEntity::LivingEntity(void *&ptr) :
        Entity(((int *) ptr)[0],
               ((int *) ptr)[1],
               ((int *) ptr)[2],
               {
                       (Uint8) (((Uint32 *) ptr)[3] >> 24u),
                       (Uint8) (((Uint32 *) ptr)[3] >> 16u),
                       (Uint8) (((Uint32 *) ptr)[3] >> 8u),
                       (Uint8) ((Uint32 *) ptr)[3]
               },
               ((float *) ptr)[5]),
        color({
                      (Uint8) (((Uint32 *) ptr)[3] >> 24u),
                      (Uint8) (((Uint32 *) ptr)[3] >> 16u),
                      (Uint8) (((Uint32 *) ptr)[3] >> 8u),
                      (Uint8) ((Uint32 *) ptr)[3]
              }),
        speed(((float *) ptr)[4]),
        size(((float *) ptr)[5]),
        waterAgility(((float *) ptr)[6]),
        rotation(((float *) ptr)[7]),
        energy(((int *) ptr)[8]),
        cooldown(((int *) ptr)[9]),
        energyLossBase(energyLossPerTick(((float *) ptr)[5])) {
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_LIVING_PARAMS;
    brain = new Brain(ptr);
}

static int getNumDigits(int x) {
    if (x < 10) return 1;
    else if (x < 100) return 2;
    else if (x < 1000) return 3;
    else if (x < 10000) return 4;
    assert(false && "Too much energy");
}

void LivingEntity::render() {
    WorldDim dim = World::getWorldDim();
    float radius = (1.0f + size) * TILE_SIZE / 2.0f;
    int radiusI = (int) round(radius);
    Renderer::copy(texture, x - dim.p.x - radiusI, y - dim.p.y - radiusI);
    if (energy <= 0) {
        Renderer::copy(digits[0], x - dim.p.x - (ENERGY_FONT_SIZE / 2), y - dim.p.y - 4 - ENERGY_FONT_SIZE);
    } else {//max width/height ratio for char is 0,7 | 12 * 0,7 = 8,4 -> width := 8
        int numDigits = getNumDigits(energy);
        int energyToDisplay = energy;
        int baseX = x - dim.p.x + numDigits * 4 -
                    4; //9 / 2 = 4.5 AND: go half a char to the lft because rendering starts in the left corner
        for (int i = 0; energyToDisplay > 0; i++) {
            Renderer::copy(digits[energyToDisplay % 10], baseX - 8 * i, y - dim.p.y - 4 - ENERGY_FONT_SIZE);
            energyToDisplay /= 10;
        }
    }
    //Renderer::renderDot(x, y, (1 + size) * TILE_SIZE / 2, color);
}

float LivingEntity::calculateDanger() {
    int min = x;
    if(y < min) min = y;
    if(World::overallWidth - x < min) min = World::overallWidth - x;
    if(World::overallHeight - y < min) min = World::overallHeight - y;
    return min > World::dangerZone ? -1.f : 1.f - (min / (float) World::dangerZone);
}

void LivingEntity::tick() {
    //################################# Breed ################################# at the beginning, so spawning happens before move ->on the right node
    int tempEnergy = this->energy;
    if (cooldown > 0) cooldown--;
    if (cooldown == 0 && tempEnergy >= 60 * (energyLossBase + speed * 8)) {
        //tempEnergy -= 60; leaving out might give better results
        Uint8 nr = color.r + (int) std::round(getRandomFloatBetween(0, 2.55));
        nr = nr < 0 ? 0 : (nr > 255 ? 255 : nr);
        Uint8 ng = color.g + (int) std::round(getRandomFloatBetween(0, 2.55));
        ng = ng < 0 ? 0 : (ng > 255 ? 255 : ng);
        Uint8 nb = color.b + (int) std::round(getRandomFloatBetween(0, 2.55));
        nb = nb < 0 ? 0 : (nb > 255 ? 255 : nb);

        // Create children
        auto child = new LivingEntity(x, y, {nr, ng, nb, 255}, speed + getRandomFloatBetween(-0.05, 0.05),
                                      size + getRandomFloatBetween(-0.05, 0.05),
                                      waterAgility + getRandomFloatBetween(-0.05, 0.05), brain->createMutatedCopy());
        if (!World::addLivingEntity(child, false)) // Not added?
            delete child;

        cooldown += 60;
    }
    //################################# Think #################################
    FoodEntity *nearestFood = World::findNearestFood(x, y, false);
    NearestLiving nearest = World::findNearestLiving(this, false);


    Matrix input(12, 1, {
            (nearestFood ? nearestFood->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.enemy ? nearest.enemy->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.mate ? nearest.mate->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (float) tempEnergy / MAX_ENERGY,
            (nearest.mate ? (float) nearest.mate->energy / MAX_ENERGY * 0.8f : 1.f),
            nearest.enemy ? (float) nearest.enemy->size : 0.f,
            (float) (nearestFood ? std::atan2(nearestFood->y - y, nearestFood->x - x) / PI : rotation),
            (float) (nearest.enemy ? std::atan2(nearest.enemy->y - y, nearest.enemy->x - x) / PI : rotation),
            (float) (nearest.mate ? std::atan2(nearest.mate->y - y, nearest.mate->x - x) / PI : rotation),
            *World::tileAt(x + (int) std::round(std::cos(rotation * PI) * TILE_SIZE),
                           y + (int) std::round(std::sin(rotation * PI) * TILE_SIZE)) == Tile::WATER ? -1.f : 1.f,
            std::atan2(World::overallHeight / 2.f - y, World::overallWidth / 2.f - x),
            calculateDanger()
    });
    ThinkResult thoughts = brain->think(input);
    rotation = thoughts.rotation;
    //################################# Move ##################################
    float agility = *World::tileAt(x, y) == Tile::WATER ? waterAgility : 1.f - waterAgility;
    int xTo = x + (int) std::round(TILE_SIZE * speed * thoughts.speed * agility * 2 * std::cos(rotation * PI));
    int yTo = y + (int) std::round(TILE_SIZE * speed * thoughts.speed * agility * 2 * std::sin(rotation * PI));
    if (xTo < 0 || yTo < 0 || xTo >= World::overallWidth || yTo >= World::overallHeight) {
        World::removeLivingEntity(this);
        return;
    }
    if(brain->printThink) std::cout << "xDif: " << (xTo - x) << " yDif: " << (yTo-y) << std::endl;
    if ((*World::tileAt(xTo, yTo) == Tile::WATER && waterAgility >= 0.2)
        || (*World::tileAt(xTo, yTo) != Tile::WATER && waterAgility < 0.8)) {
        x = (xTo + World::overallWidth) % World::overallWidth;
        y = (yTo + World::overallHeight) % World::overallHeight;
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
    tempEnergy -= round(energyLossBase + 8 * speed * thoughts.speed);
    assert(round(energyLossBase + 8 * speed * thoughts.speed) > 0 && "Entity not losing Energy");
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
    ((Uint32 *) ptr)[3] =
            ((Uint32) color.r) << 24u | ((Uint32) color.g) << 16u | ((Uint32) color.b) << 8u | ((Uint32) color.a);
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


