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

Brain::Brain(void *&ptr) : numLayers(((int *) ptr)[0]), weights(new Matrix *[(((int *) ptr)[0] - 1)]),
                           biases(new Matrix *[((int *) ptr)[0] - 1]) {
    ptr = static_cast<float *>(ptr) + 1;
    for (int i = 0; i < numLayers - 1; i++) {
        int height = ((int *) ptr)[0];
        int width = ((int *) ptr)[1];
        std::vector<float> data(static_cast<float *>(ptr) + 2, static_cast<float *>(ptr) + 2 + height * width);
        weights[i] = new Matrix(height, width, data);
        ptr = static_cast<float *>(ptr) + (2 + height * width);

        int height3 = ((int *) ptr)[0];
        int width3 = ((int *) ptr)[1];
        biases[i] = new Matrix(height3, width3, std::vector<float>(static_cast<float *>(ptr) + 2,
                                                              static_cast<float *>(ptr) + 2 + height3 * width3));
        ptr = static_cast<float *>(ptr) + (2 + height3 * width3);
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
    if(printThink) std::cout << *this << std::endl;
    if(printThink) std::cout << "Input:\n" << input << std::endl;
    for(int i = 0; i < numLayers - 1; i++) {
        input = input.dotProduct(*weights[i]);
        input += *biases[i];
        input.apply(std::tanh);
        if(printThink) std::cout << "After layer " << i << ":\n" << input << std::endl;
    }
    assert(input.height == 1 && input.width == 4 && "ThinkResult does not have the correct size.");
    ThinkResult res = {input(0, 0), input(0, 1) < 0 ? 0 : input(0, 1), input(0, 2) > 0, input(0, 3) > 0};
    if(printThink) std::cout << "Result:\n" << input << std::endl;
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
    int sum = 0;
    for (int i = 0; i < numLayers - 1; i++) {
        sum += weights[i]->getHeight() * weights[i]->getWidth() * 4 + 8;//8 for the 2 int width/height of matrix
        sum += biases[i]->getHeight() * biases[i]->getWidth() * 4 + 8;
    }
    return 4 + sum;//+4 for numLayers
}

void Brain::serialize(void *&ptr) {
    ((int *) ptr)[0] = numLayers;
    ptr = static_cast<int *>(ptr) + 1;
    for (int i = 0; i < numLayers - 1; i++) {
        ((int *) ptr)[0] = weights[i]->getHeight();
        ((int *) ptr)[1] = weights[i]->getWidth();
        assert(weights[i]->data.end() - weights[i]->data.begin() == weights[i]->getHeight() * weights[i]->getWidth());
        std::copy(weights[i]->data.begin(), weights[i]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + weights[i]->getHeight() * weights[i]->getWidth());

        ((int *) ptr)[0] = biases[i]->getHeight();
        ((int *) ptr)[1] = biases[i]->getWidth();
        assert(biases[i]->data.end() - biases[i]->data.begin() == biases[i]->getHeight() * biases[i]->getWidth());
        std::copy(biases[i]->data.begin(), biases[i]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + biases[i]->getHeight() * biases[i]->getWidth());
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
