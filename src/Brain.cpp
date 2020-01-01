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

Brain::Brain(int num, int sizes[]) : numLayers(num), weights(new Matrix *[(num - 1)]), biases(new Matrix *[num - 1]) {
    /*for(int i = 0; i < num - 1; i++) {
        weights[i] = new Matrix(sizes[i+1], sizes[i],
                                (float) -(SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])),
                                (float) (SQRT_6 / std::sqrt(sizes[i] + sizes[i+1])));
        biases[i] = new Matrix(sizes[i+1], 1, 0);
    }*/
    //TODO adjust
    weights[0] = new Matrix(sizes[0], sizes[1], {-6.997804, -0.81039757, 0.2075354, -0.013004906, -7.8445244, -0.9186338, -0.4000983, -0.045625377,
                                                 0.05314104, -0.016720857, -0.39349753, 0.536546, 0.2565775, 0.030787194, 0.20450738, 0.9535477,
                                                 0.20072763, -0.28743988, -0.08773946, 1.1504635, -0.38866192, 0.40020505, -0.042091217, 0.40625733,
                                                 0.004629854, 0.13188213, -0.03353278, 0.0006667691, -0.049692016, 0.12220437, 0.062767714, 0.013447184,
                                                 -0.22306378, -0.10067815, -0.02454825, 0.20216839, -0.011268353, -0.54071325, -0.015744686, 0.13403846,
                                                 0.19546148, -0.025772091, 0.010203318, -0.0035599903, 0.04550849, -0.068121254, -0.014139647, -0.0017841632,
                                                 -0.27228633, 0.09382318, 0.43772304, 0.90640074, -0.40264452, -0.5718175, 0.1708884, 1.0498749,
                                                 -0.29480976, 0.44161642, -0.15961838, 0.3557366, -0.135133, -0.09403086, -0.31346336, 0.50882334,
                                                 -0.025962086, -0.31383663, -0.63613176, 3.4793186, 0.017126137, -0.3532819, 1.0353293, -3.5775049,
                                                 -0.21428762, -0.16340294, -0.17085667, -0.17071712, -0.010921616, -0.20517789, -0.12620819, -0.19957899,
                                                 0.22033207, 0.17011526, 0.17153494, 0.17463565, 0.009677466, 0.19441833, 0.12854262, 0.19649689,
                                                 0.22481032, 0.34594643, 0.011964929, -0.19673212, 0.049646437, -0.01461857, 0.028413318, -0.12644103,
                                                 0.012176632, 0.01766032, -0.0038147618, -0.008248196, 0.018436909, 0.004287923, 0.010330192, -0.0018148007,
                                                 0.25205883, -0.16002762, -0.51077735, -0.4483629, -0.44602478, -0.33861408, -0.48378828, -0.83955884});
    biases[0] = new Matrix(1, sizes[1], {0.22677062, 0.15607685, 0.055244744, 0.7414462, 0.07206411, -0.11796252, 0.17464042, 0.6116576});

    weights[1] = new Matrix(sizes[1], sizes[2], {0.061702512, -4.801741, 1.0129149, 0.8794751,
                                                 -0.317838, -0.46495503, -0.76563746, -1.0492343,
                                                 -1.3235898, 0.28488132, -1.0206308, -0.23682694,
                                                 1.3795791, 0.20678356, -2.1416028, -1.3627205,
                                                 -0.05944877, -2.8916774, 1.6126696, 1.6837629,
                                                 -0.12454005, -0.4249059, -0.04751572, -0.16498646,
                                                 0.5141643, -0.25508183, -1.2846099, -1.9092698,
                                                 -1.284557, 0.07724181, -0.95667297, -1.6971796});
    biases[1] = new Matrix(1, sizes[2], {0.18327999, 0.28678775, -0.77836573, -0.8042165});
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

Brain *Brain::createMutatedCopy() {
    auto *copy = new Brain(this);
    for (int i = 0; i < numLayers - 1; i++) {
        *(copy->weights[i]) += Matrix(weights[i]->getHeight(), weights[i]->getWidth(), -MAX_MUTATION, MAX_MUTATION);
        *(copy->biases[i]) += Matrix(biases[i]->getHeight(), biases[i]->getWidth(), -MAX_MUTATION, MAX_MUTATION);
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
