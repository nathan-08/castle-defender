#pragma once
#include <map>
#include <string>
#include <utility> // pair
#include "direction.hpp"
#include "SDL.h"
#include "TileMatrix.hpp"
#include "sprite.hpp"
#include "vertex.hpp"
#include "AgentType.hpp"

struct Stats {};
struct EquippedItems {};

class Agent {
private:
  static inline int nextId=0;
protected: // abstract base class
  Agent(
      std::string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
      SDL_Texture* txtr, const TileMatrix& tileMtrx, std::map<Direction, SDL_Rect> srcRects,
      AgentType agntType
  );
public:
  int id;
  std::string name;
  int currentHp;
  int currentMp;
  int maxHp;
  int maxMp;
  Stats baseStats;
  Sprite sprite;
  bool engaged = false;
  EquippedItems equipment = EquippedItems();

  bool attack(const Agent&);
  vertex inline coords() const { return vertex{sprite.tileX, sprite.tileY}; }
};

class Player : public Agent {
public:
  Player(
      std::string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
      SDL_Texture* txtr, const TileMatrix& tileMtrx, std::map<Direction, SDL_Rect> srcRects
  );
};

class Enemy : public Agent {
public:
  Enemy(
      std::string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
      SDL_Texture* txtr, const TileMatrix& tileMtrx, std::map<Direction, SDL_Rect> srcRects
  );
};

