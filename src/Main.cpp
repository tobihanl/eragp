#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <SDL.h>
#include "Renderer.h"
#include "World.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#define WORLD_WIDTH 120 // 960/8 TODO remove fixed value
#define WORLD_HEIGH 90 // 720/8
#define MS_PER_TICK 100

int main() {
    Renderer::setup(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Instantiate a World object
    auto *world = new World();

    //============================= ADD TEST ENTITIES =============================
    for(int i = 0; i < 20; i++) {
        LivingEntity* entity = new LivingEntity(std::rand() % WINDOW_WIDTH, std::rand() % WINDOW_HEIGHT, {std::rand() % 256, std::rand() % 256, std::rand() % 256, 0}, (rand() % 10000) / 10000.0f);
        world->addLivingEntity(entity);
    }
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
            world->tick();
        }

        // Render if needed
        if (render) {
            // Render everything
            Renderer::clear();
            world->render();
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
    delete world;
    return 0;
}
