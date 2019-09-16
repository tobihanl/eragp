//TODO to disable asserts in release: #define NDEBUG
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <SDL.h>
#include <mpi.h>
#include <unistd.h>
#include "Renderer.h"
#include "World.h"
#include "Brain.h"
#include "Tile.h"

#define MS_PER_TICK 100

/**
 * Event Loop that is also rendering and updating the world.
 *
 * @param world The world, which should be updated and rendered
 */
void renderLoop() {
    LivingEntity::digits[0] = Renderer::renderFont("0", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[1] = Renderer::renderFont("1", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[2] = Renderer::renderFont("2", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[3] = Renderer::renderFont("3", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[4] = Renderer::renderFont("4", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[5] = Renderer::renderFont("5", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[6] = Renderer::renderFont("6", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[7] = Renderer::renderFont("7", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[8] = Renderer::renderFont("8", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
    LivingEntity::digits[9] = Renderer::renderFont("9", ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");

    Tile::GRASS.texture = Renderer::renderImage("grass.png");
    Tile::STONE.texture = Renderer::renderImage("stone.png");
    Tile::SAND.texture = Renderer::renderImage("sand.png");
    Tile::WATER.texture = Renderer::renderImage("water.png");

    bool render;
    int lag = 0, currentTime, elapsedTime;
    int previousTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    //=============================================================================
    //                               BEGIN MAIN LOOP
    //=============================================================================
    SDL_Event e;
    bool run = true;
    while (true) {
        render = false;

        // Calculate lag between last and current turn
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        elapsedTime = (currentTime - previousTime);
        previousTime = currentTime;
        lag += elapsedTime;

        // Process input
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                run = false;
        }

        if (!run) break;

        // Calculate missing ticks
        while (lag >= MS_PER_TICK) {
            lag -= MS_PER_TICK;

            render = true;
            World::tick();
        }

        // Render if needed
        if (render) {
            // Render everything
            Renderer::clear();
            World::render();
            Renderer::present();
        }

        // Calculate elapsed time of this loop turn
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        elapsedTime = (currentTime - previousTime);

        // Wait if loop is too fast
        if (elapsedTime <= MS_PER_TICK)
            SDL_Delay(MS_PER_TICK - elapsedTime);
    }
    //=============================================================================
    //                                END MAIN LOOP
    //=============================================================================

    // Destroy renderer (close window) and exit
    Renderer::destroy();
}

/**
 * =============================================================================
 *                                 MAIN FUNCTION
 * =============================================================================
 */
int main(int argc, char **argv) {
    // START MPI
    MPI_Init(&argc, &argv);

    int width = 960, height = 720;
    bool maimuc = false;

    // Scan program arguments
    int c;
    while ((c = getopt(argc, argv, "h::w::m::")) != -1) {
        switch (c) {
            // Height
            case 'h':
                if (optarg != nullptr) height = strtol(optarg, nullptr, 10);
                break;

                // Width
            case 'w':
                if (optarg != nullptr) width = strtol(optarg, nullptr, 10);
                break;

                // MaiMUC
            case 'm':
                maimuc = true;
                break;

                // Unknown Option
            case '?':
                if (optopt == 'h' || optopt == 'w')
                    std::cerr << "Option -h and -w require an integer!" << std::endl;
                else
                    std::cerr << "Unknown option character -" << (char) optopt << std::endl;

                return EXIT_FAILURE;

            default:
                std::cerr << "Some error occurred!" << std::endl;
                return EXIT_FAILURE;
        }
    }

    // Init and set-up world & renderer
    World::setup(width, height, maimuc);
    WorldDim dim = World::getWorldDim();
    Renderer::setup(dim.x, dim.y, dim.w, dim.h);

    //============================= ADD TEST ENTITIES =============================
    for (int i = 0; i < 100; i++) {
        auto *brain = new Brain(6, 8, 4, 4, 10, 4);
        auto *entity = new LivingEntity(std::rand() % dim.w, std::rand() % dim.h,
                                        {static_cast<Uint8>(std::rand()), static_cast<Uint8>(std::rand()),
                                         static_cast<Uint8>(std::rand()), 255},
                                        (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f,
                                        (rand() % 10000) / 10000.0f, brain);
        World::addLivingEntity(entity);
    }
    for (int i = 0; i < 100; i++) {
        World::addFoodEntity(new FoodEntity(std::rand() % dim.w, std::rand() % dim.h, 8 * 60));
    }
    //=========================== END ADD TEST ENTITIES ===========================


    // Render-Event Loop
    renderLoop();

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}