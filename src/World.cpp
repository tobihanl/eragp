#include <mpi.h>
#include <cfloat>
#include "World.h"

int *splitRect(int num, int width, int height);

// Init static attributes
int World::overallWidth = 0;
int World::overallHeight = 0;

int World::MPI_Rank = 0;
int World::MPI_Nodes = 0;

int World::x = 0;
int World::y = 0;
int World::width = 0;
int World::height = 0;

bool World::isSetup = false;

std::vector<FoodEntity> World::food = std::vector<FoodEntity>();
std::vector<LivingEntity> World::living = std::vector<LivingEntity>();

/**
 * Initialize the world, which is part of the overall world and set
 * it up.
 *
 * @param overallWidth Width of the overall world
 * @param overallHeight Height of the overall world
 */
void World::setup(int overallWidth, int overallHeight) {
    if (isSetup)
        return;

    // Set overall World size
    World::overallWidth = overallWidth;
    World::overallHeight = overallHeight;

    // Get MPI Rank and number of nodes
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_Nodes);

    // Get and set dimesions for this world
    WorldDim dim = calcWorldDimensions(MPI_Rank, MPI_Nodes);
    x = dim.x;
    y = dim.y;
    width = dim.w;
    height = dim.h;

    // World is setup
    isSetup = true;
}

void World::render() {
    //TODO render terrain
    for (auto &f : food) {
        f.render();
    }
    for (auto &e : living) {
        e.render();
    }
}

void World::tick() {
    for (auto &f : food) {
        f.tick();
    }
    for (auto &e : living) {
        e.tick();
    }
}

/**
 * Calculate dimensions (x & y position, width, height) of a world laying
 * on the node with the given MPI rank.
 *
 * @param rank Rank of the node, which world should be calculated
 * @param num Number of nodes in the MPI_COMM_WORLD
 *
 * @return dimensions of the world on the node with the given MPI rank
 */
WorldDim World::calcWorldDimensions(int rank, int num) {
    WorldDim dim;

    // Get Width and Height of the world
    int *rect = splitRect(num, overallWidth, overallHeight);
    dim.w = rect[0];
    dim.h = rect[1];

    // Get Position of the world (and update width and height if needed)
    int overlap;
    for (int i = 1; i <= MPI_Rank; i++) {
        dim.x += dim.w;

        // Height overlap? -> Last row
        if ((dim.y + dim.h) > overallHeight) {
            overlap = (dim.y + dim.h) - overallHeight;
            dim.h -= overlap;
            dim.w += (dim.w * overlap) / dim.h;
        }

        // Width overlap?
        if ((dim.x + dim.w) >= overallWidth) {
            if (i == rank) {
                overlap = (dim.x + dim.w) - overallWidth;
                if (overlap == dim.w) {
                    dim.x = 0;
                    dim.y += dim.h;
                } else {
                    dim.w -= overlap;
                }
            } else {
                dim.x = -dim.w;
                dim.y += dim.h;
            }
        }

        // Last rectangle?
        if ((i + 1) == num)
            dim.w = overallWidth - dim.x;
    }

    return dim;
}

/*
 * Returns itself if no other entities exist
 */
LivingEntity World::getNearestEntity(LivingEntity entity) {
    LivingEntity nearestEntity = living.front();
    double minDistance = DBL_MAX;

    for (auto &e : living) {
        double distance = entity.distanceToPoint(e.x, e.y);
        if (distance < minDistance && e != entity) {
            nearestEntity = e;
            minDistance = distance;
        }
    }
    return nearestEntity;
}

/**
 * Splits a given rectangle (with width and height) into num smaller
 * rectangles that have around the same area as the given rectangle
 *
 * @param   num     Number of rectangles, the given rectangle has to be split
 * @param   width   The width of the given rectangle
 * @param   height  The height of the given rectangle
 *
 * @return  Array with the width (1st index) and height (2nd index) for the
 *          resulting rectangle
 */
int *splitRect(int num, int width, int height) {
    static int dim[2] = {width, height};

    // Doesn't the rectangle have to be split?
    if (num < 2)
        return dim;

    // Split the rectangle (into two halves) as long as there are fewer rectangles then wanted
    int rects = 1;
    while (rects < num) {
        rects *= 2;
        if (width > height)
            width /= 2;
        else
            height /= 2;
    }

    // Split into too many rectangles?
    if (rects > num) {
        if (width > height)
            height += (height / num) * (rects - num);
        else
            width += (width / num) * (rects - num);
    }

    // Return width and height of one rectangle (~ 1/num of the original)
    dim[0] = width;
    dim[1] = height;
    return dim;
}
