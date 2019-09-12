#include "Matrix.h"
#include <stdexcept>
#include <math.h>
#include <random>
#include <algorithm>
#include <iterator>
#include <functional>

Matrix::Matrix(Matrix *m) : height(m->height), width(m->width), data(m->data) {

}

Matrix::Matrix(int h, int w, float initValue) : height(h), width(w), data(w * h, initValue) {

}

Matrix::Matrix(int h, int w, float from, float to) : height(h), width(w) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(from, to);

    auto gen = [&dist]() { //lambda expression
        return dist(mt);
    };
    data.resize(w * h);
    generate(data.begin(), data.end(), gen);
}

Matrix::Matrix(int h, int w, std::vector<float> d) : height(h), width(w), data(d) {
    if(w * h != d.size()) throw std::invalid_argument("A Matrix of dimensions (" + std::to_string(h) + " x " + std::to_string(w) + ") must have " + std::to_string(w * h) + " instead of " + std::to_string(d.size()) + " arguments.");
}

Matrix::Matrix() : width(1), height(1), data(1) {}

Matrix::~Matrix(){

}

size_t Matrix::getWidth() {
    return width;
}

size_t Matrix::getHeight() {
    return height;
}

float Matrix::getMax() {
    float max = data[0];
    for(float d : data) {
        if(d > max) max = d;
    }
    return max;
}

float Matrix::getMin() {
    float min = data[0];
    for(float d : data) {
        if(d < min) min = d;
    }
    return min;
}

std::ostream& operator<<(std::ostream &strm, const Matrix &m) {
    for(size_t y = 0; y < m.height; y++) {
        strm << "[";
        for(size_t x = 0; x < m.width - 1; x++) {
            strm << m.data[y * m.width + x] << ", ";
        }
        if(m.width != 0) strm << m.data[(y + 1) * m.width - 1];
        strm << "]" << std::endl;
    }
    return strm;
}

//########################### Matrix/Matrix operators ###########################

