#include <mpi.h>
#include <cfloat>
#include "World.h"

int *splitRect(int num, int width, int height);

int World::overallHeight = 0;
int World::overallWidth = 0;

World::World() : x(0), y(0), width(0), height(0), MPI_Rank(0), MPI_Nodes(0) {
    // Get MPI Rank and number of nodes
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_Nodes);

    // Get Width and Height of this World
    int *dim = splitRect(MPI_Nodes, World::overallWidth, World::overallHeight);
    width = dim[0];
    height = dim[1];

    // Get Position of this world (and update width and height if needed)
    int overlap;
    for (int i = 1; i <= MPI_Rank; i++) {
        x += width;

        // Height overlap? -> Last row
        if ((y + height) > World::overallHeight) {
            overlap = (y + height) - World::overallHeight;
            height -= overlap;
            width += (width * overlap) / height;
        }

        // Width overlap?
        if ((x + width) >= World::overallWidth) {
            if (i == MPI_Rank) {
                overlap = (x + width) - World::overallWidth;
                if (overlap == width) {
                    x = 0;
                    y += height;
                } else {
                    width -= overlap;
                }
            } else {
                x = -width;
                y += height;
            }
        }

        // Last rectangle?
        if ((i + 1) == MPI_Nodes)
            width = World::overallWidth - x;
    }
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
 * Splits a given rectangle (with width and heigth) into num smaller
 * rectangles that have around the same area as the given rectangle
 *
 * @param   num     Number of rectangles, the given rectangle has to be split
 * @param   width   The width of the given rectangle
 * @param   height  The height of the given rectangle
 *
 * @return  Array with the width (1st arg) and height (2nd arg) for the
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
