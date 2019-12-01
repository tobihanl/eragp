//TODO to disable asserts in release: #define NDEBUG
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <poll.h>
#include <SDL.h>
#include <mpi.h>
#include <unistd.h>
#include "Renderer.h"
#include "World.h"
#include "Brain.h"
#include "Tile.h"
#include "Log.h"
#include "Rng.h"

#define MS_PER_TICK 100

// Init class Log attributes
bool Log::logging = false;
FILE *Log::logFile = nullptr;
LogData Log::data = {};

void preRender(SDL_Texture **border, SDL_Texture **pauseText, SDL_Texture **padding);

/**
 * Event Loop that is also rendering and updating the world.
 *
 * @param   ticks   Amount of ticks to calculate
 *                  (-1) = no limitation
 */
void renderLoop(long ticks) {
    // Pre-render Textures for faster copying
    SDL_Texture *border = nullptr, *pauseText = nullptr, *padding = nullptr;
    preRender(&border, &pauseText, &padding);

    // Debug flags and other stuff
    Uint8 buffer = 0;
    bool paused = false, similarityMode = false, borders = false, paddings = false;
    int countSelectedEntities = 0;
    LivingEntity *selectedEntities[2] = {nullptr};
    WorldDim dim = World::getWorldDim();

    // Frame rate
    SDL_Texture *fps = nullptr;
    SDL_Rect fpsRect = {0, 10, 0, 0}; // Always 10px padding to the top
    int frameTime = 0, frames = 0;

    //=============================================================================
    //                               BEGIN MAIN LOOP
    //=============================================================================
    SDL_Event e;
    int currentTime, elapsedTime;
    int previousTime = Log::currentTime();
    bool run = ticks == -1 || ticks > 0;
    int counter = 0;
    while (run) {
        currentTime = Log::currentTime();
        frameTime += currentTime - previousTime;
        previousTime = currentTime;

        // Log new tick
        Log::data.turn = counter++;
        Log::data.food = World::getAmountOfFood();
        Log::data.livings = World::getAmountOfLivings();

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
                            selectedEntities[0] = selectedEntities[1] = nullptr;
                            countSelectedEntities = 0;
                            break;

                            // Show borders of the world
                        case SDLK_b:
                            borders = !borders;
                            break;

                            // Show padding areas
                        case SDLK_a:
                            paddings = !paddings;
                            break;

                        default:
                            break;
                    }
                    break;

                    // Mouse clicked?
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        LivingEntity *nearest = World::findNearestLivingToPoint(dim.p.x + e.button.x,
                                                                                dim.p.y + e.button.y);

                        if (similarityMode) {
                            if (nearest) selectedEntities[countSelectedEntities++] = nearest;

                            // Two entities selected?
                            if (countSelectedEntities >= 2) {
                                std::cout << "Squared difference: " << (*selectedEntities[0]).squaredDifference(
                                        *selectedEntities[1])
                                          << std::endl;
                                selectedEntities[0] = selectedEntities[1] = nullptr;
                                countSelectedEntities = 0;
                            }
                        } else {
                            if (nearest) {
                                std::cout << *nearest << std::endl;
                                nearest->brain->printThink = true;
                            }
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
            if (paddings) buffer |= 0x10u;
        }
        MPI_Bcast(&buffer, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        if (World::getMPIRank() != 0) {
            run = (buffer & 0x1u) != 0;
            paused = (buffer & 0x2u) != 0;
            similarityMode = (buffer & 0x4u) != 0;
            borders = (buffer & 0x8u) != 0;
            paddings = (buffer & 0x10u) != 0;
        }

        // Quit?
        if (!run) break;

        // Calc FPS
        if (frameTime >= 1000) {
            Renderer::cleanup(fps);

            frameTime = 0;
            fps = Renderer::renderFont(std::to_string(frames), 25, {255, 255, 255, 255}, "font.ttf");
            frames = 0;

            // 10px padding to the right
            Renderer::query(fps, &fpsRect);
            fpsRect.x = dim.w - fpsRect.w - 10;
        }

        //############################ TICK AND RENDER ############################
        Renderer::setTarget(World::entities);
        Renderer::clear();
        if (!paused) {
            int tickTime = Log::currentTime();
            World::tick();
            Log::data.tick = Log::endTime(tickTime);
        }
        Renderer::setTarget(nullptr);

        // Render everything
        int renderTime = Log::currentTime();
        Renderer::clear();
        World::render();
        if (paddings) Renderer::copy(padding, 0, 0);
        if (borders) Renderer::copy(border, 0, 0);
        if (paused) Renderer::copy(pauseText, 10, 10);
        Renderer::copy(fps, &fpsRect);
        Renderer::present();
        frames++;
        Log::data.render = Log::endTime(renderTime);

        //########################### WAIT IF TOO FAST ############################
        currentTime = Log::currentTime();
        elapsedTime = (currentTime - previousTime);

        // Delay loop turn
        if (elapsedTime <= MS_PER_TICK) {
            SDL_Delay(MS_PER_TICK - elapsedTime);
            Log::data.delay = MS_PER_TICK - elapsedTime;
        }

        // Set running status for next loop turn
        run = ticks == -1 || (--ticks) > 0;

        Log::data.overall = Log::endTime(previousTime);
        Log::writeLogData();
    }
    //=============================================================================
    //                                END MAIN LOOP
    //=============================================================================

    // Cleanup digits
    for (int i = 0; i < 10; i++)
        Renderer::cleanup(LivingEntity::digits[i]);

    // Destroy renderer (close window) and exit
    Renderer::cleanup(Tile::GRASS.texture);
    Renderer::cleanup(Tile::STONE.texture);
    Renderer::cleanup(Tile::SAND.texture);
    Renderer::cleanup(Tile::WATER.texture);
    Renderer::cleanup(border);
    Renderer::cleanup(padding);
    Renderer::cleanup(pauseText);
    Renderer::cleanup(fps);
    Renderer::destroy();
}

