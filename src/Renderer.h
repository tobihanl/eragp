#ifndef EVOLUTION_RENDERER_H
#define EVOLUTION_RENDERER_H

#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL.h>
#include "SDL/cleanup.h"
#include "Structs.h"
#include "Tile.h"
#include "FoodEntity.h"
#include "LivingEntity.h"

//================================== Structs ==================================
struct LivingTexture {
    int livingId;
    SDL_Texture *texture;

    // For std::set
    bool operator()(const LivingTexture &lhs, const LivingTexture &rhs) const {
        return lhs < rhs;
    }

    // For std::set
    bool operator<(const LivingTexture &lhs) const {
        return lhs.livingId < livingId;
    }
};

//=================================== Class ===================================
class Renderer {
private:
    static SDL_Window *win;
    static SDL_Renderer *ren;
    static bool isSetup;
    static bool hidden;
    static bool boarisch;
    static float scaling; // Factor scaling dots & fonts
    static SDL_Texture *digits[];

    static SDL_Texture *foodTexture;
    static SDL_Texture *beerTexture;
    static SDL_Texture *pretzelTexture;
    static std::set<LivingTexture> livingTextures;
    static std::set<LivingTexture> livingTexturesSwap;

    Renderer() = default;

    ~Renderer() = default;

public:
    static SDL_Texture *background;
    static SDL_Texture *entities;
    static SDL_Texture *rankTexture;

    /**
     * Set up the renderer by creating a window with the given width
     * and height
     *
     * @param   x           Position of the window (x coordinate)
     * @param   y           Position of the window (y coordinate)
     * @param   width       Width of the new window
     * @param   height      Height of the new window
     *
     * @return  0 if successful, 1 if not successful, -1 if the
     *          renderer is already setup
     */
    static int setup(int x, int y, int width, int height, float scale, bool boarischFlag);

    /**
     * Destroys the renderer by cleaning up all the SDL components and
     * quitting SDL
     */
    static void destroy() {
        if (!isSetup) return;

        Include::cleanup(ren, win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();

        isSetup = false;
    }

    /**
     * Hide window
     */
    static void hide() {
        if (!isSetup || hidden) return;
        SDL_HideWindow(win);
        hidden = true;
    }

    /**
     * Show window
     */
    static void show() {
        if (!isSetup || !hidden) return;
        SDL_ShowWindow(win);
        hidden = false;
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

    static void cleanup() {
        for (int i = 0; i < 10; i++) {
            cleanupTexture(digits[i]);
            digits[i] = nullptr;
        }
        cleanupTexture(background);
        background = nullptr;
        cleanupTexture(entities);
        entities = nullptr;
        cleanupTexture(rankTexture);
        rankTexture = nullptr;
        cleanupTexture(foodTexture);
        foodTexture = nullptr;
        cleanupTexture(pretzelTexture);
        pretzelTexture = nullptr;
        cleanupTexture(beerTexture);
        beerTexture = nullptr;
        for (const auto &item : livingTextures)
            cleanupTexture(item.texture);
        livingTextures.clear();
    }

    /**
     * Cleans-up all the textures by destroying them.
     *
     * @param   texture The texture to be destroyed
     */
    static void cleanupTexture(SDL_Texture *texture) {
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
    static SDL_Texture *renderDot(int radius, const Color &color);

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

    static void renderDigits();

    static void renderRank(int rank) {
        if (!isSetup) return;
        rankTexture = renderFont(std::to_string(rank), 25, {255, 255, 255, 255}, "font.ttf");
    }

    static void renderBackground(WorldDim dim, const std::vector<Tile *> &terrain, int rank);

    static void createEntitiesTexture(WorldDim dim) {
        if (!isSetup) return;
        entities = createTexture(dim.w, dim.h, SDL_TEXTUREACCESS_TARGET);
        SDL_SetTextureBlendMode(entities, SDL_BLENDMODE_BLEND);
    }

    static void drawEntities(const std::vector<FoodEntity *> &food, const std::vector<LivingEntity *> &living,
                             const std::vector<LivingEntity *> &livingsInPadding) {
        if (!isSetup) return;

        setTarget(entities);
        clear();

        for (const auto &f : food) drawFoodEntity(f);
        for (const auto &e : living) drawLivingEntity(e);
        for (const auto &e : livingsInPadding) drawLivingEntity(e);

        livingTextures.swap(livingTexturesSwap);
        for (const auto &item : livingTexturesSwap) cleanupTexture(item.texture);
        livingTexturesSwap.clear();

        setTarget(nullptr);
    }

    static void drawLivingEntity(LivingEntity *e);

    static void drawFoodEntity(FoodEntity *e) {
        if (!isSetup) return;
        RenderData data = e->getRenderData();
        if (boarisch) {
            if (pretzelTexture == nullptr) pretzelTexture = renderImage("pretzel-128.png");
            if (beerTexture == nullptr) beerTexture = renderImage("beer-128.png");

            int size = (int) (scaling * (128.f / 8));
            SDL_Texture *tex = (e->beer) ? beerTexture : pretzelTexture;
            SDL_Rect dst = {data.x - data.worldDim.p.x - (size / 2),
                            data.y - data.worldDim.p.y - (size / 2), size, size};
            copy(tex, &dst);
        } else {
            if (foodTexture == nullptr) foodTexture = renderDot(data.radius, data.color);

            SDL_Rect dst = {data.x - data.worldDim.p.x, data.y - data.worldDim.p.y, 0, 0};
            query(foodTexture, &dst);
            dst.x -= dst.w / 2;
            dst.y -= dst.h / 2;
            copy(foodTexture, &dst);
        }
    }

    static void drawBackground(WorldDim dim) {
        if (!isSetup) return;
        copy(background, -(WORLD_PADDING + (dim.p.x % TILE_SIZE)), -(WORLD_PADDING + (dim.p.y % TILE_SIZE)));
    }

    static void drawRank() {
        if (!isSetup) return;
        copy(rankTexture, 10, 10);
    }

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

    static int getNumDigits(int x) {
        if (x < 10) return 1;
        else if (x < 100) return 2;
        else if (x < 1000) return 3;
        else if (x < 10000) return 4;
        assert(x >= 10000 && "Too much energy");
        return -1;
    }
};

#endif //EVOLUTION_RENDERER_H
