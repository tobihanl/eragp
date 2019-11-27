#ifndef EVOLUTION_RENDERER_H
#define EVOLUTION_RENDERER_H

#include <string>
#include <iostream>
#include <SDL.h>
#include "SDL/cleanup.h"

class Renderer {
private:
    static SDL_Window *win;
    static SDL_Renderer *ren;
    static bool isSetup;

    Renderer() = default;

    ~Renderer() = default;

public:
    /**
     * Set up the renderer by creating a window with the given width
     * and height
     *
     * @param   x           Position of the window (x coordinate)
     * @param   y           Position of the window (y coordinate)
     * @param   width       Width of the new window
     * @param   height      Height of the new window
     * @param   fullscreen  Decide, whether the application should be run in
     *                      fullscreen mode or not
     *
     * @return  0 if successful, 1 if not successful, -1 if the
     *          renderer is already setup
     */
    static int setup(int x, int y, int width, int height, bool fullscreen);

    /**
     * Destroys the renderer by cleaning up all the SDL components and
     * quitting SDL
     */
    static void destroy() {
        if (!isSetup) return;

        Include::cleanup(ren, win);
        SDL_Quit();

        isSetup = false;
    }

    /**
     * Clears the SDL renderer
     */
    static void clear() {
        if (!isSetup) return;
        SDL_RenderClear(ren);
    }

    /**
     * Shows all components on the window
     */
    static void present() {
        if (!isSetup) return;
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
    static void copy(SDL_Texture *texture, const SDL_Rect *dst) {
        if (!isSetup) return;
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
    static void copy(SDL_Texture *texture, int x, int y) {
        if (!isSetup) return;

        SDL_Rect dst;
        dst.x = x;
        dst.y = y;

        SDL_QueryTexture(texture, nullptr, nullptr, &dst.w, &dst.h);
        SDL_RenderCopy(ren, texture, nullptr, &dst);
    }

    /**
     * Query a texture for its width and height. Result will be writen
     * in the SDL_Rect argument.
     *
     * @param   texture Texture to be queried
     * @param   rect    Rectangle to write the resulting width and
     *                  height into
     */
    static void query(SDL_Texture *texture, SDL_Rect *rect) {
        SDL_QueryTexture(texture, nullptr, nullptr, &rect->w, &rect->h);
    }

    /**
     * Cleans-up all the textures by destroying them.
     *
     * @param   texture The texture to be destroyed
     */
    static void cleanup(SDL_Texture *texture) {
        if (!isSetup) return;
        Include::cleanup(texture);
    }

    /**
     * Set a new target for the Renderer
     *
     * @param   target  The texture to be the new render target.
     *                  Default target when nullptr.
     *
     * @attention   The texture must have an applicable access flag
     *              (SDL_TEXTUREACCESS_TARGET)!
     */
    static void setTarget(SDL_Texture *target) {
        if (!isSetup) return;
        SDL_SetRenderTarget(ren, target);
    }

    /**
     * Creates a tetxure.
     *
     * @param   width   Width of the new texture
     * @param   height  Height of the new texture
     * @param   access  Access flag for the texture (i.e. important for
     *                  settings as a render target)
     *
     * @return  Pointer to the created SDL_Texture
     */
    static SDL_Texture *createTexture(int width, int height, int access) {
        if (!isSetup) return nullptr;
        return SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, access, width, height);
    }

    /**
     * Renders an image.
     *
     * @param   imagePath   Path to the image relative to the resource folder
     *
     * @return  Pointer to the created SDL_Texture
     */
    static SDL_Texture *renderImage(const std::string &imagePath);

    /**
     * Renders a dot/filled circle
     *
     * @param   radius  The radius of the circle
     * @param   color   The color of the circle (SDL_Color structure)
     *
     * @return  A pointer to the texture with the specified dot/filled circle
     */
    static SDL_Texture *renderDot(int radius, const SDL_Color &color);

    /**
     * Renders a filled or unfilled rectangle
     *
     * @param   width   Width of the new rectangle
     * @param   height  Height of the new rectangle
     * @param   color   Drawing color for the rectangle
     * @param   filled  Decides, whether the rectangle will be drawn filled
     *                  or not (filled area will be transparent)
     *
     * @return  Pointer to the texture with the specified rectangle.
     */
    static SDL_Texture *renderRect(int width, int height, const SDL_Color &color, bool filled);

    /**
     * Renders a text on the window
     *
     * @param   text        The text to be displayed
     * @param   size        The font size
     * @param   color       Text's color
     * @param   fontFile    Path to the file where the font is stored,
     *                      relative to resource folder (TTF-File)
     *
     * @return Pointer to the created SDL_Texture
     */
    static SDL_Texture *
    renderFont(const std::string &text, int size, const SDL_Color &color, const std::string &fontFile);

    static bool getIsSetup() { return isSetup; }

private:
    /**
     * Log an SDL error with some error message to the output stream
     * of your choice
     *
     * @param   os  The output stream to write the message to
     * @param   msg The error message to write, format will be
     *              "msg error: SDL_GetError()"
     */
    static void logSDLError(std::ostream &os, const std::string &msg) {
        os << msg << " error: " << SDL_GetError() << std::endl;
    }
};

#endif //EVOLUTION_RENDERER_H
