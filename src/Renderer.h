#ifndef EVOLUTION_RENDERER_H
#define EVOLUTION_RENDERER_H

#include <string>
#include <SDL.h>

// Structure for holding the texture with its position on the renderer
typedef struct RenderTexture {
    SDL_Texture *tex;
    SDL_Rect dst;
} RenderTexture;

// Class Renderer
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

    static void copy(const RenderTexture &texture);

    static void cleanup(RenderTexture &texture);

    static RenderTexture renderImage(const std::string &imagePath, int x, int y);

    static bool renderDot(int centerX, int centerY, int radius, const SDL_Color &color);

    static RenderTexture
    renderFont(const std::string &text, int x, int y, int size, const SDL_Color &color, const std::string &fontFile);

private:
    static void logSDLError(std::ostream &, const std::string &);
};

#endif //EVOLUTION_RENDERER_H
