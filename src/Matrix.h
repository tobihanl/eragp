#ifndef ERAGP_MAIMUC_EVO_2019_MATRIX_H_
#define ERAGP_MAIMUC_EVO_2019_MATRIX_H_

#include <vector>
#include <iostream>

typedef float(*MatrixFunction)(float);

class Matrix {
private:
    int height, width; //size_t is guaranteed to be able to express the maximum size of any object including array
    std::vector<float> data;

    friend std::ostream& operator<<(std::ostream &strm, const Matrix &m);

    friend Matrix operator+(Matrix lhs, const Matrix& rhs);
    friend Matrix operator-(Matrix lhs, const Matrix& rhs);
    friend Matrix operator*(Matrix lhs, const Matrix& rhs);
    friend Matrix operator/(Matrix lhs, const Matrix& rhs);

    friend Matrix operator+(Matrix lhs, float rhs);
    friend Matrix operator-(Matrix lhs, float rhs);
    friend Matrix operator*(Matrix lhs, float rhs);
    friend Matrix operator/(Matrix lhs, float rhs);

    friend bool operator==(const Matrix &lhs, const Matrix &rhs);
public:

    /**
     * Copy constructor
     * @param m pointer to Matrix to copy
     */
    Matrix(Matrix *m);

    /**
    * Creates a matrix without data to initialize it later on. Do not use unless necessary
    */
    Matrix();

    /**
     * Creates a matrix with all elements having the same value
     *
     * @param height height of the matrix
     * @param width width of the matrix
     * @param initValue initial value for all elements (defaults to 0)
     */
    Matrix(int height, int width, float initValue = 0);

    /**
     * Creates a matrix with random elements
     *
     * @param height the height of the matrix
     * @param width the width of the matrix
     * @param from the minimum value of the random elements
     * @param to the maximum value of the random elements
     */
    Matrix(int height, int width, float from, float to);
    /**
     * Creates a Matrix from a vector of elements
     *
     * @param height the height of the matrix
     * @param width the width of the matrix
     * @param data the elements of the matrix as a vector.
     * The first [width] elements will be the first row and so on.
     * The size has to be [width] * [height]
     */
    Matrix(int height, int width, std::vector<float> data);

    virtual ~Matrix();

    size_t getWidth();
    size_t getHeight();
    float getMax();
    float getMin();

    Matrix& operator+=(const Matrix &rhs);
    Matrix& operator-=(const Matrix &rhs);
    Matrix& operator*=(const Matrix &rhs);
    Matrix& operator/=(const Matrix &rhs);

    Matrix& operator+=(float rhs);
    Matrix& operator-=(float rhs);
    Matrix& operator*=(float rhs);
    Matrix& operator/=(float rhs);

    float& operator()(size_t y, size_t x);
    float operator()(size_t y, size_t x) const;

    Matrix dotProduct(const Matrix &other);
    /**
     * Applies a function of type MatrixFunction
     * @param fun the MatrixFunction to apply
     * @return itself
     */
    Matrix& apply(MatrixFunction fun);

    /**
     *
     * @return a copy of the matrix
     */
    Matrix copy();
    /**
     * Converts 1x1 matrix to float. throw error for other dimensions.
     * @return the element (0, 0)
     */
    float toFloat();
    /**
     * Transposes matrix
     * @return itself
     */
    Matrix& transpose();
};

#endif //ERAGP_MAIMUC_EVO_2019_MATRIX_H