#include <algorithm>
#include <cassert>
#include "Matrix.h"

Matrix::Matrix(int h, int w, float from, float to, LFSR *random) : height(h), width(w) {
    auto gen = [&random, from, to]() { //lambda expression
        return random->getNextFloatBetween(from, to);
    };
    data.resize(w * h);
    generate(data.begin(), data.end(), gen);
}

std::ostream &operator<<(std::ostream &strm, const Matrix &m) {
    for (int y = 0; y < m.height; y++) {
        strm << "[";
        for (int x = 0; x < m.width - 1; x++) {
            strm << m.data[y * m.width + x] << ", ";
        }
        if (m.width != 0) strm << m.data[(y + 1) * m.width - 1];
        strm << "]" << std::endl;
    }
    return strm;
}

//########################### Matrix/Matrix operators ###########################

Matrix &Matrix::operator+=(const Matrix &rhs) {
    assert(width == rhs.width && height == rhs.height && "Matrix dimensions do not match!");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] += rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator-=(const Matrix &rhs) {
    assert(width == rhs.width && height == rhs.height && "Matrix dimensions do not match!");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] -= rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator*=(const Matrix &rhs) {
    assert(width == rhs.width && height == rhs.height && "Matrix dimensions do not match!");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] *= rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator/=(const Matrix &rhs) {
    assert(width == rhs.width && height == rhs.height && "Matrix dimensions do not match!");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] /= rhs.data[i];
    }
    return *this;
}

Matrix Matrix::dotProduct(const Matrix &other) {
    assert(width == other.height && "Width of the first Matrix must be equal to the height of the second Matrix1");
    Matrix m(height, other.width);
    for (int x2 = 0; x2 < other.width; x2++) {
        for (int y1 = 0; y1 < height; y1++) {
            for (int x1 = 0; x1 < width; x1++) {
                m.data[y1 * m.width + x2] += data[y1 * width + x1] * other.data[x1 * other.width + x2];
            }
        }
    }
    return m;
}
