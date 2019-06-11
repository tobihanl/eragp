#ifndef EVOLUTION_RENDERER_H
#define EVOLUTION_RENDERER_H

#include <string>
#include <SDL.h>

class Renderer {
private:
    static SDL_Window *win;
    static SDL_Renderer *ren;
    static bool isSetup;
    static int windowWidth;
    static int windowHeight;

    Renderer() = default;
    ~Renderer() = default;

public:
    static int setup(int width, int height);
    static void destroy();
    static void clear();
    static void present();
    static bool renderImage(const std::string &imagePath, int x, int y);
    static bool renderDot(int centerX, int centerY, int radius, SDL_Color color);
    static bool renderFont(const std::string &text, int x, int y, int size, SDL_Color color, const std::string &fontFile);

private:
    static void logSDLError(std::ostream &, const std::string &);
};

#endif //EVOLUTION_RENDERER_H
