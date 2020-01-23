#ifndef EVOLUTION_CONSTANTS_H
#define EVOLUTION_CONSTANTS_H

#define NUMBER_OF_MAIMUC_NODES 10

#define TILE_SIZE 8
#define CHUNK_SIZE 100

#define VIEW_RANGE (20 * TILE_SIZE) // 160, should be a multiple of TILE_SIZE!
#define VIEW_RANGE_SQUARED (VIEW_RANGE * VIEW_RANGE)

#define WORLD_PADDING VIEW_RANGE // must be a multiple of TILE_SIZE!

#define ENEMY_MATE_SQUARED_DIFFERENCE_THRESHOLD 0.0016

#define MAX_FOOD_INTERVAL 1000000 //Can be much bigger because it is equally distributed

#define MSGS_PER_NEIGHBOR 3

#define FOOD_BUFFER_TICKS_CAPACITY 100

#define MPI_TAG_LIVING_ENTITY 42
#define MPI_TAG_FOOD_ENTITY 50
#define MPI_TAG_REMOVED_FOOD_ENTITY 51

#define ENERGY_FONT_SIZE 12

#define LOG_DATA_BUFFER_SIZE 100
#define LOG_TICKS_2_PAUSE 10

#endif //EVOLUTION_CONSTANTS_H
