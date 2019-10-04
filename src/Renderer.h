#ifndef EVOLUTION_RENDERER_H
#define EVOLUTION_RENDERER_H

#include <string>
#include <SDL.h>

class Renderer {
private:
    static SDL_Window *win;
    static SDL_Renderer *ren;
    static bool isSetup;

    Renderer() = default;

    ~Renderer() = default;

public:
    static int setup(int x, int y, int width, int height, bool fullscreen);

    static void destroy();

    static void clear();

    static void present();

    static void copy(SDL_Texture *texture, const SDL_Rect *dst);

    static void copy(SDL_Texture *texture, int x, int y);

    static void cleanup(SDL_Texture *texture);

    static void setTarget(SDL_Texture *texture);

    static SDL_Texture *createTexture(int width, int height, int access);

    static SDL_Texture *renderImage(const std::string &imagePath);

    static SDL_Texture *renderDot(int radius, const SDL_Color &color);

    static SDL_Texture *renderRect(int width, int height, const SDL_Color &color, bool filled);

    static SDL_Texture *
    renderFont(const std::string &text, int size, const SDL_Color &color, const std::string &fontFile);

private:
    static void logSDLError(std::ostream &, const std::string &);
};

#endif //EVOLUTION_RENDERER_H
