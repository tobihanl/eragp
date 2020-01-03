#ifndef ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
#define ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H

#include "Matrix.h"

struct ThinkResult {
    float rotation;
    float speed;
    bool attack;
    bool share;
};

class Brain {
private:
    int numLayers;
    Matrix **weights;
    Matrix **biases;

    /**
     * Copy constructor
     */
    explicit Brain(Brain *b);

    friend std::ostream &operator<<(std::ostream &strm, const Brain &b);

public:
    /**
     *
     * @param numLayers Amount of layers including input and output layer
     * @param sizes Sizes of the layers
     */
    Brain(int numLayers, int sizes[]);

    /**
     * Deserialization
     * @param ptr first byte to deserialize. Points to next free byte after execution.
     */
    explicit Brain(void *&ptr);

    virtual ~Brain();

    ThinkResult think(Matrix input);

    /**
     * Creates a mutated copy. Remember to delete it after usage-
     * @return pointer to the mutated copy
     */

    Brain *createMutatedCopy(LFSR *random);

    int serializedSized();

    void serialize(void *&ptr);

    int getNumLayers() {
        return numLayers;
    }

    bool printThink = false; //TODO remove
};

#endif //ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
