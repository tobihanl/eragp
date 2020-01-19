#include <sstream>
#include <iostream>
#include <cstdlib>
#include <poll.h>
#include <mpi.h>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <random>
#include <fstream>
#include <cassert>
#include "World.h"
#include "Brain.h"
#include "Tile.h"
#include "Log.h"
#include "Lfsr.h"

#ifdef RENDER

#include <SDL.h>
#include "Renderer.h"

#endif

// Init class Log attributes
bool Log::logging = false;
FILE *Log::logFile = nullptr;
int Log::occupied = 0;
LogData Log::buffer[] = {};
LogData Log::data = {};
bool Log::paused = true;

#ifdef RENDER

/**
 * =============================================================================
 *                                 PRE-RENDERING
 * =============================================================================
 */
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
    Renderer::renderBackground(World::getWorldDim(), World::terrain, World::getMPIRank());
    Renderer::createEntitiesTexture(World::getWorldDim());
    Renderer::renderRank(World::getMPIRank());
}

#endif

// Global render-variables
bool borders = false;
bool paddings = false;
bool paused = false;
bool similarityMode = false;
int countSelectedEntities = 0;
LivingEntity *selectedEntities[2] = {nullptr, nullptr};

// Global control-variables
int minTickPeriod = 100;
int counter = 0;
long ticks = -1;
bool render = false;
bool quit = false;
bool run = true;
bool automated = false;

/**
 * =============================================================================
 *                              COMMAND-LINE THREAD
 * =============================================================================
 */
pthread_mutex_t cmdMutex = PTHREAD_MUTEX_INITIALIZER;
bool threadSuccessfullyCreated = true;
bool cancelThread = false;

void *commandLineThread(void *args) {
    pollfd pfds[1];

    pfds[0].fd = fileno(stdin);
    pfds[0].events = POLLIN;

    //############################# PROCESS INPUT #############################
    while (true) {
        std::cout << ((render) ? "[RENDER] > " : "[HIDDEN] > ");
        std::cout.flush();

        while (!cancelThread && poll(pfds, 1, 1000) == 0);
        if (cancelThread) break;

        std::string str;
        std::cin >> str;

        // Switch command
        pthread_mutex_lock(&cmdMutex);
        switch (str.front()) {
            // Quit
            case 'q':
            case 'Q':
                run = false;
                quit = true;
                break;

#ifdef RENDER
            // Render (Show)
        case 'r':
        case 'R':
            if (!render) {
                run = false;
                render = true;
                std::cout << "==[ Switch to RENDER mode ]==" << std::endl;
            } else {
                std::cerr << "** Only allowed in HIDDEN mode **" << std::endl;
            }
            break;

            // Hide
        case 'h':
        case 'H':
            if (render) {
                run = false;
                render = false;
                std::cout << "==[ Switch to HIDDEN mode ]==" << std::endl;
            } else {
                std::cerr << "** Only allowed in RENDER mode **" << std::endl;
            }
            break;

            // Pause/Play
        case 'p':
        case 'P':
            if (render) {
                // Simulation is already paused in similarity mode!
                if (!similarityMode) paused = !paused;
            } else {
                std::cerr << "** Only allowed in RENDER mode **" << std::endl;
            }
            break;

            // Similarity mode
        case 's':
        case 'S':
            if (render) {
                similarityMode = paused = !similarityMode;
                selectedEntities[0] = selectedEntities[1] = nullptr;
                countSelectedEntities = 0;
            } else {
                std::cerr << "** Only allowed in RENDER mode **" << std::endl;
            }
            break;

            // Show borders of the world
        case 'b':
        case 'B':
            if (render) borders = !borders;
            else std::cerr << "** Only allowed in RENDER mode **" << std::endl;
            break;

            // Show padding areas
        case 'a':
        case 'A':
            if (render) paddings = !paddings;
            else std::cerr << "** Only allowed in RENDER mode **" << std::endl;
            break;
#endif

            default:
                std::cerr << "** Unknown command! **" << std::endl;
                break;
        }
        pthread_mutex_unlock(&cmdMutex);
        if (quit) break;
    }
    pthread_exit(args);
}

/**
 * Event Loop that is also rendering and updating the world.
 *
 * @param   ticks   Amount of ticks to calculate
 *                  (-1) = no limitation
 */
#ifdef RENDER

/**
 * =============================================================================
 *                                  RENDER LOOP
 * =============================================================================
 */
