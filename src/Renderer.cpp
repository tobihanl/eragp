#include "SDL/res_path.h"
#include "Renderer.h"
#include <vector>
#include <fstream>

SDL_Window *Renderer::win = nullptr;
SDL_Renderer *Renderer::ren = nullptr;
bool Renderer::isSetup = false;
bool Renderer::hidden = true;
bool Renderer::boarisch = false;
float Renderer::scaling = 1;
SDL_Texture *Renderer::digits[10];

SDL_Texture *Renderer::background = nullptr;
SDL_Texture *Renderer::entities = nullptr;
SDL_Texture *Renderer::rankTexture = nullptr;

SDL_Texture *Renderer::foodTexture = nullptr;
SDL_Texture *Renderer::beerTexture = nullptr;
SDL_Texture *Renderer::pretzelTexture = nullptr;
std::set<LivingTexture> Renderer::livingTextures = std::set<LivingTexture>();
std::set<LivingTexture> Renderer::livingTexturesSwap = std::set<LivingTexture>();

int Renderer::setup(int x, int y, int width, int height, float scale, bool boarischFlag) {
    // Renderer already set up?
    if (isSetup)
        return -1;

    boarisch = boarischFlag;
    scaling = scale;

    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logSDLError(std::cerr, "SDL_Init");
        return 1;
    }

    // Init Font Library
    if (TTF_Init() != 0) {
        logSDLError(std::cerr, "TTF_Init");
        return 1;
    }

    // Init Image Library
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        logSDLError(std::cerr, "IMG_Init");
        return 1;
    }

    // Create Window
    win = SDL_CreateWindow("Evolution", x, y, width, height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN);

    if (win == nullptr) {
        logSDLError(std::cerr, "SDL_CreateWindow");
        SDL_Quit();
        return 1;
    }

    // Create Renderer
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        Include::cleanup(win);
        logSDLError(std::cerr, "SDL_CreateRenderer");
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 0);

    // Renderer successfully set up
    isSetup = true;
    return 0;
}

void Renderer::renderBackground(WorldDim dim, const std::vector<Tile *> &terrain, int rank) {
    if (!isSetup) return;

    int heightWithPadding = dim.h + (2 * WORLD_PADDING) + (dim.p.y % TILE_SIZE) + (TILE_SIZE - (dim.p.y + dim.h) %
                                                                                               TILE_SIZE);
    int widthWithPadding = dim.w + (2 * WORLD_PADDING) + (dim.p.x % TILE_SIZE) + (TILE_SIZE - (dim.p.x + dim.w) %
                                                                                              TILE_SIZE);

    // Pre-render terrain for faster rendering
    SDL_Texture *tex = createTexture(widthWithPadding, heightWithPadding, SDL_TEXTUREACCESS_TARGET);
    setTarget(tex);
    clear();

    SDL_Texture *tileTex[4];
    tileTex[0] = renderImage("grass.png");
    tileTex[1] = renderImage("water.png");
    tileTex[2] = renderImage("stone.png");
    tileTex[3] = renderImage("sand.png");

    // Copy textures to background
    for (int py = 0; py < heightWithPadding / TILE_SIZE; py++) {
        for (int px = 0; px < widthWithPadding / TILE_SIZE; px++) {
            copy(tileTex[terrain[py * (widthWithPadding / TILE_SIZE) + px]->id],
                 px * TILE_SIZE,
                 py * TILE_SIZE);
        }
    }

    // Render a logo if one exists with name <Rank>.png
    std::string fileName = "logos/" + std::to_string(rank) + ".png";
    if (std::ifstream("./res/" + fileName)) {
        SDL_Texture *logos = renderImage(fileName);

        int imageWidth, imageHeight;
        SDL_QueryTexture(logos, nullptr, nullptr, &imageWidth, &imageHeight);
        copy(logos, (widthWithPadding - imageWidth) / 2, (heightWithPadding - imageHeight) / 2);
    }

    // Change render target back to default
    setTarget(nullptr);
    background = tex;
}

void Renderer::renderDigits() {
    if (!isSetup) return;
    for (int i = 0; i < 10; i++)
        digits[i] = renderFont(std::to_string(i), ENERGY_FONT_SIZE, {255, 255, 255, 255}, "font.ttf");
}

