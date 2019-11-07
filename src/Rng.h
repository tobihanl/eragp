#ifndef EVOLUTION_RNG_H
#define EVOLUTION_RNG_H

#include <random>

extern std::mt19937 rng;

extern int getRandomIntBetween(int from, int to);

extern float getRandomFloatBetween(float from, float to);

#endif //EVOLUTION_RNG_H
