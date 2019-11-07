#include "Rng.h"

std::mt19937 rng(1);

int getRandomIntBetween(int from, int to) {
    std::uniform_int_distribution<int> distribution(from, to);
    return distribution(rng);
}

float getRandomFloatBetween(float from, float to) {
    std::normal_distribution<float> distribution(from, to);
    return distribution(rng);
}
