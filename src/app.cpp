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
  gFont = nullptr;
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

struct TextureContainer {
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
  TileMatrix tileMatrix("src/a.map");
  Graph graph(tileMatrix); // <-- graph.dijkstra(vertex, vertex)
                           // returns pair<int distance, list<vertex> path> 
  TextureContainer textureContainer;
  textureContainer.data.emplace("dwarf", create_texture("../assets/dwarf.bmp"));
  textureContainer.data.emplace("boy", create_texture("../assets/boy.bmp"));
  textureContainer.data.emplace("goblin", create_texture("../assets/goblin.bmp"));
  textureContainer.data.emplace("skelet", create_texture("../assets/skelet.bmp"));
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
    textureContainer.data["boy"], 
    tileMatrix,
    map<Direction, SDL_Rect>{
      {UP,    SDL_Rect{0,0,8,8}},
      {DOWN,  SDL_Rect{0,8,8,8}},
      {LEFT,  SDL_Rect{0,8,8,8}},
      {RIGHT, SDL_Rect{0,0,8,8}}
  });
  vector<Enemy> enemies;
  for (int i=0; i < 5; ++i) {
    enemies.emplace_back(
      "skelet",
      6+i,8,
      100, 100,
      Stats(),
      textureContainer.data["skelet"], 
      tileMatrix,
      map<Direction, SDL_Rect>{
        {UP,    SDL_Rect{0,0,8,8}},
        {DOWN,  SDL_Rect{0,8,8,8}},
        {LEFT,  SDL_Rect{0,8,8,8}},
        {RIGHT, SDL_Rect{0,0,8,8}}
    });
  }
  SDL_Event e;
  bool quit {false};

  while (!quit) {
    if (actionCooldown == 0) {
      bool playerMoved {false};
      bool playerAttacked {false};
      bool moveAttempt {false};
      Direction inputDir;
      while (SDL_PollEvent(&e) != 0) {
        moveAttempt = false;
        if (e.type == SDL_QUIT) {
          quit = true;
        }
        if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
          switch(e.key.keysym.sym) {
            case SDLK_w:
              inputDir = UP;
              moveAttempt = true;
              break;
            case SDLK_a:
              inputDir = LEFT;
              moveAttempt = true;
              break;
            case SDLK_s:
              inputDir = DOWN;
              moveAttempt = true;
              break;
            case SDLK_d:
              inputDir = RIGHT;
              moveAttempt = true;
              break;
          }
        }
        if (moveAttempt) {
          // determine if attacking adjacent enemy
          player.sprite.face(inputDir);
          vertex facing = player.sprite.getTileFacing();
          auto target = find_if(
              enemies.begin(), enemies.end(),
              [&facing](const Enemy& e){ return e.coords() == facing; }
          );
          if (target != enemies.end()) {
            // player is attacking adjacent enemy
            playerAttacked = true;
            player.attack(*target);
            cout << "player attacks " << (*target).name << (*target).id << endl;
            // execute enemy attacks and movements
            for (auto& enemy: enemies) {
              // if enemy is adjacent (nearest path length == 1) to player
              // attack player
              auto dspResult = graph.dijkstra(enemy.coords(), player.coords());
              if (dspResult.first == 1) {
                enemy.attack(player);
                cout << enemy.name << enemy.id << " attacks player" << endl;
              }
              // move towards player
              else if (dspResult.first < 20 && dspResult.first > -1 /* && rand() % 4 */) {
                dspResult.second.pop_front();
                cout << "dsp: enemy " << enemy.id << " moving to " << dspResult.second.front().first << ","
                     << dspResult.second.front().second << endl;
                enemy.sprite.moveToTile(dspResult.second.front());
              }
            }
          }
          else {
            // player is moving into open square
            // execute enemy attacks
            map<int, bool> enemyAttacked;
            for (auto& enemy: enemies) {
              // if enemy is adjacent (nearest path length == 1) to player
              // attack player
              auto dspResult = graph.dijkstra(enemy.coords(), player.coords());
              if (dspResult.first == 1) {
                enemy.attack(player);
                cout << enemy.name << enemy.id << " attacks player" << endl;
                enemyAttacked.emplace(enemy.id, true);
              }
            }
            playerMoved = player.sprite.move(inputDir);
            // execute enemy movements
            for (auto& enemy: enemies) {
              if (enemyAttacked.count(enemy.id)) continue;
              // if in aggro range of player (nearest path length < 4)
              //      randselect(move towards player, move random dir)
              //      if target location is occupied by other player, do nothing
              auto dspResult = graph.dijkstra(enemy.coords(), player.coords());
              if (dspResult.first < 20 && dspResult.first > -1 /* && rand() % 4 */) {
                dspResult.second.pop_front();
                cout << "dsp: enemy " << enemy.id << " moving to " << dspResult.second.front().first << ","
                     << dspResult.second.front().second << endl;
                enemy.sprite.moveToTile(dspResult.second.front());
              } else {
                //enemy.sprite.moveRand();
              }
            }
          }
        }
        if (playerMoved || playerAttacked) {
          actionCooldown = 8;
        }
        if (playerMoved) {
          tileMatrix.updateVisibilityMap(player.coords());
        }
      }
    } else {
      // action cooldown, location updates
      player.sprite.advanceAnimation(actionCooldown);
      for (auto& enemy: enemies) {
        enemy.sprite.advanceAnimation(actionCooldown);
      }
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
    oss << player.sprite.tileX << ", " << player.sprite.tileY << " | "
        << enemies.at(0).sprite.tileX << "," << enemies.at(0).sprite.tileY << " | "
        << enemies.at(1).sprite.tileX << "," << enemies.at(1).sprite.tileY << endl;
    textArea.renderPrint(glyphcache, oss.str().c_str());
    for (auto& enemy : enemies) {
      enemy.sprite.draw(gRenderer);
    }
    player.sprite.draw(gRenderer);
    SDL_RenderPresent(gRenderer);
  }
}

