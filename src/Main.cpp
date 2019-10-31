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
#include "Rng.h"

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
    Uint8 buffer = 0;
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
                    if (World::getMPIRank() != 0) {
                        std::cerr << "Keyboard input only works on ROOT node (0)!" << std::endl;
                        break;
                    }

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
                    if (World::getMPIRank() == 0) run = false;
                    else std::cerr << "Simulation must be quit on ROOT node (0)!" << std::endl;
                    break;

                default:
                    break;
            }
        }

        //###################### BROADCAST APPLICATION STATUS #####################
        buffer = 0;
        if (World::getMPIRank() == 0) {
            if (run) buffer |= 0x1u;
            if (paused) buffer |= 0x2u;
            if (similarityMode) buffer |= 0x4u;
            if (borders) buffer |= 0x8u;
        }
        MPI_Bcast(&buffer, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        if (World::getMPIRank() != 0) {
            run = (buffer & 0x1u) != 0;
            paused = (buffer & 0x2u) != 0;
            similarityMode = (buffer & 0x4u) != 0;
            borders = (buffer & 0x8u) != 0;
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
    std::random_device rd;
    unsigned int randomSeed = rd();

    // Scan program arguments
    int c;
    while ((c = getopt(argc, argv, "h::w::m::f::s::")) != -1) {
        switch (c) {
            // Height
            case 'h':
                if (optarg != nullptr) height = std::stoi(optarg, nullptr);
                break;

                // Width
            case 'w':
                if (optarg != nullptr) width = std::stoi(optarg, nullptr);
                break;

                // MaiMUC
            case 'm':
                maimuc = true;
                break;

            case 'f':
                if (optarg != nullptr) foodRate = strtod(optarg, nullptr);
                break;
                // Unknown Option
            case 's':
                if (optarg != nullptr) randomSeed = std::stoi(optarg, nullptr);
                break;
            case '?':
                if (optopt == 'h' || optopt == 'w') {
                    std::cerr << "Option -h and -w require an integer!" << std::endl;
                } else if (optopt == 'f') {
                    std::cerr
                            << "Option -f requires a float indicating the amount of food spawned per 2000 tiles per tick!"
                            << std::endl;
                } else {
                    std::cerr << "Unknown option character -" << (char) optopt << std::endl;
                }

                return EXIT_FAILURE;

            default:
                std::cerr << "Some error occurred!" << std::endl;
                return EXIT_FAILURE;
        }
    }
    rng.seed(randomSeed);

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
        auto *entity = new LivingEntity(
                getRandomIntBetween(0, dim.w) + dim.x,
                getRandomIntBetween(0, dim.h) + dim.y,
                {
                        static_cast<Uint8>(getRandomIntBetween(0, 256)),
                        static_cast<Uint8>(getRandomIntBetween(0, 256)),
                        static_cast<Uint8>(getRandomIntBetween(0, 256)),
                        255},
                (float) getRandomIntBetween(0, 10000) / 10000.0f,
                (float) getRandomIntBetween(0, 10000) / 10000.0f,
                (float) getRandomIntBetween(0, 10000) / 10000.0f, brain);

        World::addLivingEntity(entity, false);
    }
    for (int i = 0; i < 100; i++) {
        World::addFoodEntity(
                new FoodEntity(getRandomIntBetween(0, dim.w) + dim.x, getRandomIntBetween(0, dim.h) + dim.y, 8 * 60),
                false);
    }
    //=========================== END ADD TEST ENTITIES ===========================


    // Render-Event Loop
    renderLoop();

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}
