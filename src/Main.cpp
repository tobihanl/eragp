#include <sstream>
#include <iostream>
#include <cstdlib>
#include <poll.h>
#include <mpi.h>
#include <unistd.h>
#include "World.h"
#include "Brain.h"
#include "Tile.h"
#include "Log.h"
#include "Rng.h"

#ifdef RENDER

#include <SDL.h>
#include "Renderer.h"

#endif

#define MS_PER_TICK 100

// Init class Log attributes
bool Log::logging = false;
FILE *Log::logFile = nullptr;
LogData Log::data = {};

#ifdef RENDER

void preRender(SDL_Texture **border, SDL_Texture **pauseText, SDL_Texture **padding) {
    WorldDim dim = World::getWorldDim();

    Renderer::renderDigits();

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
    Renderer::renderBackground(World::getWorldDim(), World::terrain);
    Renderer::createEntitiesTexture(World::getWorldDim());
    Renderer::renderRank(World::getMPIRank());
}

#endif

long ticks = -1;
bool render = false;
bool quit = false;

/**
 * Event Loop that is also rendering and updating the world.
 *
 * @param   ticks   Amount of ticks to calculate
 *                  (-1) = no limitation
 */
#ifdef RENDER

void renderLoop() {
    Renderer::show();

    // Pre-render Textures for faster copying
    SDL_Texture *border = nullptr, *pauseText = nullptr, *padding = nullptr;
    preRender(&border, &pauseText, &padding);

    // Debug flags and other stuff
    uint8_t buffer = 0;
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
    bool run = true;
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
                            quit = true;
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

                            // Hide rendering
                        case SDLK_h:
                            run = false;
                            render = false;
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
                            } else {
                                std::cout << "No nearest entity available!" << std::endl;
                            }
                        }
                    }
                    break;

                    // QUIT
                case SDL_QUIT:
                    if (World::getMPIRank() == 0) {
                        run = false;
                        quit = true;
                    } else {
                        std::cerr << "Simulation must be quit on ROOT node (0)!" << std::endl;
                    }
                    break;

                default:
                    break;
            }
        }

        if (ticks == 0) {
            quit = true;
            run = false;
        } else if (ticks > 0) {
            ticks--;
        }
        //###################### BROADCAST APPLICATION STATUS #####################
        buffer = 0;
        if (World::getMPIRank() == 0) {
            if (run) buffer |= 0x1u;
            if (paused) buffer |= 0x2u;
            if (similarityMode) buffer |= 0x4u;
            if (borders) buffer |= 0x8u;
            if (paddings) buffer |= 0x10u;
            if (quit) buffer |= 0x20u;
            if (render) buffer |= 0x40u;
        }
        MPI_Bcast(&buffer, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        if (World::getMPIRank() != 0) {
            run = (buffer & 0x1u) != 0;
            paused = (buffer & 0x2u) != 0;
            similarityMode = (buffer & 0x4u) != 0;
            borders = (buffer & 0x8u) != 0;
            paddings = (buffer & 0x10u) != 0;
            quit = (buffer & 0x20u) != 0;
            render = (buffer & 0x40u) != 0;
        }

        // Quit?
        if (!run) break;

        // Calc FPS
        if (frameTime >= 1000) {
            Renderer::cleanupTexture(fps);

            frameTime = 0;
            fps = Renderer::renderFont(std::to_string(frames), 25, {255, 255, 255, 255}, "font.ttf");
            frames = 0;

            // 10px padding to the right
            Renderer::query(fps, &fpsRect);
            fpsRect.x = dim.w - fpsRect.w - 10;
        }

        //############################ TICK AND RENDER ############################
        if (!paused) {
            Renderer::setTarget(Renderer::entities);
            Renderer::clear();

            int tickTime = Log::currentTime();
            World::tick();
            Log::data.tick = Log::endTime(tickTime);

            Renderer::setTarget(nullptr);
        }

        // Render everything
        int renderTime = Log::currentTime();
        Renderer::clear();

        Renderer::drawBackground(World::getWorldDim());
        if (!paused) Renderer::renderEntities(World::food, World::living);
        Renderer::copy(Renderer::entities, 0, 0);
        Renderer::drawRank();

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

        Log::data.overall = Log::endTime(previousTime);
        Log::writeLogData();
    }
    //=============================================================================
    //                                END MAIN LOOP
    //=============================================================================
    Renderer::cleanup();
    Renderer::cleanupTexture(border);
    Renderer::cleanupTexture(padding);
    Renderer::cleanupTexture(pauseText);
    Renderer::cleanupTexture(fps);

    Renderer::hide();
}

