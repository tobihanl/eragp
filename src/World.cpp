#include <mpi.h>
#include "World.h"

int World::overallHeight = 0;
int World::overallWidth = 0;

World::World() : x(0), y(0), width(0), height(0), MPI_Rank(0), MPI_Nodes(0) {
    // Get MPI Rank and number of nodes
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_Nodes);

    // Calculate strip (vertical/horizontal) for the world on this node
    if (overallWidth > overallHeight) {
        width = World::overallWidth / MPI_Nodes;
        height = World::overallHeight;
        x = MPI_Rank * width;
        y = 0;

        // Last node?
        if ((MPI_Rank + 1 == MPI_Nodes)) width = World::overallWidth - x;
    } else {
        height = World::overallHeight / MPI_Nodes;
        width = World::overallWidth;
        y = MPI_Rank * height;
        x = 0;

        // Last node?
        if ((MPI_Rank + 1) == MPI_Nodes) height = World::overallHeight - y;
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