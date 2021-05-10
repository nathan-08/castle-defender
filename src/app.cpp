// TODO: handle player death, pause function
// 
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
  int goldCount(0);
  int level(0);
  bool paused(false);
  bool gameRunning(false);
  TileMatrix tileMatrix("src/a.map");
  cout << tileMatrix.width() << " " << tileMatrix.height() << endl;
  Graph graph(tileMatrix); // <-- graph.dijkstra(vertex, vertex)
                           // returns pair<int distance, list<vertex> path> 
  SDL_Texture* dwarf_texture = create_texture("../assets/dwarf.bmp");
  SDL_Texture* goblin_texture = create_texture("../assets/goblin.bmp");
  SDL_Texture* items_texture = create_texture("../assets/items.bmp");
  SDL_Texture* stone = create_texture("../assets/stone.bmp");
  auto drawStone = [&stone](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{0,0,8,8};
    SDL_RenderCopy(gRenderer, stone, &src, &dest);
  };
  auto drawGrass = [&stone](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{8,0,8,8};
    SDL_RenderCopy(gRenderer, stone, &src, &dest);
  };
  auto drawGrassAlt = [&stone](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{8,8,8,8};
    SDL_RenderCopy(gRenderer, stone, &src, &dest);
  };
  auto drawFloor = [&stone](int x, int y){
    SDL_Rect dest{x,y,8,8};
    SDL_Rect src{16,0,8,8};
    SDL_RenderCopy(gRenderer, stone, &src, &dest);
  };
  auto drawHeart = [&items_texture](int x, int y){
    SDL_Rect src{0,0,8,8};
    SDL_Rect dest{x,y,8,8};
    SDL_RenderCopy(gRenderer, items_texture, &src, &dest);
  };
  auto drawGoldPiece = [&items_texture](int x, int y){
    SDL_Rect src{8,0,8,8};
    SDL_Rect dest{x,y,8,8};
    SDL_RenderCopy(gRenderer, items_texture, &src, &dest);
  };
  vector<Agent> npcs;
  vector<Item> items;
  vector<Projectile> projectiles;
  auto makeSpear = [&projectiles, &items_texture, &tileMatrix](
    int x, int y, Direction dir){
    projectiles.emplace_back(SDL_Color{0xff,0,0,0xff},x,y,&tileMatrix,
      map<Direction, SDL_Rect>{
          {UP, SDL_Rect{8*2,0,8,8}},
          {DOWN, SDL_Rect{8*3,0,8,8}},
          {LEFT, SDL_Rect{8*5,0,8,8}},
          {RIGHT, SDL_Rect{8*4,0,8,8}}
      }, items_texture, dir);
  };
  auto makeHeart = [&items, &items_texture, &tileMatrix](int x, int y){
    items.emplace_back(SDL_Color{0xff,0,0,0xff},x,y,&tileMatrix,
    map<Direction, SDL_Rect>{
        {UP, SDL_Rect{0,0,8,8}},
        {DOWN, SDL_Rect{0,0,8,8}},
        {LEFT, SDL_Rect{0,0,8,8}},
        {RIGHT, SDL_Rect{0,0,8,8}}
    }, items_texture, 0);
  };
  auto makeGold = [&items, &items_texture, &tileMatrix](int x, int y){
    items.emplace_back(SDL_Color{0xff,0,0,0xff},x,y,&tileMatrix,
    map<Direction, SDL_Rect>{
        {UP, SDL_Rect{8,0,8,8}},
        {DOWN, SDL_Rect{8,0,8,8}},
        {LEFT, SDL_Rect{8,0,8,8}},
        {RIGHT, SDL_Rect{8,0,8,8}}
    }, items_texture, 1);
  };
  auto makeGoblin = [&npcs, &goblin_texture, &tileMatrix](int x, int y){
    npcs.emplace_back(SDL_Color{0xff,0,0,0xff},x,y,&tileMatrix,
    map<Direction, SDL_Rect>{
        {UP, SDL_Rect{0,0,8,8}},
        {DOWN, SDL_Rect{16,0,8,8}},
        {LEFT, SDL_Rect{32,0,8,8}},
        {RIGHT, SDL_Rect{48,0,8,8}}
    }, goblin_texture);
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
  TextManager::TextArea messageTextArea(gRenderer, 0, 8*3, 20*8, 10);
  ostringstream oss;

  const Uint8 *keystate = nullptr;
  map<Direction, SDL_Rect> sword_map{
    {UP,   SDL_Rect{0,16,8,8}},
    {DOWN, SDL_Rect{16,16,8,8}},
    {LEFT, SDL_Rect{32,16,8,8}},
    {RIGHT,SDL_Rect{48,16,8,8}},
  };
  Agent player(
      SDL_Color{0,0,0,0xff},
      8,6,
      &tileMatrix,
      map<Direction, SDL_Rect>{
      {UP,    SDL_Rect{0,0,8,8}},
      {DOWN,  SDL_Rect{16,0,8,8}},
      {LEFT,  SDL_Rect{32,0,8,8}},
      {RIGHT, SDL_Rect{48,0,8,8}}
      }, dwarf_texture, sword_map);
  bool quit = false;
  SDL_Event e;
  auto startGame = [&player, &level, &gameRunning, &paused, &npcs, &items](){
    level = 0;
    gameRunning = true;
    paused = false;
    npcs.clear();
    items.clear();
    Mix_PlayChannel(2, soundFx["song_a"], -1);
    player.setTilePosition(8,6);
    player.actions.setAll(false);
    player.frameCount=0;
    player.dirMoving=DOWN;
    player.dirFacing=DOWN;
    player.health = 3;
  };
  //startGame();
  while (!quit) {
    if (npcs.empty()) {
      ++level;
      while (npcs.size() < level) {
        int tileX, tileY;
        if (rand()%2==0){
          tileX = tileMatrix.width()-1-npcs.size();
          tileY = tileMatrix.height()-1;
        }
        else {
          tileY = npcs.size();
          tileX = 0;
        }
        makeGoblin(tileX, tileY);
      }
    }
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      else if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch(e.key.keysym.sym) {
          case SDLK_e:
            if (player.attack()) {
              Mix_HaltChannel(0);
              Mix_PlayChannel(0, soundFx["slash"], 0);
            }
            break;
          case SDLK_p:
            if (!gameRunning) {
              startGame();
            }
            else {
              paused = !paused;
              if (paused) Mix_Pause(2);
              else Mix_Resume(2);
            }
            break;
          case SDLK_1:
            if (player.ready()) {
              vertex vert = player.getFacingVertex();
              if (projectiles.empty())
                makeSpear(vert.first, vert.second, player.dirFacing);
            }
            break;
        }
      }
    }
    if (!paused && gameRunning) {
      vertex playerCoords = player.act();
      // check for damage to player
      for (int i = 0; i < npcs.size(); ++i) {
        Agent& npc = npcs.at(i);
        if (npc.tileX == playerCoords.first && npc.tileY == playerCoords.second) {
          int result = player.takeDamage(npc);
          if (result == 0) { // <- check for 0 remaining health
            // game over condition
            gameRunning=false;
            Mix_PlayChannel(2, soundFx["song_b"], 0);
          }
          if (result == -1) {
            // blocked
            Mix_HaltChannel(0);
            Mix_PlayChannel(0, soundFx["block"], 0);
            continue;
          }
          npc.moveBack();
          Mix_HaltChannel(0);
          Mix_PlayChannel(0, soundFx["slash"], 0);
        }
      }
      for (int i=0; i < items.size(); ++i) {
        // check for pick up item
        Item& item = items.at(i);
        if (item.tileX == playerCoords.first && item.tileY == playerCoords.second) {
          items.erase(items.begin() + i);
          switch (item.id) {
            case 0: // heart
              if (player.health < 3) ++player.health;
              // play sound
              Mix_HaltChannel(0);
              Mix_PlayChannel(0, soundFx["heart"], 0);
              break;
            case 1: // gold
              ++goldCount;
              // play sound
              Mix_HaltChannel(0);
              Mix_PlayChannel(0, soundFx["gold"], 0);
              break;
          }

        }
      }
      keystate = SDL_GetKeyboardState(nullptr);
      if (keystate[SDL_SCANCODE_W]) {
        keystate[SDL_SCANCODE_RSHIFT] ? player.face(UP) : player.move(UP);
      }
      if (keystate[SDL_SCANCODE_A]) {
        keystate[SDL_SCANCODE_RSHIFT] ? player.face(LEFT) : player.move(LEFT);
      }
      if (keystate[SDL_SCANCODE_S]) {
        keystate[SDL_SCANCODE_RSHIFT] ? player.face(DOWN) : player.move(DOWN);
      }
      if (keystate[SDL_SCANCODE_D]) {
        keystate[SDL_SCANCODE_RSHIFT] ? player.face(RIGHT) : player.move(RIGHT);
      }
      if (keystate[SDL_SCANCODE_RSHIFT]) {
        player.block();
      }
      else player.actions.blocking = false;
    }
    SDL_SetRenderDrawColor(gRenderer,0,0,0,0xff);
    SDL_RenderClear(gRenderer);

    // render player and map
    for (int h=0; h < tileMatrix.height(); ++h) {
      for (int w=0; w < tileMatrix.width(); ++w) {
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
    textArea.drawRect();
    oss.str(string());
    oss << "level: " << level << " gold: " << goldCount << endl;
    const char* statusText = oss.str().c_str();
    int textWidth;
    glyphcache.sizeText(statusText, &textWidth, nullptr);
    textArea.renderPrint(glyphcache, statusText);
    for (int i = 0; i < player.health; ++i) {
      drawHeart(textWidth + 1 + 8*i, 18*8+1);
    }
    for (int i = 0; i < npcs.size(); ++i) {
      Agent& agent = npcs.at(i);
      if (!paused && gameRunning) {
        vertex npcCoords = agent.act();
        vertex playerCoords = vertex(player.tileX, player.tileY);
        // check for damage
        bool damageTaken = false;
        for (int j=0; j < projectiles.size(); ++j) {
          auto& projectile = projectiles.at(j);
          vertex projectileCoords = vertex(projectile.tileX, projectile.tileY);
          if (npcCoords == projectileCoords) {
            // projectile hit
            damageTaken = true;
            agent.takeDamage(projectile);
            projectiles.erase(projectiles.begin() + j);
          }
        }
        if (player.actions.attacking && !agent.actions.takingDamage) {
          vertex weaponCoords = player.weaponCoords();
          if (npcCoords.first == weaponCoords.first
              && npcCoords.second == weaponCoords.second
          ) {
            damageTaken = true;
            agent.takeDamage(player);
          }
        }
        if (damageTaken) {
          if (agent.health <= 0) {
            if (rand() % 100 < 50)
              makeHeart(agent.tileX, agent.tileY);
            else 
              makeGold(agent.tileX, agent.tileY);
            npcs.erase(npcs.begin() + i);
            --i;
            Mix_HaltChannel(1);
            Mix_PlayChannel(1, soundFx["die"], 0);
            continue;
          }
        }
        if (agent.ready()) {
          // move
          if (rand() % 2 == 0) {
            if (rand() % 2 == 0) {
              auto [distance, path] = graph.dijkstra(npcCoords, playerCoords);
              //int distance = dspResult.first;
              //list<vertex> path = dspResult.second;
              if (distance > -1) {
                path.pop_front();
                agent.moveToVertex(path.front());
              }
            } else {
              agent.moveRand();
            }
          }
          else {
            agent.wait(1);
          }
        }
      }
      agent.draw(gRenderer);
    }
    for (auto& item: items) {
      item.draw(gRenderer);
    }
    for (int i=0; i < projectiles.size(); ++i) {
      auto& projectile = projectiles.at(i);
      if (!paused && gameRunning) {
        vertex result = projectile.act();
        if (result.first == -1) {
          projectiles.erase(projectiles.begin() + i);
          --i;
          continue;
        }
      }
      projectile.draw(gRenderer);
    }
    player.draw(gRenderer);
    if (!gameRunning && player.health == 3) {
      messageTextArea.printMultilineCenter(
        glyphcache,
        vector<string>{
          " Defend the keep! ",
          " p - start/pause/resume ",
          " wasd - move ",
          " e - sword ",
          " 1 - throw spear "
        },
        tileMatrix.width(),
        tileMatrix.height()
      );
    }
    else if (paused) {
      messageTextArea.printCenter(glyphcache, " Paused ", tileMatrix.width(), tileMatrix.height());
    }
    else if (!gameRunning) {
      messageTextArea.printCenter(glyphcache, " Game Over ", tileMatrix.width(), tileMatrix.height());
    }
    SDL_RenderPresent(gRenderer);
  }
  SDL_DestroyTexture(dwarf_texture);
  SDL_DestroyTexture(goblin_texture);
}
