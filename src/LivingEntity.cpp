#include <cmath>
#include <cassert>
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"

#define PI 3.14159265
#define BRAIN_NOT_FOUND 1000 //TODO search better dummy value
#define AMOUNT_OF_PARAMS 10

static std::mt19937 createGenerator() {
    std::random_device rd;
    std::mt19937 gen(rd());
    return gen;
}

std::mt19937 LivingEntity::randomGenerator = createGenerator();
std::normal_distribution<float> LivingEntity::normalDistribution(0, 0.01);

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
        rotation(0.0f) {

}

LivingEntity::LivingEntity(void *&ptr) :
        Entity(((int *) ptr)[0],
               ((int *) ptr)[1],
               ((int *) ptr)[2],
               {
                       (Uint8) (((int *) ptr)[3] >> 24),
                       (Uint8) (((int *) ptr)[3] >> 16),
                       (Uint8) (((int *) ptr)[3] >> 8),
                       (Uint8) ((int *) ptr)[3]
               },
               ((float *) ptr)[5]),
        color({
                      (Uint8) (((int *) ptr)[3] >> 24),
                      (Uint8) (((int *) ptr)[3] >> 16),
                      (Uint8) (((int *) ptr)[3] >> 8),
                      (Uint8) ((int *) ptr)[3]
              }),
        speed(((float *) ptr)[4]),
        size(((float *) ptr)[5]),
        waterAgility(((float *) ptr)[6]),
        rotation(((float *) ptr)[7]),
        energy(((int *) ptr)[8]),
        cooldown(((int *) ptr)[9]) {
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
    int radiusI = round(radius);
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
    if (cooldown == 0 && energy >= 60 * 2) {
        //energy -= 60; leaving out might give better results
        Uint8 nr = color.r + std::round(normalDistribution(randomGenerator) * 255);
        nr = nr < 0 ? 0 : (nr > 255 ? 255 : nr);
        Uint8 ng = color.g + std::round(normalDistribution(randomGenerator) * 255);
        ng = ng < 0 ? 0 : (ng > 255 ? 255 : ng);
        Uint8 nb = color.b + std::round(normalDistribution(randomGenerator) * 255);
        nb = nb < 0 ? 0 : (nb > 255 ? 255 : nb);
        World::addLivingEntity(new LivingEntity(x, y, {nr, ng, nb, 255}, speed + normalDistribution(randomGenerator),
                                                size + normalDistribution(randomGenerator),
                                                waterAgility + normalDistribution(randomGenerator),
                                                brain->createMutatedCopy()));
        cooldown += 60;
    }
    //################################# Think #################################
    FoodEntity *nearestFood = World::findNearestFood(x, y);
    LivingEntity *nearestEnemy = World::findNearestEnemy(this);
    LivingEntity *nearestMate = World::findNearestMate(this);

    Matrix continuousIn(6, 1, {
            (float) (nearestFood ? nearestFood->getDistance(x, y) : BRAIN_NOT_FOUND),
            (float) (nearestEnemy ? nearestEnemy->getDistance(x, y) : BRAIN_NOT_FOUND),
            (float) (nearestMate ? nearestMate->getDistance(x, y) : BRAIN_NOT_FOUND),
            (float) energy, (float) (nearestMate ? nearestMate->energy : BRAIN_NOT_FOUND),
            nearestEnemy ? (float) nearestEnemy->size * 500 : 0.f
    });
    Matrix normalizedIn(4, 1, {
            (float) (nearestFood ? std::atan2(nearestFood->x - x, nearestFood->y - y) / PI : rotation),
            (float) (nearestEnemy ? std::atan2(nearestEnemy->x - x, nearestEnemy->y - y) / PI : rotation),
            (float) (nearestMate ? std::atan2(nearestMate->x - x, nearestMate->y - y) / PI : rotation),
            *World::tileAt(x + std::round(std::cos(rotation * PI) * TILE_SIZE),
                           y + std::round(std::sin(rotation * PI) * TILE_SIZE)) == Tile::WATER ? -1.f : 1.f
    });
    //std::cout << continuousIn << normalizedIn << std::endl;
    ThinkResult thoughts = brain->think(continuousIn, normalizedIn);
    rotation = thoughts.rotation;
    //################################# Move ##################################
    if (thoughts.move) {
        float agility = *World::tileAt(x, y) == Tile::WATER ? waterAgility : 1.f - waterAgility;
        int xTo = x + (int) std::round(TILE_SIZE * speed * agility * 2 * std::cos(rotation * PI));
        int yTo = y + (int) std::round(TILE_SIZE * speed * agility * 2 * std::sin(rotation * PI));
        /*if (*World::tileAt(xTo, yTo) == Tile::WATER && waterAgility >= 0.2 TODO reenable
            || *World::tileAt(xTo, yTo) != Tile::WATER && waterAgility < 0.8) {
            x = (xTo + World::overallWidth) % World::overallWidth;
            y = (yTo + World::overallHeight) % World::overallHeight;
        }*/
        x = (xTo + World::overallWidth) % World::overallWidth; //TODO could cause overflow for large worlds. Use long instead?
        y = (yTo + World::overallHeight) % World::overallHeight;
    }
    //################################## Eat ##################################
    if (nearestFood && nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (!World::toRemoveFood(nearestFood)) {
            World::removeFoodEntity(nearestFood); //TODO don't forget to synchronize
            energy += nearestFood->energy;
        } else {
            nearestFood = World::findNearestSurvivingFood(x, y);
            if (nearestFood && nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
                World::removeFoodEntity(nearestFood); //TODO don't forget to synchronize
                energy += nearestFood->energy;
            }
        }
    }
    //################################# Energy ################################
    energy -= (thoughts.move ? speed * 8 : 0) + size * 4 + 1;
    assert((((int) (thoughts.move ? speed * 8 : 0) + size * 4 + 1)) > 0 && "Entity not loosing Energy");
    if (energy <= 0) World::removeLivingEntity(this);
}

//TODO consider new properties when added
float LivingEntity::difference(const LivingEntity &e) {
    return std::sqrt(((e.color.r - color.r) / 255.f) * ((e.color.r - color.r) / 255.f)
                     + ((e.color.g - color.g) / 255.f) * ((e.color.g - color.g) / 255.f)
                     + ((e.color.b - color.b) / 255.f) * ((e.color.b - color.b) / 255.f)
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
    ((int *) ptr)[3] = ((int) color.r << 24) | ((int) color.g << 16) | ((int) color.b << 8) | color.a;
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


