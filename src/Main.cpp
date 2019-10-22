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

    // Debug flags and other stuff
    bool paused = false, similarityMode = false, borders = false;
    std::vector<LivingEntity *> selectedEntities;
    WorldDim dim = World::getWorldDim();
    SDL_Texture *border = Renderer::renderRect(dim.w, dim.h, {255, 0, 0, 255}, false);
    SDL_Texture *pauseText = Renderer::renderFont("Paused", 25, {0, 0, 0, 255}, "font.ttf");

    //=============================================================================
    //                               BEGIN MAIN LOOP
    //=============================================================================
    SDL_Event e;
    int currentTime, elapsedTime, previousTime;
    bool run = true;
    while (true) {
        // Calculate lag between last and current turn
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        previousTime = currentTime;

        //############################# PROCESS INPUT #############################
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                // Key pressed?
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        // Pause/Play
                        case SDLK_p:
                            // Simulation is already paused in similarity mode!
                            if (!similarityMode) paused = !paused;
                            break;

                            // QUIT
                        case SDLK_q:
                            run = false;
                            break;

                            // Similarity mode
                        case SDLK_s:
                            similarityMode = paused = !similarityMode;
                            selectedEntities.clear();
                            break;

                            // Show borders of the world
                        case SDLK_b:
                            borders = !borders;
                            break;

                        default:
                            break;
                    }
                    break;

                    // Mouse clicked?
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        LivingEntity *nearest = World::findNearestLiving(dim.x + e.button.x, dim.y + e.button.y, -1);

                        if (similarityMode) {
                            if (nearest) selectedEntities.push_back(nearest);

                            // Two entities selected?
                            if (selectedEntities.size() == 2) {
                                std::cout << "Difference: " << selectedEntities[0]->difference(*selectedEntities[1])
                                          << std::endl;
                                selectedEntities.clear();
                            }
                        } else {
                            if (nearest) std::cout << *nearest << std::endl;
                            else std::cout << "No nearest entity available!" << std::endl;
                        }
                    }
                    break;

                    // QUIT
                case SDL_QUIT:
                    run = false;
                    break;

                default:
                    break;
            }
        }

        // Quit?
        if (!run) break;

        //############################ TICK AND RENDER ############################
        if (!paused) World::tick();

        // Render everything
        Renderer::clear();
        World::render();
        if (paused) Renderer::copy(pauseText, 10, 10);
        if (borders) Renderer::copy(border, 0, 0);
        Renderer::present();

        //########################### WAIT IF TOO FAST ############################
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        elapsedTime = (currentTime - previousTime);

        // Delay loop turn
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
    float foodRate = 1.f;  //food spawned per 2000 tiles per tick

    // Scan program arguments
    int c;
    while ((c = getopt(argc, argv, "h::w::m::f::")) != -1) {
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

            case 'f':
                if(optarg != nullptr) foodRate = atof(optarg);
                break;
                // Unknown Option
            case '?':
                if (optopt == 'h' || optopt == 'w') {
                    std::cerr << "Option -h and -w require an integer!" << std::endl;
                } else if(optopt == 'f') {
                    std::cerr << "Option -f requires a float indicating the amount of food spawned per 2000 tiles per tick!" << std::endl;
                } else {
                    std::cerr << "Unknown option character -" << (char) optopt << std::endl;
                }

                return EXIT_FAILURE;

            default:
                std::cerr << "Some error occurred!" << std::endl;
                return EXIT_FAILURE;
        }
    }

    // Init and set-up world & renderer
    World::setup(width, height, maimuc, foodRate);
    WorldDim dim = World::getWorldDim();
    if (maimuc)//TODO change back
        Renderer::setup(0, 0, dim.w, dim.h, true);
    else
        Renderer::setup(dim.x, dim.y, dim.w, dim.h, false);

    //============================= ADD TEST ENTITIES =============================
    for (int i = 0; i < 10; i++) {
        auto *brain = new Brain(6, 8, 4, 4, 10, 4);
        auto *entity = new LivingEntity((std::rand() % dim.w) + dim.x, (std::rand() % dim.h) + dim.y,
                                        {static_cast<Uint8>(std::rand()), static_cast<Uint8>(std::rand()),
                                         static_cast<Uint8>(std::rand()), 255},
                                        (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f,
                                        (rand() % 10000) / 10000.0f, brain);
        World::addLivingEntity(entity, false);
    }
    for (int i = 0; i < 100; i++) {
        World::addFoodEntity(new FoodEntity((std::rand() % dim.w) + dim.x, (std::rand() % dim.h) + dim.y, 8 * 60),
                             false);
    }
    //=========================== END ADD TEST ENTITIES ===========================


    // Render-Event Loop
    renderLoop();

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}
