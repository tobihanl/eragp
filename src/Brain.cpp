#include "Brain.h"
#include <cassert>
#include <cmath>
#define SQRT_6 2.4494898319244385
#define MAX_MUTATION 0.05

Brain::Brain(Brain *b) : numLayers(b->numLayers), weights(new Matrix *[(b->numLayers - 1) * 2]),
                         biases(new Matrix *[b->numLayers - 1]), lastResult(new Matrix(b->lastResult)) {
    for (int i = 0; i < numLayers - 1; i++) {
        weights[i * 2] = new Matrix(b->weights[i * 2]);
        weights[i * 2 + 1] = new Matrix(b->weights[i * 2 + 1]);
        biases[i] = new Matrix(b->biases[i]);
    }
}

Brain::Brain(int num, int sizes[]) : numLayers(num), weights(new Matrix *[(num - 1)*2]), biases(new Matrix *[num - 1]), lastResult(new Matrix(sizes[num - 1], 1, 0)) {
    //standard NN
    /*for(int i = 0; i < num - 1; i++) {
        weights[i] = new Matrix(sizes[i+1], sizes[i],
                                (float) -(SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])),
                                (float) (SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])));
        biases[i] = new Matrix(sizes[i+1], 1, 0);
    }*/
    //RNN
    for(int i = 0; i < num - 1; i++) {
        weights[i * 2] = new Matrix(sizes[i+1], sizes[i],
                                (float) -(SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])),
                                (float) (SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])));
        weights[i * 2 + 1] = new Matrix(sizes[i+1], lastResult->getHeight(),
                                    (float) -(SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])),
                                    (float) (SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])));
        biases[i] = new Matrix(sizes[i+1], 1, 0);
    }
}

Brain::Brain(void *&ptr) : numLayers(((int *) ptr)[0]), weights(new Matrix *[(((int *) ptr)[0] - 1)*2]),
                           biases(new Matrix *[((int *) ptr)[0] - 1]) {
    ptr = static_cast<float *>(ptr) + 1;
    for (int i = 0; i < numLayers - 1; i++) {
        int height = ((int *) ptr)[0];
        int width = ((int *) ptr)[1];
        std::vector<float> data(static_cast<float *>(ptr) + 2, static_cast<float *>(ptr) + 2 + height * width);
        weights[i * 2] = new Matrix(height, width, data);
        ptr = static_cast<float *>(ptr) + (2 + height * width);

        int height2 = ((int *) ptr)[0];
        int width2 = ((int *) ptr)[1];
        std::vector<float> data2(static_cast<float *>(ptr) + 2, static_cast<float *>(ptr) + 2 + height2 * width2);
        weights[i * 2 + 1] = new Matrix(height2, width2, data2);
        ptr = static_cast<float *>(ptr) + (2 + height2 * width2);

        int height3 = ((int *) ptr)[0];
        biases[i] = new Matrix(height3, 1, std::vector<float>(static_cast<float *>(ptr) + 2,
                                                              static_cast<float *>(ptr) + 2 + height3));
        ptr = static_cast<float *>(ptr) + (2 + height3);
    }
    int height = ((int *) ptr)[0];
    lastResult = new Matrix(height, 1, std::vector<float>(static_cast<float *>(ptr) + 2,
                                                          static_cast<float *>(ptr) + 2 + height));
    ptr = static_cast<float *>(ptr) + (2 + height);
}

Brain::~Brain() {
    for (int i = 0; i < numLayers - 1; i++) {
        delete weights[i * 2];
        delete weights[i * 2 + 1];
        delete biases[i];
    }
    delete[] weights;
    delete[] biases;
    delete lastResult;
}

//TODO optimize dotProduct() and usage of Matrix (not that bad, because it only stores pointers to the data)
ThinkResult Brain::think(Matrix input) {
    assert(input.getWidth() == 1 && input.getHeight() == weights[0]->getWidth() &&
           "Wrong size of input Matrix in Brain::think()");
    if(printThink) std::cout << *this << std::endl;
    if(printThink) std::cout << "Input:\n" << input << std::endl;
    for(int i = 0; i < numLayers - 1; i++) {
        input = weights[i * 2]->dotProduct(input);
        input += weights[i * 2 + 1]->dotProduct(*lastResult);
        input += *biases[i];
        input.apply(std::tanh);
        if(printThink) std::cout << "After layer " << i << ":\n" << input << std::endl;
    }
    lastResult = new Matrix(input); //TODO quickfix, make more efficient/avoid copy
    ThinkResult res = {input(0, 0), input(1, 0) > -0.5, input(2, 0) > 0, input(3, 0) > 0};
    if(printThink) std::cout << "Result:\n" << input << std::endl;
    return res;
}

