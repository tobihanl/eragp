
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

SDL_Texture* LivingEntity::digits[10];

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float sp, float si, Brain* b) : Entity(startX, startY, c, (int) ((1.0f + si) * TILE_SIZE / 2)), color(c), speed(sp), size(si), brain(b), energy(60 * 2), cooldown(60) {

}

static int getNumDigits(int x) {
    if(x < 10) return 1;
    else if(x < 100) return 2;
    else if(x < 1000) return 3;
    else if(x < 10000) return 4;
    else if(x < 100000) return 5;
    assert(false && "Too much energy");
}

void LivingEntity::render() {
    float radius = (1.0f + size) * TILE_SIZE / 2.0f;
    int radiusI = round(radius);
    Renderer::copy(texture, x - radiusI, y - radiusI);
    if(energy <= 0) {
        Renderer::copy(digits[0], x - (ENERGY_FONT_SIZE / 2), y - 4 - ENERGY_FONT_SIZE);
    } else {//max width/height ratio for char is 0,7 | 12 * 0,7 = 8,4 -> width := 8
        int numDigits = getNumDigits(energy);
        int energyToDisplay = energy;
        int baseX = x + numDigits * 4 - 4; //9 / 2 = 4.5 AND: go half a char to the lft because rendering starts in the left corner
        for(int i = 0; energyToDisplay > 0; i++) {
            Renderer::copy(digits[energyToDisplay % 10], baseX - 8 * i, y - 4 - ENERGY_FONT_SIZE);
            energyToDisplay /= 10;
        }
    }
    //Renderer::renderDot(x, y, (1 + size) * TILE_SIZE / 2, color);
}

void LivingEntity::tick() {
    //################################# Think #################################
    FoodEntity* nearestFood = World::findNearestFood(x, y);
    Matrix thoughts(3, 1, {(float)(!nearestFood ? x : nearestFood->x - x), (float)(!nearestFood ? y : nearestFood->y - y), (float)energy});
    thoughts = brain->think(thoughts);
    bool brainMove = thoughts(0, 0) > -1000;
    //################################# Move ##################################
    if(brainMove) {//evaluate whether to move TODO change bias after applying norm function
        float brainXDif = thoughts(1, 0);
        float brainYDif = thoughts(2, 0);
        float factor = TILE_SIZE * speed / std::sqrt(brainXDif * brainXDif + brainYDif * brainYDif);
        x += (int) std::round(factor * brainXDif);
        if(x < 0) x = 0;
        else if(x >= WORLD_WIDTH) x = WORLD_WIDTH - 1;
        y += (int) std::round(factor * brainYDif);
        if(y < 0) y = 0;
        else if(y >= WORLD_HEIGHT) y = WORLD_HEIGHT - 1;
    }
    //################################## Eat ##################################
    if(nearestFood && nearestFood->getSquaredDistance(x, y) < TILE_SIZE * TILE_SIZE) {
        energy += nearestFood->energy; // TODO: [BUG] Multiple LivingEntities can eat the same food!
        World::removeFoodEntity(nearestFood);
    }
    //################################# Breed #################################
    if(cooldown > 0) cooldown--;
    if(cooldown == 0 && energy >= 60 * 2) {
        World::addLivingEntity(new LivingEntity(x, y, color, speed + normalDistribution(randomGenerator), size + normalDistribution(randomGenerator), brain->createMutatedCopy()));
        cooldown += 60;
    }
    //################################# Energy ################################
    energy -= (brainMove ? speed * 8 : 0) + size * 4 + 0.5;
    if(energy <= 0) World::removeLivingEntity(this);
}

LivingEntity::~LivingEntity() {
    delete brain;
}