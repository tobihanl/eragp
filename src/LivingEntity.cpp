#include <cmath>
#include <cassert>
#include "LivingEntity.h"
#include "World.h"
#include "FoodEntity.h"

#define PI 3.14159265
#define MAX_ENERGY 9999
#define MAX_DRUNKNESS 400

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, Color c, float sp, float si, float wa, Brain *b, uint32_t seed) :
        Entity(startX < 0 ? 0 : (startX >= World::overallWidth ? World::overallWidth - 1 : startX),
                startY < 0 ? 0 : (startY >= World::overallHeight ? World::overallHeight - 1 : startY), c,
                (int) ((1.0f + si) * TILE_SIZE / 2), 60 * 2),
        speed(sp >= 0 ? (sp > 2 ? 2 : sp) : 0),
        size(si >= 0 ? (si > 2 ? 2 : si) : 0),
        waterAgility(wa < 0 ? 0 : (wa > 1 ? 1 : wa)),
        brain(b),
        cooldown(60),
        drunkness(0),
        rotation(0.0f),
        random(LFSR(seed)),
        energyLossBase(energyLossPerTick(si)) {
}

LivingEntity::LivingEntity(void *&ptr, bool minimal) : Entity(0, 0, 0, {}, 0.0f, 0) {
    id = ((int32_t *) ptr)[0];
    x = ((int32_t *) ptr)[1];
    y = ((int32_t *) ptr)[2];
    color = {
            (uint8_t) (((uint32_t *) ptr)[3] >> 24u),
            (uint8_t) (((uint32_t *) ptr)[3] >> 16u),
            (uint8_t) (((uint32_t *) ptr)[3] >> 8u),
            (uint8_t) ((uint32_t *) ptr)[3]
    };
    energy = ((int32_t *) ptr)[4];
    cooldown = ((int32_t *) ptr)[5];
    drunkness = ((int32_t *) ptr)[6];

    ptr = static_cast<uint32_t *>(ptr) + AMOUNT_32_BIT_LIVING_PARAMS;

    random = LFSR(((uint64_t *) ptr)[0]);

    ptr = static_cast<uint64_t *>(ptr) + AMOUNT_64_BIT_LIVING_PARAMS;

    speed = ((float *) ptr)[0];
    size = ((float *) ptr)[1];
    waterAgility = ((float *) ptr)[2];
    rotation = ((float *) ptr)[3];
    energyLossBase = energyLossPerTick(size);
    radius = (int) ((1.0f + size) * TILE_SIZE / 2);

    ptr = static_cast<float *>(ptr) + AMOUNT_FLOAT_LIVING_PARAMS;

    if (!minimal)
        brain = new Brain(ptr);
    else
        brain = nullptr;
}

RenderData LivingEntity::getRenderData() {
    return {World::getWorldDim(), radius, color, x, y, energy};
}

float LivingEntity::calculateDanger() {
    int min = x;
    if (y < min) min = y;
    if (World::overallWidth - x < min) min = World::overallWidth - x;
    if (World::overallHeight - y < min) min = World::overallHeight - y;
    return min > World::dangerZone ? -1.f : 1.f - ((float) min / (float) World::dangerZone);
}

// must be called before moving entity so spawning happens on right node
LivingEntity *LivingEntity::breed() {
    if (cooldown > 0) cooldown--;
    if (cooldown == 0 && (float) energy >= 60 * (energyLossBase + speed * 8)) {
        //tempEnergy -= 60; leaving out might give better results
        uint8_t nr = color.r + (int) std::round(random.getNextFloatBetween(0, 2.55));
        nr = nr < 0 ? 0 : (nr > 255 ? 255 : nr);
        uint8_t ng = color.g + (int) std::round(random.getNextFloatBetween(0, 2.55));
        ng = ng < 0 ? 0 : (ng > 255 ? 255 : ng);
        uint8_t nb = color.b + (int) std::round(random.getNextFloatBetween(0, 2.55));
        nb = nb < 0 ? 0 : (nb > 255 ? 255 : nb);

        cooldown += 60;
        // Create children
        return new LivingEntity(
                x + random.getNextIntBetween(-2 * TILE_SIZE, 2 * TILE_SIZE), y + random.getNextIntBetween(-2 * TILE_SIZE, 2 * TILE_SIZE), {nr, ng, nb, 255},
                speed + random.getNextFloatBetween(-0.05, 0.05),
                size + random.getNextFloatBetween(-0.05, 0.05),
                waterAgility + random.getNextFloatBetween(-0.05, 0.05), brain->createMutatedCopy(&random),
                random.getNextInt());
    }
    return nullptr;
}

