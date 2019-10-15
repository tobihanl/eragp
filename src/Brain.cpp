#include "Brain.h"
#include <assert.h>
#include <cmath>

Brain::Brain(Brain *b) : numLayers(b->numLayers), weights(new Matrix *[b->numLayers - 1]),
                         biases(new Matrix *[b->numLayers - 1]) {
    for (int i = 0; i < numLayers - 1; i++) {
        weights[i] = new Matrix(b->weights[i]);
        biases[i] = new Matrix(b->biases[i]);
    }

}

Brain::Brain(int continuousInSize, int hiddenPreSize, int processedInSize, int normalizedInSize, int hiddenSize,
             int outSize) : numLayers(5), weights(new Matrix *[4]), biases(new Matrix *[4]) {
    weights[0] = new Matrix(hiddenPreSize, continuousInSize, -std::sqrt(1.f / continuousInSize),
                            std::sqrt(1.f / continuousInSize));
    biases[0] = new Matrix(hiddenPreSize, 1, 0);
    weights[1] = new Matrix(processedInSize, hiddenPreSize, -std::sqrt(1.f / hiddenPreSize),
                            std::sqrt(1.f / hiddenPreSize));
    biases[1] = new Matrix(processedInSize, 1, 0);
    weights[2] = new Matrix(hiddenSize, processedInSize + normalizedInSize,
                            -std::sqrt(1.f / (processedInSize + normalizedInSize)),
                            std::sqrt(1.f / processedInSize + normalizedInSize));
    biases[2] = new Matrix(hiddenSize, 1, 0);
    weights[3] = new Matrix(outSize, hiddenSize, -std::sqrt(1.f / hiddenSize), std::sqrt(1.f / hiddenSize));
    biases[3] = new Matrix(outSize, 1, 0);

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
    for (int i = 0; i < numLayers - 1; i++) {
        delete weights[i];
        delete biases[i];
    }
}

//TODO optimize dotProduct() and usage of Matrix (not that bad, because it only stores pointers to the data), implement normalization
ThinkResult Brain::think(Matrix input, Matrix normalizedInput) {
    assert(input.getWidth() == 1 && input.getHeight() == weights[0]->getWidth() &&
           "Wrong size of continuous input Matrix in Brain::think()");
    assert(normalizedInput.getWidth() == 1 &&
           normalizedInput.getHeight() + weights[1]->getHeight() == weights[2]->getWidth() &&
           "Wrong size of normalized input Matrix in Brain::think()");

    input = weights[0]->dotProduct(input);
    input += biases[0];
    input.apply(std::tanh);

    input = weights[1]->dotProduct(input);
    input += biases[1];
    input.apply(std::tanh);

    input.data.reserve(input.height + normalizedInput.height);
    input.data.insert(input.data.end(), normalizedInput.data.begin(), normalizedInput.data.end());
    input.height += normalizedInput.height;
    input = weights[2]->dotProduct(input);
    input += biases[2];
    input.apply(std::tanh);

    input = weights[3]->dotProduct(input);
    input += biases[3];
    input.apply(std::tanh);

    ThinkResult res = {input(0, 0), input(1, 0) > -0.5, input(2, 0) > 0, input(3, 0) > 0};
    return res;
}

Brain *Brain::createMutatedCopy() {
    Brain *copy = new Brain(this);
    for (int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i]) += new Matrix(weights[i]->getHeight(), weights[i]->getWidth(), -0.01, 0.01);
        *(copy->biases[i]) += new Matrix(biases[i]->getHeight(), 1, -0.01, 0.01);
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