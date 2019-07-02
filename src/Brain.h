#ifndef ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
#define ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H

#include "Matrix.h"

class Brain {
private:
    Matrix weights[];
    Matrix biases[];
public:
    Brain(int layerAmount, int layerSizes[]);

    //public const Matrix& think();
    
};


#endif //ERAGP_MAIMUC_EVO_2019_DESKTOP_BRAIN_H
