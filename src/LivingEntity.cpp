
#include "LivingEntity.h"
#include "Renderer.h"
#include "World.h"
#include "FoodEntity.h"
#include <cmath>
#include <math.h>

static std::mt19937 createGenerator() {
    std::random_device rd;
    std::mt19937 gen(rd());
    return gen;
}

std::mt19937 LivingEntity::randomGenerator = createGenerator();
std::normal_distribution<float> LivingEntity::normalDistribution(0, 0.1);

SDL_Texture* LivingEntity::digits[10] = {
        Renderer::renderFont("0", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("1", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("2", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("3", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("4", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("5", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("6", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("7", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("8", 10, {255, 255, 255, 0}, "font.ttf"),
        Renderer::renderFont("9", 10, {255, 255, 255, 0}, "font.ttf")};

//################################Begin object##############################################

LivingEntity::LivingEntity(int startX, int startY, SDL_Color c, float sp, float si, Brain* b) : Entity(startX, startY, c, (int) ((1.0f + si) * TILE_SIZE / 2)), color(c), speed(sp), size(si), brain(b), energy(60 * 2), cooldown(60) {

}

void LivingEntity::render() {
    if(energy <= 0) {
        Renderer::copy(digits[0], x, y);
    } else {
        int energyToDisplay = energy;
        for(int i = 0; energyToDisplay > 0; i++) {
            Renderer::copy(digits[energyToDisplay % 10], x + (i << 3), (int) (y + ((1.0f + size) * TILE_SIZE * 3 / 4)));
            energyToDisplay /= 10;
        }
    }
    Renderer::copy(texture, x + (int)((1.0f + size) * TILE_SIZE / 2), y + (int)((1.0f + size) * TILE_SIZE / 2));
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
        energy += nearestFood->energy;
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