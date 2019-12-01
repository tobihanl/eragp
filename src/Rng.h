#ifndef EVOLUTION_RNG_H
#define EVOLUTION_RNG_H

#include <random>

static std::mt19937 rng(1);

static int getRandomIntBetween(int from, int to) {
    std::uniform_int_distribution<int> distribution(from, to);
    return distribution(rng);
}

static float getRandomFloatBetween(float from, float to) {
    std::uniform_real_distribution<float> distribution(from, to);
    return distribution(rng);
}

#endif //EVOLUTION_RNG_H