void Renderer::drawLivingEntity(LivingEntity *e) {
    if (!isSetup) return;

    SDL_Texture *dot;
    RenderData data = e->getRenderData();
    LivingTexture cmp = {e->getId(), nullptr};

    auto it = livingTextures.find(cmp);
    if (it != livingTextures.end()) {
        dot = it->texture;
        livingTextures.erase(it);
        livingTexturesSwap.insert(*it);
    } else {
        dot = renderDot(data.radius, data.color);
        livingTexturesSwap.insert({e->getId(), dot});
    }

    SDL_Rect dst = {data.x - data.worldDim.p.x, data.y - data.worldDim.p.y, 0, 0};
    query(dot, &dst);
    dst.x -= dst.w / 2;
    dst.y -= dst.h / 2;
    copy(dot, &dst);

    assert(data.energy > 0 && "Energy must be greater than 0");
    query(digits[0], &dst);
    int numDigits = getNumDigits(data.energy);
    int energyToDisplay = data.energy;
    int baseX = data.x - data.worldDim.p.x + numDigits * (dst.w / 2) - (3 * dst.w / 4);
    int baseY = data.y - data.worldDim.p.y - (dst.h / 2) - dst.h;
    for (int i = 0; energyToDisplay > 0; i++) {
        copy(digits[energyToDisplay % 10], baseX - dst.w * i, baseY);
        energyToDisplay /= 10;
    }
}

SDL_Texture *Renderer::renderImage(const std::string &imagePath) {
    if (!isSetup) return nullptr;

    std::string file = Include::getResourcePath() + imagePath;
    SDL_Texture *tex = IMG_LoadTexture(ren, file.c_str());
    if (tex == nullptr) {
        logSDLError(std::cerr, "IMG_LoadTexture");
        return nullptr;
    }

    return tex;
}

SDL_Texture *Renderer::renderDot(int radius, const Color &color) {
    if (!isSetup) return nullptr;
    radius = (int) (scaling * (float) radius);

    int squaredRadius = radius * radius, doubledRadius = radius + radius;

    // Create Texture and Pixel array
    SDL_Texture *texture = createTexture(doubledRadius, doubledRadius, SDL_TEXTUREACCESS_STATIC);
    auto *pixels = new Uint32[doubledRadius * doubledRadius];

    // Calculate positions of all points needed to draw a filled circle/dot
    int dx, dy;
    for (int w = 0; w < doubledRadius; w++) {
        for (int h = 0; h < doubledRadius; h++) {
            dx = radius - w;
            dy = radius - h;

            if ((dx * dx + dy * dy) < squaredRadius)
                pixels[h * doubledRadius + w] = (color.a << 24u) + (color.r << 16u) + (color.g << 8u) + color.b;
            else
                pixels[h * doubledRadius + w] = 0; // Transparent
        }
    }

    // Draw filled circle/Dot on texture and return it
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, nullptr, pixels, doubledRadius * (int) sizeof(Uint32));

    delete[] pixels;
    return texture;
}

SDL_Texture *Renderer::renderRect(int width, int height, const SDL_Color &color, bool filled) {
    if (!isSetup) return nullptr;

    SDL_Texture *texture = createTexture(width, height, SDL_TEXTUREACCESS_STATIC);
    auto *pixels = new Uint32[width * height];

    // Go through every pixel
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            // If not filled: not at a border?
            if (!filled && h > 0 && h < height - 1 && w > 0 && w < width - 1)
                pixels[h * width + w] = 0; // Transparent
            else
                pixels[h * width + w] = (color.a << 24u) + (color.r << 16u) + (color.g << 8u) + color.b;
        }
    }

    // Draw rect on texture and return it
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, nullptr, pixels, width * (int) sizeof(Uint32));

    delete[] pixels;
    return texture;
}

SDL_Texture *Renderer::renderFont(const std::string &text, int size, const SDL_Color &color,
                                  const std::string &fontFile) {
    if (!isSetup) return nullptr;

    std::string file = Include::getResourcePath() + fontFile;
    TTF_Font *font = TTF_OpenFont(file.c_str(), (int) (scaling * (float) size));
    if (font == nullptr) {
        logSDLError(std::cerr, "TTF_OpenFont");
        return nullptr;
    }

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr) {
        Include::cleanup(font);
        logSDLError(std::cerr, "TTF_RenderText_Blended");
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surface);
    if (tex == nullptr) {
        Include::cleanup(surface, font);
        logSDLError(std::cerr, "SDL_CreateTextureFromSurface");
        return nullptr;
    }

    Include::cleanup(surface, font);
    return tex;
}

