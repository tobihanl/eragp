//
// Copied and modified from
// https://www.willusher.io/sdl2%20tutorials/2014/06/16/postscript-0-properly-finding-resource-paths
//

#ifndef RES_PATH_H
#define RES_PATH_H

#include <iostream>
#include <string>
#include <SDL.h>

std::string getResourcePath(const std::string &subDir = "")
{
    #ifdef _WIN32
    const char PATH_SEP = '\\';
    #else
    const char PATH_SEP = '/';
    #endif

    // IMPORTANT:   Change this path according to the location of the binary!
    //              Otherwise SDL won't be able to find any resources!
    const std::string PATH_TO_BINARY = "cmake-build-debug";

    static std::string baseRes;
    if (baseRes.empty())
    {
        char *basePath = SDL_GetBasePath();
        if (basePath != nullptr)
        {
            baseRes = basePath;
            SDL_free(basePath);
        }
        else
        {
            std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
            return "";
        }

        size_t pos = baseRes.rfind(PATH_TO_BINARY);
        baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
    }

    return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
}

#endif //RES_PATH_H