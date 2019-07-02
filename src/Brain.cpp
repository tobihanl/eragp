#include "Brain.h"

Brain::Brain(int amount, int sizes[]) : weights(amount - 1), biases(amount - 1) {
    for(int i = 0; i < amount - 1; i++) {
        layers[i] = Matrix(sizes[i + 1], sizes[i], 0.0f, 1.0f);
        biases[i] = Matrix(sizes[i + 1], 1, 0.0f, 1.0f);
    }
}