void LivingEntity::think() {
    float agility = *World::tileAt(x, y) == Tile::WATER ? waterAgility : 1.f - waterAgility;
    nearestFood = World::findNearestFood(x, y, false);
    NearestLiving nearest = World::findNearestLiving(this, false);
    nearestEnemy = nearest.enemy;
    nearestMate = nearest.mate;

    Matrix input(1, 14, {
            (nearestFood ? nearestFood->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.enemy ? nearest.enemy->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (nearest.mate ? nearest.mate->getDistance(x, y) / VIEW_RANGE * 0.8f : 1.f),
            (float) energy / MAX_ENERGY,
            waterAgility * 2 - 1,
            speed * agility,//both between 0 and 1
            (nearest.mate ? (float) nearest.mate->energy / MAX_ENERGY * 0.8f : 1.f),
            nearest.enemy ? (float) nearest.enemy->size : 0.f,
            (float) (nearestFood ? std::atan2(nearestFood->y - y, nearestFood->x - x) / PI : rotation),
            (float) (nearest.enemy ? std::atan2(nearest.enemy->y - y, nearest.enemy->x - x) / PI : rotation),
            (float) (nearest.mate ? std::atan2(nearest.mate->y - y, nearest.mate->x - x) / PI : rotation),
            *World::tileAt(x + (int) std::round(std::cos(rotation * PI) * TILE_SIZE * speed * agility),
                           y + (int) (std::round(std::sin(rotation * PI) * TILE_SIZE) * speed * agility)) == Tile::WATER
            ? 1.f : -1.f,
            static_cast<float>(std::atan2((float) World::overallHeight / 2.f - (float) y,
                                          (float) World::overallWidth / 2.f - (float) x) / PI),
            calculateDanger()
    });
    thoughts = brain->think(input);

    //################################# Move ##################################
    thoughts.rotation = thoughts.rotation + (drunkness > 0 ? drunkness * (1. / MAX_DRUNKNESS) * std::sin((drunkness % 20) / 10.f * PI) : 0);
    if(drunkness > 0) drunkness--;
    int xTo = x + (int) std::round(TILE_SIZE * speed * thoughts.speed * agility * std::cos(thoughts.rotation * PI));
    int yTo = y + (int) std::round(TILE_SIZE * speed * thoughts.speed * agility * std::sin(thoughts.rotation * PI));

    if (xTo < 0 || yTo < 0 || xTo >= World::overallWidth || yTo >= World::overallHeight) {
        toBeRemoved = true;
        return;
    }
    if (brain->printThink) std::cout << "xDif: " << (xTo - x) << " yDif: " << (yTo - y) << std::endl;
    if ((*World::tileAt(xTo, yTo) == Tile::WATER && waterAgility >= 0.2)
        || (*World::tileAt(xTo, yTo) != Tile::WATER && waterAgility < 0.8)) {
        nextX = (xTo + World::overallWidth) % World::overallWidth;
        nextY = (yTo + World::overallHeight) % World::overallHeight;
    } else {
        nextX = x;
        nextY = y;
    }
}

void LivingEntity::updateMovement() {
    x = nextX;
    y = nextY;
    rotation = thoughts.rotation;
}

void LivingEntity::tick() {
    int tempEnergy = this->energy;

    //################################## Attack ##################################
    if (thoughts.attack && nearestEnemy && !nearestEnemy->toBeRemoved &&
        nearestEnemy->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (size > nearestEnemy->size) {
            nearestEnemy->toBeRemoved = true;
            tempEnergy += nearestEnemy->energy;
        } else {
            toBeRemoved = true;
            return;
        }
    }
    //################################## Share ##################################
    if (thoughts.share && tempEnergy > 80 && nearestMate && !nearestMate->toBeRemoved &&
        nearestMate->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        nearestMate->addEnergy(60);
        tempEnergy -= 60;
    }
    //################################## Eat ##################################
    if (nearestFood &&
        nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        if (World::toRemoveFood(nearestFood)) {
            FoodEntity *temp = World::findNearestFood(x, y, true);
            nearestFood = (temp && temp->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) ? temp
                                                                                           : nullptr;
        }
        if (nearestFood) {
            if(nearestFood->beer) drunkness = MAX_DRUNKNESS;
            tempEnergy += nearestFood->energy;
            World::removeFoodEntity(nearestFood, false); //don't forget to synchronize
        }
    }

    //################################# Energy ################################
    tempEnergy -= (int) round(energyLossBase + 8 * speed * thoughts.speed);
    assert(round(energyLossBase + 8 * speed * thoughts.speed) > 0 && "Entity not losing Energy");
    if (tempEnergy <= 0) toBeRemoved = true;
    if (tempEnergy > MAX_ENERGY) {
        this->energy = MAX_ENERGY;
    } else {
        this->energy = tempEnergy;
    }
    brain->printThink = false; //TODO remove
}

void LivingEntity::addEnergy(int e) {
    if (energy + e > MAX_ENERGY) {
        energy = MAX_ENERGY;
    } else {
        energy += e;
    }
}

/**
 * Writes the data to the given point in memory and sets the pointer to point to the next free byte after the written data
 * Only works on platforms with sizeof(int) = sizeof(float) = 32 bit
 * @param ptr Where to write the data. Use fullSerializedSize() before, to determine the required space for allocation
 */
void LivingEntity::fullSerialize(void *&ptr) {
    minimalSerialize(ptr);

    if (brain != nullptr)
        brain->serialize(ptr);
}

void LivingEntity::minimalSerialize(void *&ptr) {
    ((int32_t *) ptr)[0] = (int32_t) id;
    ((int32_t *) ptr)[1] = (int32_t) x;
    ((int32_t *) ptr)[2] = (uint32_t) y;
    ((uint32_t *) ptr)[3] =
            ((uint32_t) color.r) << 24u | ((uint32_t) color.g) << 16u |
            ((uint32_t) color.b) << 8u | ((uint32_t) color.a);
    ((int32_t *) ptr)[4] = (int32_t) energy;
    ((int32_t *) ptr)[5] = (int32_t) cooldown;
    ((int32_t *) ptr)[6] = (int32_t) drunkness;


    ptr = static_cast<uint32_t *>(ptr) + AMOUNT_32_BIT_LIVING_PARAMS;

    ((uint64_t *) ptr)[0] = random.getLfsrRegister();

    ptr = static_cast<uint64_t *>(ptr) + AMOUNT_64_BIT_LIVING_PARAMS;

    ((float *) ptr)[0] = speed;
    ((float *) ptr)[1] = size;
    ((float *) ptr)[2] = waterAgility;
    ((float *) ptr)[3] = rotation;

    ptr = static_cast<float *>(ptr) + AMOUNT_FLOAT_LIVING_PARAMS;
}

std::ostream &operator<<(std::ostream &strm, const LivingEntity &l) {
    int brainLayers = (l.brain != nullptr) ? l.brain->getNumLayers() : -1;
    strm << "Entity:[id: " << l.id << ", x: " << l.x << ", y: " << l.y << ", color:{r: " << ((int) l.color.r) << ", g: "
         << ((int) l.color.g) << ", b: " << ((int) l.color.b) << "}, speed: " << l.speed << ", size: " << l.size
         << ", waterAgility: " << l.waterAgility << ", brainLayers: " << brainLayers << "]";
    if (l.brain != nullptr) strm << *l.brain;
    return strm;
}


LivingEntity::~LivingEntity() {
    delete brain;
}
