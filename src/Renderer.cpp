#include <iostream>
#include <utility>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "SDL/res_path.h"
#include "SDL/cleanup.h"
#include "Renderer.h"

SDL_Window *Renderer::win = nullptr;
SDL_Renderer *Renderer::ren = nullptr;
bool Renderer::isSetup = false;
int Renderer::windowWidth = 0;
int Renderer::windowHeight = 0;

/**
 * Set up the renderer by creating a window with the given width
 * and height
 *
 * @return  0 if successful, 1 if not successful, -1 if the
 *          renderer is already setup
 */
int Renderer::setup(int width, int height) {
    // Renderer already set up?
    if (isSetup)
        return -1;

    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        logSDLError(std::cerr, "SDL_Init");
        return 1;
    }

    // Init Font Library
    if (TTF_Init() != 0) {
        logSDLError(std::cerr, "TTF_Init");
        return 1;
    }

    // Create Window
    win = SDL_CreateWindow("Evolution", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                           SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        logSDLError(std::cerr, "SDL_CreateWindow");
        SDL_Quit();
        return 1;
    }

    windowHeight = height;
    windowWidth = width;

    // Create Renderer
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        Include::cleanup(win);
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
 * quitting SDL
 */
void Renderer::destroy() {
    Include::cleanup(ren, win);
    SDL_Quit();

    isSetup = false;
    windowWidth = 0;
    windowHeight = 0;
}

/**
 * Clears the SDL renderer
 */
void Renderer::clear() {
    SDL_RenderClear(ren);
}

/**
 * Shows all components on the window
 */
void Renderer::present() {
    SDL_RenderPresent(ren);
}

/**
 * Renders all the RenderTextures by copying them into the SDL renderer.
 */
void Renderer::copy(const RenderTexture &texture) {
    SDL_RenderCopy(ren, texture.tex, nullptr, &texture.dst);
}

/**
 * Cleans-up all the RenderTextures by destroying the textures.
 */
void Renderer::cleanup(RenderTexture &texture) {
    Include::cleanup(texture.tex);
    texture.tex = nullptr; // No texture available anymore
}

/**
 * Renders an image on the window on the given position
 *
 * @param x Coordinates according to the top-left corner of the window
 * @param y Coordinates according to the top-left corner of the window
 * @param imagePath Path to the image relative to the resource folder
 */
RenderTexture Renderer::renderImage(const std::string &imagePath, int x, int y) {
    std::string file = Include::getResourcePath() + imagePath;
    SDL_Texture *tex = IMG_LoadTexture(ren, file.c_str());
    if (tex == nullptr) {
        logSDLError(std::cerr, "IMG_LoadTexture");
        return {};
    }

    // Draw Image to the specified location
    SDL_Rect dst;
    dst.x = (x >= 0) ? x : windowWidth + x;
    dst.y = (y >= 0) ? y : windowHeight + y;
    SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);

    return {tex, dst};
}

/**
 * Renders a dot/filled circle on the window on the given (centered!) position
 *
 * @param centerX The position of the circle-center
 * @param centerY The position of the circle-center
 * @param radius The radius of the circle
 * @param color The color of the circle
 */
bool Renderer::renderDot(int centerX, int centerY, int radius, const SDL_Color &color) {
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);

    // Draw filled Circle/Dot
    int dx, dy;
    for (int w = 0; w <= (radius + radius); w++) {
        for (int h = 0; h <= (radius + radius); h++) {
            dx = radius - w;
            dy = radius - h;

            if ((dx * dx + dy * dy) < (radius * radius))
                SDL_RenderDrawPoint(ren, centerX + dx, centerY + dy);
        }
    }

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
    return true;
}

/**
 * Renders a text on the window
 *
 * @param text The text to be displayed
 * @param x The position of the text on the window
 * @param y The position of the text on the window
 * @param size The font size
 * @param color Text's color
 * @param fontFile Path to the file where the font is stored, relative to resource folder (TTF-File)
 */
RenderTexture Renderer::renderFont(const std::string &text, int x, int y, int size, const SDL_Color &color,
                                   const std::string &fontFile) {
    std::string file = Include::getResourcePath() + fontFile;
    TTF_Font *font = TTF_OpenFont(file.c_str(), size);
    if (font == nullptr) {
        logSDLError(std::cerr, "TTF_OpenFont");
        return {};
    }

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr) {
        Include::cleanup(font);
        logSDLError(std::cerr, "TTF_RenderText_Blended");
        return {};
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surface);
    if (tex == nullptr) {
        Include::cleanup(surface, font);
        logSDLError(std::cerr, "SDL_CreateTextureFromSurface");
        return {};
    }

    SDL_Rect dst;
    dst.x = (x >= 0) ? x : windowWidth + x;
    dst.y = (y >= 0) ? y : windowHeight + y;
    SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);

    Include::cleanup(surface, font);
    return {tex, dst};
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
