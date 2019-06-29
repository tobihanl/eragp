#include <iostream>
#include <chrono>
#include <SDL.h>
#include <mpi.h>
#include <unistd.h>
#include "Renderer.h"
#include "World.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#define MS_PER_TICK 100

/**
 * Event Loop that is also rendering and updating the world.
 *
 * @param world The world, which should be updated and rendered
 */
void renderLoop(World &world) {
    Renderer::setup(WINDOW_WIDTH, WINDOW_HEIGHT);

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
            world.tick();
        }

        // Render if needed
        if (render) {
            // Render everything
            Renderer::clear();
            world.render();
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

    // Scan program arguments
    int c;
    while ((c = getopt(argc, argv, "h:w:")) != -1) {
        switch (c) {
            case 'h':
                World::overallHeight = strtol(optarg, nullptr, 10);
                break;

            case 'w':
                World::overallWidth = strtol(optarg, nullptr, 10);
                break;

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

    // Instantiate a World object
    World world;

    // Render-Event Loop
    renderLoop(world);

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}