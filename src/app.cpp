#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdexcept>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "app.hpp"
#include "init.hpp"
#include "agent.hpp"
#include "direction.hpp"
#include "TextManager.hpp"
#include "const.hpp"
#include "TileMatrix.hpp"
#include "Graph.hpp"
using namespace std;

static SDL_Window *gWindow = nullptr;
static SDL_Renderer *gRenderer = nullptr;
static TTF_Font *gFont = nullptr;
static map<string, Mix_Chunk*> soundFx;

void App::init() {
  cout << "---initializing---" << endl;
  if (!sdl_init(&gWindow, &gRenderer)) throw runtime_error("--sdl_init");
  if (!load_font(&gFont)) throw runtime_error("--load_font");
}
void App::close() {
  cout << "App::close" << endl;
  TTF_CloseFont(gFont);
  SDL_DestroyRenderer(gRenderer); 
  gRenderer = nullptr;
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;

  TTF_Quit();
  SDL_Quit();
  Mix_Quit();
  for (auto e : soundFx) {
    if (e.second) {
      Mix_FreeChunk(e.second);
      e.second = nullptr;
    }
  }
}

class TextureContainer {
public:
  map<string, SDL_Texture*> data;
  ~TextureContainer() {
    for (const auto& p: data) {
      SDL_DestroyTexture(p.second);
    }
  }
};

SDL_Texture* create_texture(const char* path) {
  SDL_Surface* surface = SDL_LoadBMP(path);
  if (!surface) {
    throw runtime_error("failed to create surface");
  }
  SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0xff,0xff,0xff));
  SDL_Texture* texture = SDL_CreateTextureFromSurface(
    gRenderer, surface
  );
  if (!texture) {
    throw runtime_error("failed to create texture");
  }
  SDL_FreeSurface(surface);
  return texture;
}

void App::mainloop() {
  int actionCooldown(0);
  bool gameRunning(true);
  TileMatrix tileMatrix("src/test.map");
  Graph graph(tileMatrix); // <-- graph.dijkstra(vertex, vertex)
                           // returns pair<int distance, list<vertex> path> 
  TextureContainer textureContainer;
  textureContainer.data.emplace("dwarf", create_texture("../assets/dwarf.bmp"));
  textureContainer.data.emplace("goblin", create_texture("../assets/goblin.bmp"));
  textureContainer.data.emplace("items", create_texture("../assets/items.bmp"));
  textureContainer.data.emplace("stone", create_texture("../assets/stone.bmp"));
  auto drawStone = [&textureContainer](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{0,0,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["stone"], &src, &dest);
  };
  auto drawGrass = [&textureContainer](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{8,0,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["stone"], &src, &dest);
  };
  auto drawGrassAlt = [&textureContainer](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{8,8,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["stone"], &src, &dest);
  };
  auto drawFloor = [&textureContainer](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{16,0,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["stone"], &src, &dest);
  };
  auto drawHeart = [&textureContainer](int x, int y){
    SDL_Rect src{0,0,8,8};
    SDL_Rect dest{x,y,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["items"], &src, &dest);
  };
  auto drawGoldPiece = [&textureContainer](int x, int y){
    SDL_Rect src{8,0,8,8};
    SDL_Rect dest{x,y,8,8};
    SDL_RenderCopy(gRenderer, textureContainer.data["items"], &src, &dest);
  };
  soundFx["die"] = Mix_LoadWAV("../assets/audio/die.wav");
  soundFx["slash"] = Mix_LoadWAV("../assets/audio/slash.wav");
  soundFx["block"] = Mix_LoadWAV("../assets/audio/block.wav");
  soundFx["heart"] = Mix_LoadWAV("../assets/audio/heart.wav");
  soundFx["gold"] = Mix_LoadWAV("../assets/audio/gold.wav");
  soundFx["song_a"] =Mix_LoadWAV("../assets/audio/song_a.wav");
  soundFx["song_b"] =Mix_LoadWAV("../assets/audio/song_b.wav");
  TextManager::GlyphCache glyphcache(gRenderer, gFont);
  TextManager::TextArea textArea(gRenderer, 0, 18*8, 20*8, 10);
  ostringstream oss;
  const Uint8 *keystate = nullptr;
  Player player(
    "player",
    0,0,
    100, 100,
    Stats(),
    textureContainer.data["dwarf"], 
    tileMatrix,
    map<Direction, SDL_Rect>{
      {UP,    SDL_Rect{0,0,8,8}},
      {DOWN,  SDL_Rect{16,0,8,8}},
      {LEFT,  SDL_Rect{32,0,8,8}},
      {RIGHT, SDL_Rect{48,0,8,8}}
  });
  SDL_Event e;
  bool quit {false};

  while (!quit) {
    if (actionCooldown == 0) {
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
        bool playerMoved {false};
        if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
          switch(e.key.keysym.sym) {
            case SDLK_w:
              playerMoved = player.sprite.move(UP);
              break;
            case SDLK_a:
              playerMoved = player.sprite.move(LEFT);
              break;
            case SDLK_s:
              playerMoved = player.sprite.move(DOWN);
              break;
            case SDLK_d:
              playerMoved = player.sprite.move(RIGHT);
              break;
            case SDLK_e:
              break;
            case SDLK_p:
              break;
            case SDLK_1:
              break;
          }
        }
        if (playerMoved) {
          actionCooldown = 8;
          tileMatrix.updateVisibilityMap(player.coords());
        }
      }
    } else {
      // action cooldown, location updates
      player.sprite.advanceAnimation(actionCooldown);
      --actionCooldown;
    }
    // render player and map // should be handled by TileMatrix
    for (int h=0; h < tileMatrix.height(); ++h) {
      for (int w=0; w < tileMatrix.width(); ++w) {
        if (!tileMatrix.isVisibleTile(vertex(w,h))) {
          SDL_SetRenderDrawColor(gRenderer,0,0,0,0xff);
          SDL_Rect dest{w*8,h*8,8,8};
          SDL_RenderFillRect(gRenderer, &dest);
        }
        else {
          int tilecode = tileMatrix.at(vertex(w,h));
          switch (tilecode) {
            case 0:
              drawGrass(w*8,h*8);
              break;
            case 3:
              drawGrassAlt(w*8,h*8);
              break;
            case 1:
              drawStone(w*8,h*8);
              break;
            case 2:
              drawFloor(w*8,h*8);
              break;
          }
        }
      }
    }
    textArea.drawRect();
    oss.str(string());
    oss << "work in progress!" << endl;
    textArea.renderPrint(glyphcache, oss.str().c_str());
    player.sprite.draw(gRenderer);
    SDL_RenderPresent(gRenderer);
  }
}
