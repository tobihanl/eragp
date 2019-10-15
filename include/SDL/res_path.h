//
// Copied and modified from
// https://www.willusher.io/sdl2%20tutorials/2014/06/16/postscript-0-properly-finding-resource-paths
//

#ifndef RES_PATH_H
#define RES_PATH_H

#include <iostream>
#include <string>
#include <SDL.h>

namespace Include {
    std::string getResourcePath(const std::string &subDir = "") {
#ifdef _WIN32
        const char PATH_SEP = '\\';
#else
        const char PATH_SEP = '/';
#endif

        static std::string baseRes;
        if (baseRes.empty()) {
            char *basePath = SDL_GetBasePath();
            if (basePath != nullptr) {
                baseRes = basePath;
                SDL_free(basePath);
            } else {
                std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
                return "";
            }

            // Exclude build or debug folder if found
            size_t pos = baseRes.rfind("build");
            if (pos == std::string::npos) pos = baseRes.rfind("debug");
            baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
        }

        return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
    }
}

#endif //RES_PATH_H
