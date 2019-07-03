//TODO to disable asserts in release: #define NDEBUG
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <SDL.h>
#include "Renderer.h"
#include "World.h"
#include "Brain.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#define MS_PER_TICK 100

int main() {
    Renderer::setup(WINDOW_WIDTH, WINDOW_HEIGHT);
    //============================= ADD TEST ENTITIES =============================
    for(int i = 0; i < 20; i++) {
        int layers[3] = {3, 4, 10};
        Brain *brain = new Brain(3, layers);
        LivingEntity* entity = new LivingEntity(std::rand() % WINDOW_WIDTH, std::rand() % WINDOW_HEIGHT, {std::rand() % 256, std::rand() % 256, std::rand() % 256, 0}, (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f, brain);
        World::addLivingEntity(entity);
    }
    std::cout << "finished init" << std::endl;//TODO remove
    //=========================== END ADD TEST ENTITIES ===========================
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
    return 0;
}