Matrix& Matrix::operator+=(const Matrix &rhs) {
    if(width != rhs.width || height != rhs.height) throw std::invalid_argument("Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" + std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    size_t s = width * height;
    for(size_t i = 0; i < s; i++) {
        data[i] += rhs.data[i];
    }
    return *this;
}

Matrix operator+(Matrix lhs, const Matrix& rhs) {
    lhs += rhs;
    return lhs;
}

Matrix& Matrix::operator-=(const Matrix &rhs) {
    if(width != rhs.width || height != rhs.height) throw std::invalid_argument("Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" + std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    size_t s = width * height;
    for(size_t i = 0; i < s; i++) {
        data[i] -= rhs.data[i];
    }
    return *this;
}

Matrix operator-(Matrix lhs, const Matrix& rhs) {
    lhs -= rhs;
    return lhs;
}

Matrix& Matrix::operator*=(const Matrix &rhs) {
    if(width != rhs.width || height != rhs.height) throw std::invalid_argument("Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" + std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    size_t s = width * height;
    for(size_t i = 0; i < s; i++) {
        data[i] *= rhs.data[i];
    }
    return *this;
}

Matrix operator*(Matrix lhs, const Matrix& rhs) {
    lhs *= rhs;
    return lhs;
}

Matrix& Matrix::operator/=(const Matrix &rhs) {
    if(width != rhs.width || height != rhs.height) throw std::invalid_argument("Matrix dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") and (" + std::to_string(rhs.height) + " x " + std::to_string(rhs.width) + ") do not match.");
    size_t s = width * height;
    for(size_t i = 0; i < s; i++) {
        data[i] /= rhs.data[i];
    }
    return *this;
}

Matrix operator/(Matrix lhs, const Matrix& rhs) {
    lhs /= rhs;
    return lhs;
}

//########################### Matrix/float operators ###########################

Matrix& Matrix::operator+=(float rhs) {
    size_t s = width * height;
    for(int i = 0; i < s; i++) {
        data[i] += rhs;
    }
    return *this;
}

Matrix operator+(Matrix lhs, float rhs) {
    lhs += rhs;
    return lhs;
}

Matrix& Matrix::operator-=(float rhs) {
    size_t s = width * height;
    for(int i = 0; i < s; i++) {
        data[i] -= rhs;
    }
    return *this;
}

Matrix operator-(Matrix lhs, float rhs) {
    lhs -= rhs;
    return lhs;
}

Matrix& Matrix::operator*=(float rhs) {
    size_t s = width * height;
    for(int i = 0; i < s; i++) {
        data[i] *= rhs;
    }
    return *this;
}

Matrix operator*(Matrix lhs, float rhs) {
    lhs *= rhs;
    return lhs;
}

Matrix& Matrix::operator/=(float rhs) {
    size_t s = width * height;
    for(int i = 0; i < s; i++) {
        data[i] /= rhs;
    }
    return *this;
}

Matrix operator/(Matrix lhs, float rhs) {
    lhs /= rhs;
    return lhs;
}
//########################### Matrix Subscript ###########################

float& Matrix::operator()(size_t y, size_t x) {
    if(x >= width || y >= height) throw std::invalid_argument("The indices (" + std::to_string(y) + ", " + std::to_string(x) + ") are invalid for a Matrix of size ("  + std::to_string(height) + " x " + std::to_string(width) + ").");
    return data[y * width + x];
}

float Matrix::operator()(size_t y, size_t x) const {
    if(x >= width || y >= height) throw std::invalid_argument("The indices (" + std::to_string(y) + ", " + std::to_string(x) + ") are invalid for a Matrix of size ("  + std::to_string(height) + " x " + std::to_string(width) + ").");
    return data[y * width + x];
}

//########################### Comparison ###########################

bool operator==(const Matrix &lhs, const Matrix &rhs) {
    if(lhs.width != rhs.width || lhs.height != rhs.height) return false;
    size_t s = lhs.width * lhs.height;
    for(size_t i = 0; i < s; i++) {
        if(!(fabs(lhs.data[i] - rhs.data[i]) < lhs.data[i] * 0.001)) return false;
    }
    return true;
}

Matrix Matrix::dotProduct(const Matrix &other) {
    if(width != other.height) throw std::invalid_argument("Width of the first Matrix with dimensions (" + std::to_string(height) + " x " + std::to_string(width) + ") must be equal to the height of the second Matrix with dimensions (" + std::to_string(other.height) + " x " + std::to_string(other.width) + ").");
    Matrix m(height, other.width);
    for(size_t x2 = 0; x2 < other.width; x2++) {
        for(size_t y1 = 0; y1 < height; y1++) {
            for(size_t x1 = 0; x1 < width; x1++) {
                m.data[y1 * m.width + x2] += data[y1 * width + x1] * other.data[x1 * other.width + x2];
            }
        }
    }
    return m;
}

Matrix& Matrix::apply(MatrixFunction fun) {
    size_t s = width * height;
    for(int i = 0; i < s; i++) {
        data[i] = fun(data[i]);
    }
    return *this;
}

Matrix Matrix::copy() {
    return Matrix(width, height, data);
}

float Matrix::toFloat() {
    if(width != 1 || height != 1) throw std::invalid_argument("Matrix must be of dimensions (1 x 1) instead of (" + std::to_string(height) + " x " + std::to_string(width) + ") to convert to float.");
    return data[0];
}

Matrix& Matrix::transpose() {
    if(width == height) {
        for(size_t x = 0; x < width - 1; x++) {
            for(size_t y = x + 1; y < width; y++) { // y = x + 1 -> always takes all values below the "middle line"
                //swap data[x, y] and data[y, x]
                float temp = data[y * width + x];
                data[y * width + x] = data[x * width + y];
                data[x * width + y] = temp;
            }
        }
    } else { //inefficient af, because I copy it instead of calculating permutation circles
        std::vector<float> dataNew(width * height);
        for(size_t y = 0; y < height; y++) {
	        for(size_t x = 0; x < width; x++) {
	        	dataNew[x * height + y] = data[y * width + x];
            }
        }
        data = dataNew;
        size_t w = width;
        width = height;
        height = w;
    }
    return *this;
}