#endif

/*
 * Normal loop updating the world without rendering it.
 *
* @param    ticks   Amount of ticks to calculate
 *                  (-1) = no limitation
 */
void normalLoop() {
    // Inspired from http://www.coldestgame.com/site/blog/cybertron/non-blocking-reading-stdin-c
    pollfd cin[1];
    if (World::getMPIRank() == 0) {
        cin[0].fd = fileno(stdin);
        cin[0].events = POLLIN;
        std::cout << "> ";
        std::cout.flush();
    }

    uint8_t buffer = 0;
    bool run = true;
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
                    quit = true;
                    break;

#ifdef RENDER
                case 'r':
                case 'R':
                    run = false;
                    render = true;
                    break;
#endif

                default:
                    break;
            }

            if (run) {
                std::cout << "> ";
                std::cout.flush();
            }
        }

        if (ticks == 0) {
            quit = true;
            run = false;
        } else if (ticks > 0) {
            ticks--;
        }
        //###################### BROADCAST APPLICATION STATUS #####################
        buffer = 0;
        if (World::getMPIRank() == 0) {
            if (run) buffer |= 0x1u;
            if (quit) buffer |= 0x2u;
            if (render) buffer |= 0x4u;
        }
        MPI_Bcast(&buffer, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        if (World::getMPIRank() != 0) {
            run = (buffer & 0x1u) != 0;
            quit = (buffer & 0x2u) != 0;
            render = (buffer & 0x4u) != 0;
        }

        // Quit?
        if (!run) break;

        //################################## TICK #################################
        int tickTime = Log::currentTime();
        World::tick();
        Log::data.tick = Log::endTime(tickTime);


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
    bool maimuc = false;
    float foodRate = 1.f;  //food spawned per 2000 tiles per tick
    float zoom = 1.f;
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

#ifdef RENDER
    if (maimuc)
        Renderer::setup(0, 0, dim.w, dim.h, true);
    else
        Renderer::setup(dim.p.x, dim.p.y, dim.w, dim.h, false);
#endif

    //============================= ADD TEST ENTITIES =============================
    long max = livings / World::getMPINodes();
    if (World::getMPIRank() >= World::getMPINodes() - livings % World::getMPINodes()) max++;
    for (long i = 0; i < max; i++) {
        auto *brain = new Brain(6, 8, 4, 4, 10, 4);
        auto *entity = new LivingEntity(
                getRandomIntBetween(0, dim.w) + dim.p.x,
                getRandomIntBetween(0, dim.h) + dim.p.y,
                {
                        static_cast<uint8_t>(getRandomIntBetween(0, 256)),
                        static_cast<uint8_t>(getRandomIntBetween(0, 256)),
                        static_cast<uint8_t>(getRandomIntBetween(0, 256)),
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

    while (!quit) {
#ifdef RENDER
        if (render) renderLoop();
        else normalLoop();
#else
        normalLoop();
#endif
    }

    if (World::getMPIRank() == 0)
        std::cout << "EXITING..." << std::endl;

    World::finalize();

#ifdef RENDER
    Renderer::destroy();
#endif

    Log::endLogging();

    // END MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}
