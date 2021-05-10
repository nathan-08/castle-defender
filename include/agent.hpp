#pragma once
#include <map>
#include <utility> // pair
#include "direction.hpp"
#include "SDL.h"
#include "TileMatrix.hpp"
typedef std::pair<int, int> vertex;

struct Actions {
  bool moving=false;
  bool attacking=false;
  bool takingDamage=false;
  bool blocking=false;
  bool stunned=false;
  bool any() {
    return (moving || attacking || takingDamage || blocking || stunned);
  }
  void setAll(bool status) {
    moving=status;
    attacking=status;
    takingDamage=status;
    blocking=status;
    stunned=status;
  }
};

class Agent {
private:
public:
  int tileX = 0;
  int tileY = 0;
  int x = 0;
  int y = 0;
  int health = 3;
  SDL_Color color;
  const TileMatrix* tileMatrix;
  std::map<Direction, SDL_Rect> source_rects;
  std::map<Direction, SDL_Rect> weapon_rects;
  SDL_Texture* texture;
  int frameCount = 0;
  Direction dirFacing = DOWN;
  Direction dirMoving = DOWN;
  bool step=false;

  Actions actions;

  Agent(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix* tileMtrx,
    std::map<Direction, SDL_Rect> rects,
    SDL_Texture* text,
    std::map<Direction, SDL_Rect> weap_rects=std::map<Direction,SDL_Rect>()
  );

  vertex act();
  vertex weaponCoords();
  bool move(Direction d, bool faceDirection=true);
  bool face(Direction d);
  bool moveToVertex(const vertex& vrt);
  void moveBack();
  void moveBackInstant();
  void moveRand();
  vertex getFacingVertex();
  int takeDamage(Agent& enemy);
  bool attack();
  bool block();
  bool ready();
  void draw(SDL_Renderer*);
  void wait(int n=1);
  void stun(int n=1);
  void setTilePosition(int, int);
};

class Player : public Agent {
public:
  Player(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix* tileMtrx,
    std::map<Direction, SDL_Rect> rects,
    SDL_Texture* text,
    std::map<Direction, SDL_Rect> weap_rects=std::map<Direction,SDL_Rect>()
  );
};

class Enemy : public Agent {
public:
  Enemy(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix* tileMtrx,
    std::map<Direction, SDL_Rect> rects,
    SDL_Texture* text,
    std::map<Direction, SDL_Rect> weap_rects=std::map<Direction,SDL_Rect>()
  );
};

class Item : public Agent {
public:
  int id;
  Item(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix* tileMtrx,
    std::map<Direction, SDL_Rect> rects,
    SDL_Texture* text,
    int _id,
    std::map<Direction, SDL_Rect> weap_rects=std::map<Direction,SDL_Rect>()
  );
};

class Projectile : public Agent {
public:
  Projectile(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix* tileMtrx,
    std::map<Direction, SDL_Rect> rects,
    SDL_Texture* text,
    Direction dir
  );
  vertex act();
  void draw(SDL_Renderer*);
};