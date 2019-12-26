#ifndef ERAGP_MAIMUC_EVO_2019_MATRIX_H_
#define ERAGP_MAIMUC_EVO_2019_MATRIX_H_

#include <vector>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "Lfsr.h"

typedef float(*MatrixFunction)(float);

class Matrix {
private:
    friend std::ostream &operator<<(std::ostream &strm, const Matrix &m);

    //########################### Matrix/Matrix operators ###########################
    friend Matrix operator+(Matrix lhs, const Matrix &rhs) {
        lhs += rhs;
        return lhs;
    }

    friend Matrix operator-(Matrix lhs, const Matrix &rhs) {
        lhs -= rhs;
        return lhs;
    }

    friend Matrix operator*(Matrix lhs, const Matrix &rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend Matrix operator/(Matrix lhs, const Matrix &rhs) {
        lhs /= rhs;
        return lhs;
    }

    //########################### Matrix/float operators ###########################
    friend Matrix operator+(Matrix lhs, float rhs) {
        lhs += rhs;
        return lhs;
    }

    friend Matrix operator-(Matrix lhs, float rhs) {
        lhs -= rhs;
        return lhs;
    }

    friend Matrix operator*(Matrix lhs, float rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend Matrix operator/(Matrix lhs, float rhs) {
        lhs /= rhs;
        return lhs;
    }

    //########################### Comparison ###########################
    friend bool operator==(const Matrix &lhs, const Matrix &rhs) {
        if (lhs.width != rhs.width || lhs.height != rhs.height) return false;
        int s = lhs.width * lhs.height;
        for (int i = 0; i < s; i++) {
            if (fabs(lhs.data[i] - rhs.data[i]) >= lhs.data[i] * 0.001) return false;
        }
        return true;
    }

    friend bool operator!=(const Matrix &lhs, const Matrix &rhs) {
        return !(lhs == rhs);
    }

public:
    int height, width;


    std::vector<float> data;

    /**
     * Copy constructor
     * @param m pointer to Matrix to copy
     */
    explicit Matrix(Matrix *m) : height(m->height), width(m->width), data(m->data) {};

    /**
    * Creates a matrix without data to initialize it later on. Do not use unless necessary
    */
    Matrix() : width(1), height(1), data(1) {}

    /**
     * Creates a matrix with all elements having the same value
     *
     * @param height height of the matrix
     * @param width width of the matrix
     * @param initValue initial value for all elements (defaults to 0)
     */
    Matrix(int h, int w, float initValue = 0) : height(h), width(w), data(w * h, initValue) {}

    /**
     * Creates a Matrix from a vector of elements
     *
     * @param height the height of the matrix
     * @param width the width of the matrix
     * @param data the elements of the matrix as a vector.
     * The first [width] elements will be the first row and so on.
     * The size has to be [width] * [height]
     */
    Matrix(int height, int width, const std::vector<float> &data) : height(height), width(width), data(data) {
        if (width * height != data.size())
            throw std::invalid_argument(
                    "A Matrix of dimensions (" + std::to_string(height) + " x " + std::to_string(width) +
                    ") must have " + std::to_string(width * height) + " instead of " + std::to_string(data.size()) +
                    " arguments.");
    }

    /**
     * Creates a matrix with random elements
     *
     * @param height the height of the matrix
     * @param width the width of the matrix
     * @param from the minimum value of the random elements
     * @param to the maximum value of the random elements
     * @param random Random number generator to use
     */
    Matrix(int height, int width, float from, float to, LFSR *random);

    virtual ~Matrix() = default;

    int getWidth() { return width; }

    int getHeight() { return height; }

    float getMax() {
        float max = data[0];
        for (float d : data) {
            if (d > max) max = d;
        }
        return max;
    }

    float getMin() {
        float min = data[0];
        for (float d : data) {
            if (d < min) min = d;
        }
        return min;
    }

    Matrix &operator+=(const Matrix &rhs);

    Matrix &operator-=(const Matrix &rhs);

    Matrix &operator*=(const Matrix &rhs);

    Matrix &operator/=(const Matrix &rhs);

    //########################### Matrix/float operators ###########################
    Matrix &operator+=(float rhs) {
        int s = width * height;
        for (int i = 0; i < s; i++) {
            data[i] += rhs;
        }
        return *this;
    }

    Matrix &operator-=(float rhs) {
        int s = width * height;
        for (int i = 0; i < s; i++) {
            data[i] -= rhs;
        }
        return *this;
    }

    Matrix &operator*=(float rhs) {
        int s = width * height;
        for (int i = 0; i < s; i++) {
            data[i] *= rhs;
        }
        return *this;
    }

    Matrix &operator/=(float rhs) {
        int s = width * height;
        for (int i = 0; i < s; i++) {
            data[i] /= rhs;
        }
        return *this;
    }

    //########################### Matrix Subscript ###########################
    float &operator()(int y, int x) {
        if (x >= width || y >= height)
            throw std::invalid_argument("The indices (" + std::to_string(y) + ", " + std::to_string(x) +
                                        ") are invalid for a Matrix of size (" + std::to_string(height) + " x " +
                                        std::to_string(width) + ").");
        return data[y * width + x];
    }

    float operator()(int y, int x) const {
        if (x >= width || y >= height)
            throw std::invalid_argument(
                    "The indices (" + std::to_string(y) + ", " + std::to_string(x) +
                    ") are invalid for a Matrix of size (" + std::to_string(height) + " x " + std::to_string(width) +
                    ").");
        return data[y * width + x];
    }

    Matrix dotProduct(const Matrix &other);

    /**
     * Applies a function of type MatrixFunction
     * @param fun the MatrixFunction to apply
     * @return itself
     */
    Matrix &apply(MatrixFunction fun) {
        int s = width * height;
        for (int i = 0; i < s; i++) {
            data[i] = fun(data[i]);
        }
        return *this;
    }

    /**
     * Copy.
     *
     * @return a copy of the matrix
     */
    Matrix copy() { return Matrix(height, width, data); }

    /**
     * Converts 1x1 matrix to float. throw error for other dimensions.
     * @return the element (0, 0)
     */
    float toFloat() {
        if (width != 1 || height != 1)
            throw std::invalid_argument(
                    "Matrix must be of dimensions (1 x 1) instead of (" + std::to_string(height) + " x " +
                    std::to_string(width) + ") to convert to float.");
        return data[0];
    }

    /**
     * Transposes matrix
     * @return itself
     */
    Matrix &transpose();

    int serializedSize() { return (2 + width * height) * 4; }
};

#endif //ERAGP_MAIMUC_EVO_2019_MATRIX_H