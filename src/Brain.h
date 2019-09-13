#ifndef ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
#define ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H

#include "Matrix.h"

class Brain {
private:
    int numLayers;
    Matrix **weights;
    Matrix **biases;

    /**
     * Copy constructor
     */
    Brain(Brain *b);

    friend std::ostream &operator<<(std::ostream &strm, const Brain &b);
public:
    Brain(int layerAmount, int layerSizes[]);

    /**
     * Deserialization
     * @param ptr first byte to deserialize. Points to next free byte after execution.
     */
    Brain(void *&ptr);

    virtual ~Brain();

    Matrix think(Matrix input);

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
