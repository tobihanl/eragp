#include <iostream>
#include <string>
#include <SDL.h>
#include "SDL/res_path.h"
#include "SDL/cleanup.h"
#include "Renderer.h"

SDL_Window *Renderer::win = nullptr;
SDL_Renderer *Renderer::ren = nullptr;
bool Renderer::isSetup = false;

/**
 * Set up the renderer by creating a window with the given width
 * and height.
 *
 * @return  0 if successful, 1 if not successful, -1 if the
 *          renderer is already setup
 */
int Renderer::setup(int width, int height)
{
    // Renderer already set up?
    if (isSetup)
        return -1;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        logSDLError(std::cerr, "SDL_Init");
        return 1;
    }

    win = SDL_CreateWindow("Evolution", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
        logSDLError(std::cerr, "SDL_CreateWindow");
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
        cleanup(win);
        logSDLError(std::cerr, "SDL_CreateRenderer");
        SDL_Quit();
        return 1;
    }

    // Renderer successfully set up
    isSetup = true;
    return 0;
}

/**
 * Destroys the renderer by cleaning up all the SDL components and
 * quitting SDL.
 *
 * @return  0 for success
 */
int Renderer::destroy()
{
    cleanup(ren, win);
    SDL_Quit();

    isSetup = false;
    return 0;
}

/**
 * Clears the SDL_Renderer.
 */
void Renderer::clear() {
    SDL_RenderClear(ren);
}

/**
 * Shows all renderer components on the window.
 */
void Renderer::present() {
    SDL_RenderPresent(ren);
}

void Renderer::renderImage(const int x, const int y, const std::string &image)
{

}

void Renderer::renderDot(const int x, const int y, const int radius)
{

}

void Renderer::renderFont(const int x, const int y, const int size, const std::string &text)
{

}

/**
 * Log an SDL error with some error message to the output stream of our choice
 *
 * @param os The output stream to write the message to
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void Renderer::logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " error: " << SDL_GetError() << std::endl;
}