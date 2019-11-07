#include <cmath>
#include <cassert>
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"
#include "Rng.h"

#define PI 3.14159265
#define AMOUNT_OF_PARAMS 10

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
        energyLossWithMove(energyLossPerTick(true, sp, si)),
        energyLossWithoutMove(energyLossPerTick(false, sp, si)) {

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
        energyLossWithMove(energyLossPerTick(true, ((float *) ptr)[4], ((float *) ptr)[5])),
        energyLossWithoutMove(energyLossPerTick(false, ((float *) ptr)[4], ((float *) ptr)[5])) {
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_PARAMS;
    brain = new Brain(ptr);
}

static int getNumDigits(int x) {
    if (x < 10) return 1;
    else if (x < 100) return 2;
    else if (x < 1000) return 3;
    else if (x < 10000) return 4;
    else if (x < 100000) return 5;
    assert(false && "Too much energy");
}

void LivingEntity::render() {
    WorldDim dim = World::getWorldDim();
    float radius = (1.0f + size) * TILE_SIZE / 2.0f;
    int radiusI = (int) round(radius);
    Renderer::copy(texture, x - dim.x - radiusI, y - dim.y - radiusI);
    if (energy <= 0) {
        Renderer::copy(digits[0], x - dim.x - (ENERGY_FONT_SIZE / 2), y - dim.y - 4 - ENERGY_FONT_SIZE);
    } else {//max width/height ratio for char is 0,7 | 12 * 0,7 = 8,4 -> width := 8
        int numDigits = getNumDigits(energy);
        int energyToDisplay = energy;
        int baseX = x - dim.x + numDigits * 4 -
                    4; //9 / 2 = 4.5 AND: go half a char to the lft because rendering starts in the left corner
        for (int i = 0; energyToDisplay > 0; i++) {
            Renderer::copy(digits[energyToDisplay % 10], baseX - 8 * i, y - dim.y - 4 - ENERGY_FONT_SIZE);
            energyToDisplay /= 10;
        }
    }
    //Renderer::renderDot(x, y, (1 + size) * TILE_SIZE / 2, color);
}

void LivingEntity::tick() {
    //################################# Breed ################################# at the beginning, so spawning happens before move ->on the right node
    if (cooldown > 0) cooldown--;
    if (cooldown == 0 && energy >= 60 * energyLossWithMove) {
        //energy -= 60; leaving out might give better results
        Uint8 nr = color.r + (int) std::round(getRandomFloatBetween(0, 2.55));
        nr = nr < 0 ? 0 : (nr > 255 ? 255 : nr);
        Uint8 ng = color.g + (int) std::round(getRandomFloatBetween(0, 2.55));
        ng = ng < 0 ? 0 : (ng > 255 ? 255 : ng);
        Uint8 nb = color.b + (int) std::round(getRandomFloatBetween(0, 2.55));
        nb = nb < 0 ? 0 : (nb > 255 ? 255 : nb);
        World::addLivingEntity(new LivingEntity(x, y, {nr, ng, nb, 255}, speed + getRandomFloatBetween(0, 0.01),
                                                size + getRandomFloatBetween(0, 0.01),
                                                waterAgility + getRandomFloatBetween(0, 0.01),
                                                brain->createMutatedCopy()), false);
        cooldown += 60;
    }
    //################################# Think #################################
    FoodEntity *nearestFood = World::findNearestFood(x, y);
    LivingEntity *nearestEnemy = World::findNearestEnemy(this);
    LivingEntity *nearestMate = World::findNearestMate(this);

    Matrix continuousIn(6, 1, {
            (float) (nearestFood ? nearestFood->getDistance(x, y) : VIEW_RANGE * 2),
            (float) (nearestEnemy ? nearestEnemy->getDistance(x, y) : VIEW_RANGE * 2),
            (float) (nearestMate ? nearestMate->getDistance(x, y) : VIEW_RANGE * 2),
            (float) energy, (float) (nearestMate ? nearestMate->energy : VIEW_RANGE * 2),
            nearestEnemy ? (float) nearestEnemy->size * 500 : 0.f
    });
    Matrix normalizedIn(4, 1, {
            (float) (nearestFood ? std::atan2(nearestFood->x - x, nearestFood->y - y) / PI : rotation),
            (float) (nearestEnemy ? std::atan2(nearestEnemy->x - x, nearestEnemy->y - y) / PI : rotation),
            (float) (nearestMate ? std::atan2(nearestMate->x - x, nearestMate->y - y) / PI : rotation),
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
        if ((*World::tileAt(xTo, yTo) == Tile::WATER && waterAgility >= 0.2)
            || (*World::tileAt(xTo, yTo) != Tile::WATER && waterAgility < 0.8)) {
            x = (xTo + World::overallWidth) % World::overallWidth;
            y = (yTo + World::overallHeight) % World::overallHeight;
        }
    }
    //################################## Eat ##################################
    if (nearestFood && nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (!World::toRemoveFood(nearestFood)) {
            World::removeFoodEntity(nearestFood, false); //TODO don't forget to synchronize
            energy += nearestFood->energy;
        } else {
            nearestFood = World::findNearestSurvivingFood(x, y);
            if (nearestFood && nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
                World::removeFoodEntity(nearestFood, false); //TODO don't forget to synchronize
                energy += nearestFood->energy;
            }
        }
    }
    //################################# Energy ################################
    energy -= thoughts.move ? energyLossWithMove : energyLossWithoutMove;
    assert(thoughts.move ? energyLossWithMove : energyLossWithoutMove > 0 && "Entity not losing Energy");
    if (energy <= 0) World::removeLivingEntity(this);
}

int LivingEntity::energyLossPerTick(bool move, float speed, float size) {
    return (int) round((move ? speed * 8 : 0) + size * 4 + 1);
}

//TODO consider new properties when added
float LivingEntity::difference(const LivingEntity &e) {
    return std::sqrt(((float) (e.color.r - color.r) / 255.f) * ((float) (e.color.r - color.r) / 255.f)
                     + ((float) (e.color.g - color.g) / 255.f) * ((float) (e.color.g - color.g) / 255.f)
                     + ((float) (e.color.b - color.b) / 255.f) * ((float) (e.color.b - color.b) / 255.f)
                     + (e.speed - speed) * (e.speed - speed)
                     + (e.size - size) * (e.size - size)
                     + (e.waterAgility - waterAgility) * (e.waterAgility - waterAgility));//TODO consider brain
}

int LivingEntity::serializedSize() {
    return AMOUNT_OF_PARAMS * 4 + brain->serializedSized();
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
    ptr = static_cast<int *>(ptr) + AMOUNT_OF_PARAMS;
    brain->serialize(ptr);
}

std::ostream &operator<<(std::ostream &strm, const LivingEntity &l) {
    strm << "Entity:[id: " << l.id << ", x: " << l.x << ", y: " << l.y << ", color:{r: " << ((int) l.color.r) << ", g: "
         << ((int) l.color.g) << ", b: " << ((int) l.color.b) << "}, speed: " << l.speed << ", size: " << l.size
         << ", waterAgility: " << l.waterAgility << ", brainLayers: " << l.brain->getNumLayers() << "]";
    return strm;
}


LivingEntity::~LivingEntity() {
    delete brain;
}


