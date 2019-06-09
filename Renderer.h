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
    static int setup(int width, int height);
    static int destroy();
    static void clear();
    static void present();
    static void renderImage(int x, int y, const std::string &image);
    static void renderDot(int x, int y, int radius); // TODO: Add color param
    static void renderFont(int x, int y, int size, const std::string &text);

private:
    static void logSDLError(std::ostream &os, const std::string &msg);
};

#endif //EVOLUTION_RENDERER_H