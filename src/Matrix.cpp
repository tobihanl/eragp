#include <algorithm>
#include "Matrix.h"
#include "Rng.h"

Matrix::Matrix(int h, int w, float from, float to) : height(h), width(w) {
    std::uniform_real_distribution<float> dist(from, to);

    auto gen = [&dist]() { //lambda expression
        return dist(rng);
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
    if (width != rhs.width || height != rhs.height)
        throw std::invalid_argument(
                "Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" +
                std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] += rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator-=(const Matrix &rhs) {
    if (width != rhs.width || height != rhs.height)
        throw std::invalid_argument(
                "Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" +
                std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] -= rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator*=(const Matrix &rhs) {
    if (width != rhs.width || height != rhs.height)
        throw std::invalid_argument(
                "Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" +
                std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] *= rhs.data[i];
    }
    return *this;
}

Matrix &Matrix::operator/=(const Matrix &rhs) {
    if (width != rhs.width || height != rhs.height)
        throw std::invalid_argument(
                "Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" +
                std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    int s = width * height;
    for (int i = 0; i < s; i++) {
        data[i] /= rhs.data[i];
    }
    return *this;
}

Matrix Matrix::dotProduct(const Matrix &other) {
    if (width != other.height)
        throw std::invalid_argument(
                "Width of the first Matrix with dimensions (" + std::to_string(height) + " x " + std::to_string(width) +
                ") must be equal to the height of the second Matrix with dimensions (" + std::to_string(other.height) +
                " x " + std::to_string(other.width) + ").");
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

Matrix &Matrix::transpose() {
    if (width == height) {
        for (int x = 0; x < width - 1; x++) {
            for (int y = x + 1; y < width; y++) { // y = x + 1 -> always takes all values below the "middle line"
                //swap data[x, y] and data[y, x]
                float temp = data[y * width + x];
                data[y * width + x] = data[x * width + y];
                data[x * width + y] = temp;
            }
        }
    } else { //inefficient af, because I copy it instead of calculating permutation circles
        std::vector<float> dataNew(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                dataNew[x * height + y] = data[y * width + x];
            }
        }
        data = dataNew;
        int w = width;
        width = height;
        height = w;
    }
    return *this;
}