/*
 * Normal loop updating the world without rendering it.
 *
* @param    ticks   Amount of ticks to calculate
 *                  (-1) = no limitation
 */
void normalLoop(long ticks) {
    // Inspired from http://www.coldestgame.com/site/blog/cybertron/non-blocking-reading-stdin-c
    pollfd cin[1];
    if (World::getMPIRank() == 0) {
        cin[0].fd = fileno(stdin);
        cin[0].events = POLLIN;
        std::cout << "> ";
        std::cout.flush();
    }

    Uint8 buffer = 0;
    bool run = ticks == -1 || ticks > 0;
    int counter = 0;
    while (run) {
        // Log new tick
        int loopTime = Log::currentTime();
        Log::data.turn = counter++;
        Log::data.food = World::getAmountOfFood();
        Log::data.livings = World::getAmountOfLivings();

        //############################# PROCESS INPUT #############################
        if (World::getMPIRank() == 0 && poll(cin, 1, 1)) {
            std::string str;
            std::cin >> str;

            // Switch command
            switch (str.front()) {
                // Quit
                case 'q':
                case 'Q':
                    run = false;
                    break;

                default:
                    break;
            }

            if (run) {
                std::cout << "> ";
                std::cout.flush();
            }
        }

        //###################### BROADCAST APPLICATION STATUS #####################
        buffer = 0;
        if (World::getMPIRank() == 0) {
            if (run) buffer |= 0x1u;
        }
        MPI_Bcast(&buffer, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        if (World::getMPIRank() != 0) {
            run = (buffer & 0x1u) != 0;
        }

        // Quit?
        if (!run) break;

        //################################## TICK #################################
        int tickTime = Log::currentTime();
        World::tick();
        Log::data.tick = Log::endTime(tickTime);

        // Set running status for next loop turn
        run = ticks == -1 || (--ticks) > 0;

        Log::data.overall = Log::endTime(loopTime);
        Log::writeLogData();
    }
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
    bool maimuc = false, render = false;
    float foodRate = 1.f;  //food spawned per 2000 tiles per tick
    float zoom = 1.f;
    long ticks = -1;
    long livings = 50, food = 100;
    std::string filename;

    std::random_device rd;
    unsigned int randomSeed = rd();

    // Scan program arguments
    opterr = 0;
    int c;
    while ((c = getopt(argc, argv, "h:w:m::f:r::t:s:l:e:z:")) != -1) {
        char *ptr = nullptr;
        switch (c) {
            // Render flag
            case 'r':
                render = true;
                break;

                // MaiMUC flag
            case 'm':
                maimuc = true;
                break;

                // Height
            case 'h':
                if (optarg) {
                    // TODO: strtol -> std::stoi?
                    height = (int) strtol(optarg, &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -h requires an integer!" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

                // Width
            case 'w':
                if (optarg) {
                    // TODO: strtol -> std::stoi?
                    width = (int) strtol(optarg, &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -w requires an integer!" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

                // Amount of ticks for simulation
            case 't':
                if (optarg) {
                    // TODO: strtol -> std::stoi?
                    ticks = strtol(optarg, &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -t requires an integer!" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

            case 'f':
                if (optarg) {
                    foodRate = (float) strtod(optarg, &ptr);
                    if (*ptr) {
                        std::cerr << "Option -f requires a float indicating the amount of food spawned per 2000 tiles"
                                  << "per tick!" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

            case 's':
                if (optarg != nullptr) randomSeed = std::stoi(optarg, nullptr);
                break;

                // Zoom
            case 'z':
                if (optarg) {
                    zoom = (float) strtod(optarg, &ptr);
                    if (*ptr || zoom < 0.1f) {
                        std::cerr << "Option -z requires a (positive, >=0.1) float for the zoom of the world!"
                                  << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

                // Log to file
            case 'l':
                if (optarg != nullptr) {
                    if (optarg[0] == '-') {
                        std::cerr << "Option -l requires a string (path to file) specifying the logging location!"
                                  << std::endl;
                        return EXIT_FAILURE;
                    }
                    filename = optarg;
                }
                break;

                // Amount of entities
            case 'e':
                if (optarg != nullptr) {
                    if (optarg[0] == '-') {
                        std::cerr
                                << "Option -e requires a string specifying the amount of livings and food to spawn on the"
                                << "entire world. Format: {Livings},{Food}" << std::endl;
                        return EXIT_FAILURE;
                    }
                    std::istringstream opts(optarg);
                    std::string l, f;
                    getline(opts, l, ',');
                    getline(opts, f, ',');
                    livings = strtol(l.c_str(), &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -e requires two integers delimited by a comma!" << std::endl;
                        return EXIT_FAILURE;
                    }
                    food = strtol(f.c_str(), &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -e requires two integers delimited by a comma!" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                break;

                // Unknown Option
            case '?':
                if (optopt == 'h' || optopt == 'w' || optopt == 't') {
                    std::cerr << "Option -" << (char) optopt << " requires an integer!" << std::endl;
                } else if (optopt == 'f') {
                    std::cerr << "Option -f requires a float indicating the amount of food spawned per 2000 tiles "
                              << "per tick!" << std::endl;
                } else if (optopt == 'l') {
                    std::cerr << "Option -l requires a string (path to file) specifying the logging location!"
                              << std::endl;
                } else if (optopt == 'e') {
                    std::cerr << "Option -e requires a string specifying the amount of livings and food to spawn on the"
                              << "entire world. Format: {Livings},{Food}" << std::endl;
                } else if (optopt == 'z') {
                    std::cerr << "Option -z requires a (positive, >=0.1) float for the zoom of the world!" << std::endl;
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

    // Init world
    World::setup(width, height, maimuc, foodRate, zoom);
    WorldDim dim = World::getWorldDim();
    if (!filename.empty())
        Log::startLogging(filename + "-" + std::to_string(World::getMPIRank()) + ".csv");

    // Init renderer
    if (render) {
        if (maimuc)
            Renderer::setup(0, 0, dim.w, dim.h, true);
        else
            Renderer::setup(dim.p.x, dim.p.y, dim.w, dim.h, false);
    }

    //============================= ADD TEST ENTITIES =============================
    long max = livings / World::getMPINodes();
    if (World::getMPIRank() >= World::getMPINodes() - livings % World::getMPINodes()) max++;
    for (long i = 0; i < max; i++) {
        auto *brain = new Brain(6, 8, 4, 4, 10, 4);
        auto *entity = new LivingEntity(
                getRandomIntBetween(0, dim.w) + dim.p.x,
                getRandomIntBetween(0, dim.h) + dim.p.y,
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
    max = food / World::getMPINodes();
    if (World::getMPIRank() >= World::getMPINodes() - food % World::getMPINodes()) max++;
    for (long i = 0; i < max; i++) {
        World::addFoodEntity(
                new FoodEntity(
                        getRandomIntBetween(0, dim.w) + dim.p.x,
                        getRandomIntBetween(0, dim.h) + dim.p.y,
                        8 * 60),
                false);
    }
    //=========================== END ADD TEST ENTITIES ===========================

    // Render simulation?
    if (render)
        renderLoop(ticks);
    else
        normalLoop(ticks);

    // Exit application
    if (World::getMPIRank() == 0)
        std::cout << "EXITING..." << std::endl;

    World::finalize();
    Log::endLogging();

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}

void preRender(SDL_Texture **border, SDL_Texture **pauseText, SDL_Texture **padding) {
    WorldDim dim = World::getWorldDim();

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

    *border = Renderer::renderRect(dim.w, dim.h, {255, 0, 0, 255}, false);
    *pauseText = Renderer::renderFont("Paused", 25, {0, 0, 0, 255}, "font.ttf");

    // Render padding Rects
    *padding = Renderer::createTexture(dim.w, dim.h, SDL_TEXTUREACCESS_TARGET);
    Renderer::setTarget(*padding);
    SDL_SetTextureBlendMode(*padding, SDL_BLENDMODE_BLEND);
    Renderer::clear();
    for (const auto &p : *World::getPaddingRects())
        Renderer::copy(Renderer::renderRect(p.rect.w, p.rect.h, {0, 0, 255, 255}, false),
                       p.rect.p.x - dim.p.x,
                       p.rect.p.y - dim.p.y);
    Renderer::setTarget(nullptr);

    // Render terrain and create texture for entities
    World::background = World::renderTerrain();
    World::entities = Renderer::createTexture(dim.w, dim.h, SDL_TEXTUREACCESS_TARGET);
    SDL_SetTextureBlendMode(World::entities, SDL_BLENDMODE_BLEND);
    World::rankTexture = Renderer::renderFont(std::to_string(World::getMPIRank()), 25, {255, 255, 255, 255},
                                              "font.ttf");
}
