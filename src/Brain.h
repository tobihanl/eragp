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

public:
    Brain(int layerAmount, int layerSizes[]);
    virtual ~Brain();

    Matrix think(Matrix input);
    /**
     * Creates a mutated copy. Remember to delete it after usage-
     * @return pointer to the mutated copy
     */
    Brain* createMutatedCopy();
    
};


#endif //ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
