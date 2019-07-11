//TODO to disable asserts in release: #define NDEBUG
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <SDL.h>
#include "Renderer.h"
#include "World.h"
#include "Brain.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#define MS_PER_TICK 100

int main() {
    Renderer::setup(WINDOW_WIDTH, WINDOW_HEIGHT);
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

    //============================= ADD TEST ENTITIES =============================
    for(int i = 0; i < 100; i++) {
        int layers[3] = {3, 4, 10};
        auto *brain = new Brain(3, layers);
        auto *entity = new LivingEntity(std::rand() % WORLD_WIDTH, std::rand() % WORLD_HEIGHT,
                                        {static_cast<Uint8>(std::rand()), static_cast<Uint8>(std::rand()),
                                         static_cast<Uint8>(std::rand()), 0},
                                                (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f, brain);
        World::addLivingEntity(entity);
    }
    for(int i = 0; i < 250; i++) {
        World::addFoodEntity(new FoodEntity(std::rand() % WORLD_WIDTH, std::rand() % WORLD_HEIGHT, 8 * 60));
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
