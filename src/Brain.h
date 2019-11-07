#ifndef ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
#define ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H

#include "Matrix.h"

struct ThinkResult {
    float rotation;
    bool move;
    bool attack;
    bool share;
};

class Brain {
private:
    int numLayers;//still used for flexibility in further expansion
    Matrix **weights;
    Matrix **biases;

    /**
     * Copy constructor
     */
    explicit Brain(Brain *b);

    friend std::ostream &operator<<(std::ostream &strm, const Brain &b);
public:
    /**
     * Amount of layers is now fixed for simplicity with exactly one hidden layer for preprocessing and one for actual.
     *
     * @param continuousInSize amount of continuous inputs (e.g. energy, distance...)
     * @param hiddenPreSize amount of neurons in the hidden layer fore preprocessing of the continuous data
     * @param processedInSize amount of output neurons of the preprocessing network -> additional input of actual network
     * @param normalizedInSize amount of inputs in range -1 to 1 (rotation, landInFront)
     * @param hiddenSize amount of hidden neurons in the actual thinking
     * @param outSize amount of decisions to be made
     */
    Brain(int continuousInSize, int hiddenPreSize, int processedInSize, int normalizedInSize, int hiddenSize,
          int outSize);

    /**
     * Deserialization
     * @param ptr first byte to deserialize. Points to next free byte after execution.
     */
    explicit Brain(void *&ptr);

    virtual ~Brain();

    ThinkResult think(Matrix continuousInput, Matrix normalizedInput);

    /**
     * Creates a mutated copy. Remember to delete it after usage-
     * @return pointer to the mutated copy
     */

    Brain *createMutatedCopy();

    int serializedSized();

    void serialize(void *&ptr);

    int getNumLayers();
};

#endif //ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