Brain *Brain::createMutatedCopy() {
    auto *copy = new Brain(this);
    for (int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i * 2]) += Matrix(weights[i * 2]->getHeight(), weights[i * 2]->getWidth(), -MAX_MUTATION, MAX_MUTATION);
        *(copy->weights[i * 2 + 1]) += Matrix(weights[i * 2 + 1]->getHeight(), weights[i * 2 + 1]->getWidth(), -MAX_MUTATION, MAX_MUTATION);
        *(copy->biases[i]) += Matrix(biases[i]->getHeight(), 1, -MAX_MUTATION, MAX_MUTATION);
    }
    return copy;
}

int Brain::serializedSized() {
    int sum = 0;
    for (int i = 0; i < numLayers - 1; i++) {
        sum += weights[i * 2]->getHeight() * weights[i * 2]->getWidth() * 4 + 8;//8 for the 2 int width/height of matrix
        sum += weights[i * 2 + 1]->getHeight() * weights[i * 2 + 1]->getWidth() * 4 + 8;//8 for the 2 int width/height of matrix
        sum += biases[i]->getHeight() * 4 + 8;
    }
    sum += lastResult->getHeight() * 4 + 8;
    return 4 + sum;//+4 for numLayers
}

void Brain::serialize(void *&ptr) {
    ((int *) ptr)[0] = numLayers;
    ptr = static_cast<int *>(ptr) + 1;
    for (int i = 0; i < numLayers - 1; i++) {
        ((int *) ptr)[0] = weights[i * 2]->getHeight(); //delete *2 if going back to normal NN
        ((int *) ptr)[1] = weights[i * 2]->getWidth();
        assert(weights[i * 2]->data.end() - weights[i * 2]->data.begin() == weights[i * 2]->getHeight() * weights[i * 2]->getWidth());
        std::copy(weights[i * 2]->data.begin(), weights[i * 2]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + weights[i * 2]->getHeight() * weights[i * 2]->getWidth());

        //JUST FOR RNN
        ((int *) ptr)[0] = weights[i * 2 + 1]->getHeight();
        ((int *) ptr)[1] = weights[i * 2 + 1]->getWidth();
        assert(weights[i * 2  + 1]->data.end() - weights[i * 2 + 1]->data.begin() == weights[i * 2 + 1]->getHeight() * weights[i * 2 + 1]->getWidth());
        std::copy(weights[i * 2  + 1]->data.begin(), weights[i * 2 + 1]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + weights[i * 2 + 1]->getHeight() * weights[i * 2 + 1]->getWidth());
        //JUST FOR RNN

        ((int *) ptr)[0] = biases[i]->getHeight();
        ((int *) ptr)[1] = 1;
        assert(biases[i]->data.end() - biases[i]->data.begin() == biases[i]->getHeight());
        std::copy(biases[i]->data.begin(), biases[i]->data.end(),
                  ((float *) ptr) + 2);// +2 only works because of same size
        ptr = static_cast<float *>(ptr) + (2 + biases[i]->getHeight());
    }
    ((int *) ptr)[0] = lastResult->getHeight();
    ((int *) ptr)[1] = 1;
    std::copy(lastResult->data.begin(), lastResult->data.end(),
              ((float *) ptr) + 2);// +2 only works because of same size
    ptr = static_cast<float *>(ptr) + (2 + lastResult->getHeight());
}

std::ostream &operator<<(std::ostream &strm, const Brain &b) {
    strm << "Brain[numLayers: " << b.numLayers;
    for (int i = 0; i < b.numLayers - 1; i++) {
        strm << std::endl << "w[" << (i * 2) << "]:\n " << *b.weights[i * 2];
        strm << std::endl << "w[" << (i * 2 + 1) << "]:\n " << *b.weights[i * 2 + 1];
        strm << std::endl << "b[" << i << "]:\n " << *b.biases[i];
    }
    strm << std::endl << "last:\n " << *b.lastResult;
    strm << "]";
    return strm;
}
