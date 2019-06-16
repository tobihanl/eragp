//
// Created by jonas on 05.06.2019.
//

#include <SDL.h>
#include "Renderer.h"

int main() {
    Renderer::setup(640, 480);

    // Render all needed textures
    SDL_Texture *img = Renderer::renderImage("background.bmp");
    SDL_Texture *hello = Renderer::renderFont("Hello World!", 20, {255, 255, 255, 0}, "font.ttf");
    SDL_Texture *text = Renderer::renderFont("I am an evolution simulating program", 20, {255, 255, 255, 0},
                                             "font.ttf");

    // Position all the textures
    SDL_Rect dstImg = {250, 100, 0, 0};
    SDL_Rect dstHello = {10, 10, 0, 0};
    SDL_Rect dstText = {10, 30, 0, 0};
    SDL_QueryTexture(img, nullptr, nullptr, &dstImg.w, &dstImg.h);
    SDL_QueryTexture(hello, nullptr, nullptr, &dstHello.w, &dstHello.h);
    SDL_QueryTexture(text, nullptr, nullptr, &dstText.w, &dstText.h);

    // Example Event Loop for letting SDL show up a window with some content
    SDL_Event e;
    bool run = true;
    while (run) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                run = false;
        }

        Renderer::clear();

        Renderer::renderDot(100, 100, 5, {255, 0, 0, 0});
        Renderer::renderDot(200, 100, 5, {0, 255, 0, 0});
        Renderer::renderDot(100, 200, 5, {0, 0, 255, 0});
        Renderer::renderDot(200, 200, 5, {255, 255, 255, 0});

        Renderer::copy(img, &dstImg);
        Renderer::copy(hello, &dstHello);
        Renderer::copy(text, &dstText);

        Renderer::present();

        // 2 frames-per-second
        SDL_Delay(1000 / 5);
    }

    // Important: Clean-up all the textures!
    Renderer::cleanup(img);
    Renderer::cleanup(hello);
    Renderer::cleanup(text);

    Renderer::destroy();
    return 0;
}
