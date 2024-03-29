#include "Brain.h"
#include <cassert>
#include <cmath>

#define SQRT_6 2.4494898319244385
#define MAX_MUTATION 0.05

Brain::Brain(Brain *b) : numLayers(b->numLayers), weights(new Matrix *[(b->numLayers - 1)]),
                         biases(new Matrix *[b->numLayers - 1]) {
    for (int i = 0; i < numLayers - 1; i++) {
        weights[i] = new Matrix(b->weights[i]);
        biases[i] = new Matrix(b->biases[i]);
    }
}

Brain::Brain(void *&ptr) {
    numLayers = ((int32_t *) ptr)[0];
    weights = new Matrix *[numLayers - 1];
    biases = new Matrix *[numLayers - 1];
    ptr = static_cast<int32_t *>(ptr) + 1;

    for (int i = 0; i < numLayers - 1; i++) {
        int height = ((int32_t *) ptr)[0];
        int width = ((int32_t *) ptr)[1];
        ptr = static_cast<int32_t *>(ptr) + 2;

        std::vector<float> data(static_cast<float *>(ptr), static_cast<float *>(ptr) + (height * width));
        weights[i] = new Matrix(height, width, data);
        ptr = static_cast<float *>(ptr) + (height * width);

        int height2 = ((int32_t *) ptr)[0];
        int width2 = ((int32_t *) ptr)[1];
        ptr = static_cast<int32_t *>(ptr) + 2;

        std::vector<float> data2(static_cast<float *>(ptr), static_cast<float *>(ptr) + (height2 * width2));
        biases[i] = new Matrix(height2, width2, data2);
        ptr = static_cast<float *>(ptr) + (height2 * width2);
    }
}

Brain::~Brain() {
    for (int i = 0; i < numLayers - 1; i++) {
        delete weights[i];
        delete biases[i];
    }
    delete[] weights;
    delete[] biases;
}

ThinkResult Brain::think(Matrix input) {
    assert(input.getWidth() == weights[0]->getHeight() && input.getHeight() == 1 &&
           "Wrong size of input Matrix in Brain::think()");
    for(int i = 0; i < numLayers - 1; i++) {
        input = input.dotProduct(*weights[i]);
        input += *biases[i];
        input.apply(std::tanh);
    }
    assert(input.height == 1 && input.width == 4 && "ThinkResult does not have the correct size.");
    ThinkResult res = {input(0, 0), input(0, 1) < 0 ? 0 : input(0, 1), input(0, 2) > 0, input(0, 3) > 0};
    return res;
}

Brain *Brain::createMutatedCopy(LFSR *random) {
    auto *copy = new Brain(this);
    for (int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i]) += Matrix(weights[i]->getHeight(), weights[i]->getWidth(), -MAX_MUTATION, MAX_MUTATION, random);
        *(copy->biases[i]) += Matrix(biases[i]->getHeight(), biases[i]->getWidth(), -MAX_MUTATION, MAX_MUTATION, random);
    }
    return copy;
}

int Brain::serializedSized() {
    int sum = sizeof(int32_t);
    for (int i = 0; i < numLayers - 1; i++) {
        sum += weights[i]->getHeight() * weights[i]->getWidth() * (int) sizeof(float) + 2 * (int) sizeof(int32_t);
        sum += biases[i]->getHeight() * biases[i]->getWidth() * (int) sizeof(float) + 2 * (int) sizeof(int32_t);
    }
    return sum;
}

void Brain::serialize(void *&ptr) {
    ((int32_t *) ptr)[0] = (int32_t) numLayers;
    ptr = static_cast<int32_t *>(ptr) + 1;

    for (int i = 0; i < numLayers - 1; i++) {
        assert(biases[i]->data.end() - biases[i]->data.begin() == biases[i]->getHeight() * biases[i]->getWidth());
        assert(weights[i]->data.end() - weights[i]->data.begin() == weights[i]->getHeight() * weights[i]->getWidth());

        ((int32_t *) ptr)[0] = (int32_t) weights[i]->getHeight();
        ((int32_t *) ptr)[1] = (int32_t) weights[i]->getWidth();
        ptr = static_cast<int32_t *>(ptr) + 2;

        std::copy(weights[i]->data.begin(), weights[i]->data.end(), (float *) ptr);
        ptr = static_cast<float *>(ptr) + (weights[i]->getHeight() * weights[i]->getWidth());

        ((int32_t *) ptr)[0] = (int32_t) biases[i]->getHeight();
        ((int32_t *) ptr)[1] = (int32_t) biases[i]->getWidth();
        ptr = static_cast<int32_t *>(ptr) + 2;

        std::copy(biases[i]->data.begin(), biases[i]->data.end(), (float *) ptr);
        ptr = static_cast<float *>(ptr) + (biases[i]->getHeight() * biases[i]->getWidth());
    }
}

std::ostream &operator<<(std::ostream &strm, const Brain &b) {
    strm << "Brain[numLayers: " << b.numLayers;
    for (int i = 0; i < b.numLayers - 1; i++) {
        strm << std::endl << "w[" << (i) << "]:\n " << *b.weights[i];
        strm << std::endl << "b[" << i << "]:\n " << *b.biases[i];
    }
    strm << "]";
    return strm;
}
