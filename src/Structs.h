#ifndef EVOLUTION_STRUCTS_H
#define EVOLUTION_STRUCTS_H

#include <cstdint>

struct Point {
    int x = 0;
    int y = 0;
};

struct Rect {
    struct Point p;
    int w = 0;
    int h = 0;
};

struct PaddingRect {
    int rank = 0;
    struct Rect rect;
};

typedef struct Rect WorldDim;

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct RenderData {
    WorldDim worldDim;
    int radius = 0;
    Color color = {0};
    int x = 0;
    int y = 0;
    int energy = 0;
};

typedef struct {
    int turn = -1;
    int food = -1;
    int livings = -1;
    int mpi = -1;
    int tick = -1;
    int render = -1;
    int delay = -1;
    int overall = -1;
} LogData;

#endif //EVOLUTION_STRUCTS_H
