//
// Created by jonas on 05.06.2019.
//

#include <SDL.h>
#include "Renderer.h"

int main()
{
    Renderer::setup(640, 480);

    // Example Event Loop for letting SDL show up a window
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
        Renderer::present();

        // 5 frames-per-second
        SDL_Delay(1000/5);
    }

    Renderer::destroy();
    return 0;
}