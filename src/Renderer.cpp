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

/**
 * Set up the renderer by creating a window with the given width
 * and height
 *
 * @param width Width of the new window
 * @param height Height of the new window
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
 * Renders a textures by copying it into the SDL renderer at the
 * specified location.
 *
 * @param   texture Pointer to the texture being copied into the
 *                  renderer
 *
 * @param   dst     Destination rectangle, where the texture has
 *                  to be drawn
 */
void Renderer::copy(SDL_Texture *texture, const SDL_Rect *dst) {
    SDL_RenderCopy(ren, texture, nullptr, dst);
}

/**
 * Renders a texture by copying it into the SDL renderer at the
 * specified x and y position. The width and height of the
 * texture will be automatically evaluated.
 *
 * @param   texture Pointer to the texture being copied into the
 *                  renderer
 *
 * @param   x       The x position of the texture on the renderer
 * @param   y       The y position of the texture on the renderer
 */
void Renderer::copy(SDL_Texture *texture, int x, int y) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    SDL_QueryTexture(texture, nullptr, nullptr, &dst.w, &dst.h);
    SDL_RenderCopy(ren, texture, nullptr, &dst);
}

/**
 * Cleans-up all the textures by destroying them.
 *
 * @param texture The texture to be destroyed
 */
void Renderer::cleanup(SDL_Texture *texture) {
    Include::cleanup(texture);
}

/**
 * Renders an image on the window on the given position
 *
 * @param imagePath Path to the image relative to the resource folder
 *
 * @return Pointer to the created SDL_Texture
 */
SDL_Texture *Renderer::renderImage(const std::string &imagePath) {
    std::string file = Include::getResourcePath() + imagePath;
    SDL_Texture *tex = IMG_LoadTexture(ren, file.c_str());
    if (tex == nullptr) {
        logSDLError(std::cerr, "IMG_LoadTexture");
        return {};
    }

    return tex;
}

/**
 * Renders a dot/filled circle on the window on the given (centered!) position
 *
 * @param centerX The position of the circle-center
 * @param centerY The position of the circle-center
 * @param radius The radius of the circle
 * @param color The color of the circle (SDL_Color structure)
 */
void Renderer::renderDot(int centerX, int centerY, int radius, const SDL_Color &color) {
    int squaredRadius = radius * radius, doubledRadius = radius + radius;
    SDL_Point points[4 * squaredRadius];

    // Calculate positions of all points needed to draw a filled circle/dot
    int i = 0;
    int dx, dy;
    for (int w = 0; w <= doubledRadius; w++) {
        for (int h = 0; h <= doubledRadius; h++) {
            dx = radius - w;
            dy = radius - h;

            if ((dx * dx + dy * dy) < squaredRadius) {
                points[i].x = centerX + dx;
                points[i].y = centerY + dy;
                i++;
            }
        }
    }

    // Draw filled circle/Dot
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoints(ren, points, i);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
}

/**
 * Renders a text on the window
 *
 * @param text The text to be displayed
 * @param size The font size
 * @param color Text's color
 * @param fontFile Path to the file where the font is stored, relative to resource folder (TTF-File)
 *
 * @return Pointer to the created SDL_Texture
 */
SDL_Texture *Renderer::renderFont(const std::string &text, int size, const SDL_Color &color,
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

    Include::cleanup(surface, font);
    return tex;
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
