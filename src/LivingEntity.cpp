
#include <cmath>
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"
#include <cmath>
#include <cassert>

static std::mt19937 createGenerator() {
    std::random_device rd;
    std::mt19937 gen(rd());
    return gen;
}

std::mt19937 LivingEntity::randomGenerator = createGenerator();
std::normal_distribution<float> LivingEntity::normalDistribution(0, 0.1);

SDL_Texture *LivingEntity::digits[10];

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float sp, float si, Brain *b) : Entity(startX, startY,
                                                                                                       c, (int) ((1.0f +
                                                                                                                  si) *
                                                                                                                 TILE_SIZE /
                                                                                                                 2)),
                                                                                                color(c),
                                                                                                speed(sp >= 0 ? sp : 0),
                                                                                                size(si >= 0 ? si : 0),
                                                                                                brain(b),
                                                                                                energy(60 * 2),
                                                                                                cooldown(60) {

}


LivingEntity::LivingEntity(void *&ptr) : Entity(((int *) ptr)[0], ((int *) ptr)[1], ((int *) ptr)[2],
                                                {(Uint8)(((int *) ptr)[3] >> 24), (Uint8)(((int *) ptr)[3] >> 16),
                                                 (Uint8)(((int *) ptr)[3] >> 8), (Uint8)((int *) ptr)[3]},
                                                ((float *) ptr)[5]), speed(((float *) ptr)[4]),
                                         size(((float *) ptr)[5]) {
    color = {(Uint8)(((int *) ptr)[3] >> 24), (Uint8)(((int *) ptr)[3] >> 16), (Uint8)(((int *) ptr)[3] >> 8),
             (Uint8)((int *) ptr)[3]};
    ptr = static_cast<int *>(ptr) + 6;
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
    float radius = (1.0f + size) * TILE_SIZE / 2.0f;
    int radiusI = round(radius);
    Renderer::copy(texture, x - radiusI, y - radiusI);
    if (energy <= 0) {
        Renderer::copy(digits[0], x - (ENERGY_FONT_SIZE / 2), y - 4 - ENERGY_FONT_SIZE);
    } else {//max width/height ratio for char is 0,7 | 12 * 0,7 = 8,4 -> width := 8
        int numDigits = getNumDigits(energy);
        int energyToDisplay = energy;
        int baseX = x + numDigits * 4 -
                    4; //9 / 2 = 4.5 AND: go half a char to the lft because rendering starts in the left corner
        for (int i = 0; energyToDisplay > 0; i++) {
            Renderer::copy(digits[energyToDisplay % 10], baseX - 8 * i, y - 4 - ENERGY_FONT_SIZE);
            energyToDisplay /= 10;
        }
    }
    //Renderer::renderDot(x, y, (1 + size) * TILE_SIZE / 2, color);
}

void LivingEntity::tick() {
    //################################# Think #################################
    FoodEntity *nearestFood = World::findNearestFood(x, y);
    Matrix thoughts(3, 1,
                    {(float) (!nearestFood ? x : nearestFood->x - x), (float) (!nearestFood ? y : nearestFood->y - y),
                     (float) energy});
    thoughts = brain->think(thoughts);
    bool brainMove = thoughts(0, 0) > -1000;
    WorldDim dim = World::getWorldDim();
    //################################# Move ##################################
    if (brainMove) {//evaluate whether to move TODO change bias after applying norm function
        float brainXDif = thoughts(1, 0);
        float brainYDif = thoughts(2, 0);
        float factor = TILE_SIZE * speed / std::sqrt(brainXDif * brainXDif + brainYDif * brainYDif);
        x += (int) std::round(factor * brainXDif);
        y += (int) std::round(factor * brainYDif);

        // check if entity needs to be moved to a neighbor node
        if (x >= dim.w) {
            if (y < 0) World::moveToNeighbor(this, 1);
            else if (y >= dim.h) World::moveToNeighbor(this, 3);
            else World::moveToNeighbor(this, 2);
        } else if (x < 0) {
            if (y < 0) World::moveToNeighbor(this, 7);
            else if (y >= dim.h) World::moveToNeighbor(this, 5);
            else World::moveToNeighbor(this, 6);
        } else {
            if (y < 0) World::moveToNeighbor(this, 0);
            else if (y >= dim.h) World::moveToNeighbor(this, 4);
        }

        // calculate position on new node, might have to be done on new node if dimensions differ
        if (x >= dim.w) x -= dim.w;
        else if (x < 0) x = dim.w - x;

        if (y >= dim.h) y -= dim.h;
        else if (y < 0) y = dim.h - y;
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
    energy -= (brainMove ? speed * 8 : 0) + size * 4 + 1;
    assert((((int) (brainMove ? speed * 8 : 0) + size * 4 + 1)) > 0 && "Entity not loosing Energy");
    if (energy <= 0) World::removeLivingEntity(this);

    //################################# Breed #################################
    if (cooldown > 0) cooldown--;
    if (cooldown == 0 && energy >= 60 * 2) {
        energy -= 60;
        World::addLivingEntity(new LivingEntity(x, y, color, speed + normalDistribution(randomGenerator),
                                                size + normalDistribution(randomGenerator),
                                                brain->createMutatedCopy()));
        cooldown += 60;
    }
}

int LivingEntity::serializedSize() {
    return 6 * 4 + brain->serializedSized();
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
    ptr = static_cast<int *>(ptr) + 6;
    brain->serialize(ptr);
}

std::ostream &operator<<(std::ostream &strm, const LivingEntity &l) {
    strm << "Entity:[id: " << l.id << ", x: " << l.x << ", y: " << l.y << ", color:{r: " << ((int) l.color.r) << ", g: "
         << ((int) l.color.g) << ", b: " << ((int) l.color.b) << "}, speed: " << l.speed << ", size: " << l.size <<
         ", energy: " << l.energy
         << ", brainLayers: " << l.brain->getNumLayers() << "]";
    return strm;
}

LivingEntity::~LivingEntity() {
    delete brain;
}


