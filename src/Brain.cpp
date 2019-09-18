#include "Brain.h"
#include <assert.h>

Brain::Brain(Brain *b) : numLayers(b->numLayers), weights(new Matrix *[b->numLayers - 1]),
                         biases(new Matrix *[b->numLayers - 1]) {
    for (int i = 0; i < numLayers - 1; i++) {
        weights[i] = new Matrix(b->weights[i]);
        biases[i] = new Matrix(b->biases[i]);
    }

}

Brain::Brain(int amount, int sizes[]) : numLayers(amount), weights(new Matrix *[amount - 1]),
                                        biases(new Matrix *[amount - 1]) {
    for (int i = 0; i < amount - 1; i++) {
        weights[i] = new Matrix(sizes[i + 1], sizes[i], 0.0f, 1.0f);
        biases[i] = new Matrix(sizes[i + 1], 1, 0.0f, 1.0f);
    }
}

Brain::Brain(void *&ptr) : numLayers(((int *) ptr)[0]), weights(new Matrix *[((int *) ptr)[0] - 1]),
                           biases(new Matrix *[((int *) ptr)[0] - 1]) {
    ptr = static_cast<float *>(ptr) + 1;
    for (int i = 0; i < numLayers - 1; i++) {
        int height = ((int *) ptr)[0];
        int width = ((int *) ptr)[1];
        std::vector<float> data(static_cast<float *>(ptr) + 2, static_cast<float *>(ptr) + 2 + height * width);
        weights[i] = new Matrix(height, width, data);
        ptr = static_cast<float *>(ptr) + (2 + height * width);
        int height2 = ((int *) ptr)[0];
        biases[i] = new Matrix(height2, 1, std::vector<float>(static_cast<float *>(ptr) + 2,
                                                              static_cast<float *>(ptr) + 2 + height2));
        ptr = static_cast<float *>(ptr) + (2 + height2);
    }
}

Brain::~Brain() {
    delete[] weights;
    delete[] biases;
}

//TODO optimize dotProduct() and usage of Matrix (not that bad, because it only stores pointers to the data), implement normalization
Matrix Brain::think(Matrix input) {
    assert(input.getWidth() == 1 && input.getHeight() == weights[0]->getWidth() &&
           "Wrong size of input Matrix in Brain::think()");

    for (int i = 0; i < numLayers - 1; i++) {
        input = weights[i]->dotProduct(input);
        input += biases[i];
    }
    return input;
}

Brain *Brain::createMutatedCopy() {
    Brain *copy = new Brain(this);
    for (int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i]) += new Matrix(weights[i]->getHeight(), weights[i]->getWidth(), -0.05, 0.05);
        *(copy->biases[i]) += new Matrix(biases[i]->getHeight(), 1, -0.05, 0.05);
    }
    return copy;
}

int Brain::serializedSized() {
    int sum = 0;
    for (int i = 0; i < numLayers - 1; i++) {
        sum += weights[i]->getHeight() * weights[i]->getWidth() * 4 + 8;//8 for the 2 int width/height of matrix
        sum += biases[i]->getHeight() * 4 + 8;
    }
    return 4 + sum;
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
        ((int *) ptr)[1] = 1;
        assert(biases[i]->data.end() - biases[i]->data.begin() == biases[i]->getHeight());
        std::copy(biases[i]->data.begin(), biases[i]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + biases[i]->getHeight());
    }
}

int Brain::getNumLayers() {
    return numLayers;
}

std::ostream &operator<<(std::ostream &strm, const Brain &b) {
    strm << "Brain[numLayers: " << b.numLayers;
    for (int i = 0; i < b.numLayers - 1; i++) {
        strm << std::endl << "w[" << i << "]: " << *b.weights[i];
        strm << std::endl << "b[" << i << "]: " << *b.biases[i];
    }
    strm << "]";
    return strm;
}
