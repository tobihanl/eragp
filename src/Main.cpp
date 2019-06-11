//
// Created by jonas on 05.06.2019.
//

#include <SDL.h>
#include "Renderer.h"

int main()
{
    Renderer::setup(640, 480);

    // Example Event Loop for letting SDL show up a window with some content
    SDL_Event e;
    bool run = true;
    while (run)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                run = false;
        }

        Renderer::clear();
        Renderer::renderImage("background.bmp", 250, 100);
        Renderer::renderDot(100, 100, 5, {255, 0, 0, 0});
        Renderer::renderDot(200, 100, 5, {0, 255, 0, 0});
        Renderer::renderDot(100, 200, 5, {0, 0, 255, 0});
        Renderer::renderDot(200, 200, 5, {255, 255, 255, 0});
        Renderer::renderFont("Hello World!", 10, 10, 20, {255, 255, 255, 0}, "font.ttf");
        Renderer::renderFont("I am an evolution simulating program", 10, 30, 20, {255, 255, 255, 0}, "font.ttf");
        Renderer::present();

        // 2 frames-per-second
        SDL_Delay(1000/2);
    }

    Renderer::destroy();
    return 0;
}
