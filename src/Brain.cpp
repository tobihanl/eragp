#include "Brain.h"
#include <assert.h>

Brain::Brain(Brain *b) : numLayers(b->numLayers), weights(new Matrix*[b->numLayers - 1]), biases(new Matrix*[b->numLayers - 1]) {
    for(int i = 0; i < numLayers - 1; i++) {
        weights[i] = new Matrix(b->weights[i]);
        biases[i] = new Matrix(b->biases[i]);
    }

}

Brain::Brain(int amount, int sizes[]) : numLayers(amount), weights(new Matrix*[amount - 1]), biases(new Matrix*[amount - 1]) {
    for(int i = 0; i < amount - 1; i++) {
        weights[i] = new Matrix(sizes[i + 1], sizes[i], 0.0f, 1.0f);
        biases[i] = new Matrix(sizes[i + 1], 1, 0.0f, 1.0f);
    }
}

Brain::~Brain() {
    delete[] weights;
    delete[] biases;
}

//TODO optimize dotProduct() and usage of Matrix (not that bad, because it only stores pointers to the data), implement normalization
Matrix Brain::think(Matrix input) {
    assert(input.getWidth() == 1 && input.getHeight() == weights[0]->getWidth() && "Wrong size of input Matrix in Brain::think()");

    for(int i = 0; i < numLayers - 1; i++) {
        input = weights[i]->dotProduct(input);
        input += biases[i];
    }
    return input;
}

Brain* Brain::createMutatedCopy() {
    Brain *copy = new Brain(this);
    for(int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i]) += new Matrix(weights[i]->getHeight(), weights[i]->getWidth(), -0.05, 0.05);
        *(copy->biases[i]) += new Matrix(biases[i]->getHeight(), 1, -0.05, 0.05);
    }
    return copy;
}