void renderLoop() {
    Renderer::show();

    // Pre-render Textures for faster copying
    SDL_Texture *border = nullptr, *pauseText = nullptr, *padding = nullptr;
    preRender(&border, &pauseText, &padding);

    // Frame rate
    SDL_Texture *fps = nullptr;
    SDL_Rect fpsRect = {0, 10, 0, 0}; // Always 10px padding to the top
    int frameTime = 0, frames = 0;

    uint8_t buffer;
    WorldDim dim = World::getWorldDim();
    run = true;

    // Force quit application when failing! --> must be broadcasted to all applications!
    if (!threadSuccessfullyCreated && !automated) {
        std::cerr << "** CLI Thread couldn't be created. Please try again! **" << std::endl;
        run = false;
        quit = true;
    }

    //=============================================================================
    //                               BEGIN MAIN LOOP
    //=============================================================================
    SDL_Event e;
    int currentTime, elapsedTime;
    int previousTime = Log::currentTime();
    while (true) {
        currentTime = Log::currentTime();
        frameTime += currentTime - previousTime;
        previousTime = currentTime;

        // Log new tick
        if (counter % LOG_TICKS_2_PAUSE == 0)
            Log::enable();

        if (Log::isEnabled()) {
            Log::data.turn = counter;
            Log::data.food = World::getAmountOfFood();
            Log::data.livings = World::getAmountOfLivings();
        }
        counter++;

        //######################## PROCESS KEYBOARD-INPUT #########################
        pthread_mutex_lock(&cmdMutex);
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

                        default:
                            break;
                    }
                    break;

                    // Mouse clicked?
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_RIGHT) {
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
                                if (nearest->brain != nullptr)
                                    nearest->brain->printThink = true;
                            } else {
                                std::cout << "No nearest entity available!" << std::endl;
                            }
                        }
                    } else if (e.button.button == SDL_BUTTON_LEFT) {
                        auto *f = new FoodEntity(
                                dim.p.x + e.button.x,
                                dim.p.y + e.button.y,
                                8 * 60);
                        f->beer = true;
                        World::addFoodEntity(f, false);
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
        if (!run) {
            pthread_mutex_unlock(&cmdMutex);
            break;
        }
        pthread_mutex_unlock(&cmdMutex);

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

        //################################## TICK #################################
        if (!paused) {
            if (Log::isEnabled()) {
                int tickTime = Log::currentTime();
                World::tick();
                Log::data.tick = Log::endTime(tickTime);
            } else {
                World::tick();
            }
        }

        //################################# RENDER ################################
        int renderTime = (Log::isEnabled()) ? Log::currentTime() : 0;
        Renderer::clear();

        Renderer::drawBackground(World::getWorldDim());
        if (!paused) Renderer::renderEntities(World::food, World::living, World::livingsInPadding);
        Renderer::copy(Renderer::entities, 0, 0);
        Renderer::drawRank();

        if (paddings) Renderer::copy(padding, 0, 0);
        if (borders) Renderer::copy(border, 0, 0);
        if (paused) Renderer::copy(pauseText, 10, 10);
        Renderer::copy(fps, &fpsRect);
        Renderer::present();
        frames++;

        if (Log::isEnabled())
            Log::data.render = Log::endTime(renderTime);

        //########################### WAIT IF TOO FAST ############################
        currentTime = Log::currentTime();
        elapsedTime = (currentTime - previousTime);

        // Delay loop turn
        if (elapsedTime <= minTickPeriod)
            SDL_Delay(minTickPeriod - elapsedTime);

        if (Log::isEnabled()) {
            Log::data.delay = (elapsedTime <= minTickPeriod) ? minTickPeriod - elapsedTime : 0;
            Log::data.overall = Log::endTime(previousTime);
            Log::saveLogData();
            Log::disable();
        }
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

/**
 * =============================================================================
 *                                  NORMAL LOOP
 * =============================================================================
 */
void normalLoop() {
    uint8_t buffer = 0;
    run = true;

    // Force quit application when failing! --> must be broadcasted to all applications!
    if (!threadSuccessfullyCreated && !automated) {
        std::cerr << "** CLI Thread couldn't be created. Please try again! **" << std::endl;
        run = false;
        quit = true;
    }

    while (true) {
        if (counter % LOG_TICKS_2_PAUSE == 0)
            Log::enable();

        int loopTime = (Log::isEnabled()) ? Log::currentTime() : 0;
        if (Log::isEnabled()) {
            Log::data.turn = counter;
            Log::data.food = World::getAmountOfFood();
            Log::data.livings = World::getAmountOfLivings();
        }
        counter++;

        pthread_mutex_lock(&cmdMutex);
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
        if (!run) {
            pthread_mutex_unlock(&cmdMutex);
            break;
        }
        pthread_mutex_unlock(&cmdMutex);

        //################################## TICK #################################
        if (Log::isEnabled()) {
            int tickTime = Log::currentTime();
            World::tick();
            Log::data.tick = Log::endTime(tickTime);

            Log::data.overall = Log::endTime(loopTime);
            Log::saveLogData();
            Log::disable();
        } else {
            World::tick();
        }
    }
}

/**
 * =============================================================================
 *                                CREATE ENTITIES
 * =============================================================================
 */
void createEntities(long livings, long food, LFSR &random) {
    int nodes = World::getMPINodes();

    int lCounts[nodes], fCounts[nodes];
    int lDisplacement[nodes], fDisplacement[nodes];
    void *sendLBuffer = nullptr, *sendFBuffer = nullptr;

    // TODO: Maybe free?
    std::ifstream file("./res/brains.dat", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    if (size == -1) std::cout << "Cannot determine size of brains.dat" << std::endl;
    file.seekg(0, std::ios::beg);
    std::vector<char> vec(size);
    if (!file.read(vec.data(), size)) std::cout << "Could not open brains.dat" << std::endl;
    int *buffer = reinterpret_cast<int *>(vec.data());
    int numBrains = buffer[0];
    int sizeBrains = buffer[1];
    assert(numBrains * sizeBrains + 8 == size && "Invalid brains.data file!");
    buffer += 2;

    if (World::getMPIRank() == 0) {
        std::list < LivingEntity * > sendLivings[nodes];
        std::list < FoodEntity * > sendFood[nodes];

        // Create Entities
        for (long i = 0; i < livings; i++) {
            Point p = {static_cast<int>(random.getNextIntBetween(0, World::overallWidth)),
                       static_cast<int>(random.getNextIntBetween(0, World::overallHeight))};
            void *brainPos = (void *) (buffer + (random.getNextIntBetween(0, numBrains) * sizeBrains /
                                                 4));//don't care about being modified
            auto *entity = new LivingEntity(
                    p.x, p.y,
                    {
                            static_cast<uint8_t>(random.getNextIntBetween(0, 256)),
                            static_cast<uint8_t>(random.getNextIntBetween(0, 256)),
                            static_cast<uint8_t>(random.getNextIntBetween(0, 256)),
                            255
                    },
                    random.getNextFloatBetween(0.0f, 1.0f),
                    random.getNextFloatBetween(0.0f, 1.0f),
                    random.getNextFloatBetween(0.0f, 1.0f),
                    new Brain(brainPos),
                    random.getNextInt());
            sendLivings[World::rankAt(p.x, p.y)].push_back(entity);
        }
        for (long i = 0; i < food; i++) {
            Point p = {static_cast<int>(random.getNextIntBetween(0, World::overallWidth)),
                       static_cast<int>(random.getNextIntBetween(0, World::overallHeight))};
            sendFood[World::rankAt(p.x, p.y)].push_back(new FoodEntity(p.x, p.y, 8 * 60));
        }

        // Calc sizes and displacements for nodes
        int lTotalSize = 0, fTotalSize = 0;
        for (int rank = 0; rank < nodes; rank++) {
            lDisplacement[rank] = lTotalSize;
            fDisplacement[rank] = fTotalSize;

            lCounts[rank] = fCounts[rank] = 0;
            for (const auto &e : sendLivings[rank]) lCounts[rank] += e->fullSerializedSize();
            for (const auto &e : sendFood[rank]) fCounts[rank] += e->fullSerializedSize();

            lTotalSize += lCounts[rank];
            fTotalSize += fCounts[rank];
        }

        // Serialize everything
        void *lBuffer = sendLBuffer = malloc(lTotalSize);
        void *fBuffer = sendFBuffer = malloc(fTotalSize);
        for (int rank = 0; rank < nodes; rank++) {
            for (const auto &e : sendLivings[rank]) {
                e->fullSerialize(lBuffer);
                delete e;
            }
            for (const auto &e : sendFood[rank]) {
                e->fullSerialize(fBuffer);
                delete e;
            }
        }
    }

    // Send & receive Entities
    int lCount = 0, fCount = 0;
    MPI_Scatter(lCounts, 1, MPI_INT, &lCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(fCounts, 1, MPI_INT, &fCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

    void *lBuffer, *recvLBuffer = lBuffer = malloc(lCount);
    void *fBuffer, *recvFBuffer = fBuffer = malloc(fCount);
    MPI_Scatterv(sendLBuffer, lCounts, lDisplacement, MPI_BYTE, recvLBuffer, lCount, MPI_BYTE,
                 0, MPI_COMM_WORLD);
    MPI_Scatterv(sendFBuffer, fCounts, fDisplacement, MPI_BYTE, recvFBuffer, fCount, MPI_BYTE,
                 0, MPI_COMM_WORLD);

    // Save Entities
    while (lBuffer < static_cast<char *>(recvLBuffer) + lCount) {
        auto *e = new LivingEntity(lBuffer, false);
        World::addLivingEntity(e);
    }
    while (fBuffer < static_cast<char *>(recvFBuffer) + fCount) {
        auto *e = new FoodEntity(fBuffer);
        World::addFoodEntity(e, true);
    }

    if (sendLBuffer != nullptr) free(sendLBuffer);
    if (sendFBuffer != nullptr) free(sendFBuffer);
    free(recvLBuffer);
    free(recvFBuffer);
}

/**
 * =============================================================================
 *                                 MAIN FUNCTION
 * =============================================================================
 */
int main(int argc, char **argv) {
    int timeTotal = Log::currentTime();
    int timeInit = Log::currentTime();
    // START MPI
    MPI_Init(&argc, &argv);

    int width = 960, height = 720;
    bool maimuc = false;
    bool boarisch = false;
    float foodRate = 1.f;  //food spawned per 2000 tiles per tick
    float zoom = 1.f;
    int numThreads = 1;
    long livings = 50, food = 100;
    std::string filename;

    std::random_device rd;
    unsigned int randomSeed = rd();

    // Scan program arguments
    opterr = 0;
    int c;
    while ((c = getopt(argc, argv, "h:w:m::f:r::t:s:l:e:z:p:b::o:a::")) != -1) {
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
            case 'b':
                boarisch = true;
                break;
            case 'a':
                automated = true;
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
            case 'o':
                if (optarg) {
                    numThreads = (int) strtol(optarg, &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -o requires an integer!" << std::endl;
                        return EXIT_FAILURE;
                    }
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

                // Frames-per-second (FPS)
            case 'p':
                if (optarg) {
                    int frameRate = (int) strtol(optarg, &ptr, 10);
                    if (*ptr) {
                        std::cerr << "Option -p requires an integer!" << std::endl;
                        return EXIT_FAILURE;
                    }
                    minTickPeriod = (frameRate > 0) ? 1000 / frameRate : 0;
                }
                break;

                // Unknown Option
            case '?':
                if (optopt == 'h' || optopt == 'w' || optopt == 't' || optopt == 'p') {
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
    LFSR random = LFSR(randomSeed);

    // Init world
    World::setup(width, height, maimuc, foodRate, zoom, random.getNextInt(), numThreads);
    WorldDim dim = World::getWorldDim();
    if (!filename.empty())
        Log::startLogging(filename + "-" + std::to_string(World::getMPIRank()) + ".csv");

#ifdef RENDER
    if (maimuc)
        Renderer::setup(0, 0, dim.w, dim.h, true, boarisch);
    else
        Renderer::setup(dim.p.x, dim.p.y, dim.w, dim.h, false, boarisch);
#endif

    auto threadId = (pthread_t)
    nullptr;
    if (World::getMPIRank() == 0 && !automated) {
        threadSuccessfullyCreated = (0 == pthread_create(&threadId, nullptr, commandLineThread, nullptr));
    }
    timeInit = Log::endTime(timeInit);

    int timeCreateEntities = Log::currentTime();
    createEntities(livings, food, random);
    timeCreateEntities = Log::endTime(timeCreateEntities);

    int timeLoop = Log::currentTime();
    while (!quit) {
#ifdef RENDER
        if (render) renderLoop();
        else normalLoop();
#else
        normalLoop();
#endif
    }
    timeLoop = Log::endTime(timeLoop);

    int timeFinalize = Log::currentTime();
    cancelThread = true;
    if (World::getMPIRank() == 0 && threadSuccessfullyCreated && !automated) {
        pthread_join(threadId, nullptr);
    }

    World::finalize();

#ifdef RENDER
    Renderer::destroy();
#endif

    Log::endLogging();

    // END MPI
    MPI_Finalize();
    timeFinalize = Log::endTime(timeFinalize);
    timeTotal = Log::endTime(timeTotal);

    printf("Initialization:   \t%d\n", timeInit);
    printf("Create Entities:  \t%d\n", timeCreateEntities);
    printf("Loop:             \t%d\n", timeLoop);
    printf("Finalize:         \t%d\n", timeFinalize);
    printf("Total:            \t%d\n", timeTotal);

    return EXIT_SUCCESS;
}
