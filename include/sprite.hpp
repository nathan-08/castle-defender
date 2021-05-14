#pragma once
#include <map>
#include "vertex.hpp"
#include "direction.hpp"
#include "TileMatrix.hpp"
#include "AgentType.hpp"
#include "SDL.h"

struct Sprite {
private:
  bool colorModActive {false};
  SDL_Color colorMod {0,0,0,0};
public:
  int x;
  int y;
  int tileX;
  int tileY;
  SDL_Texture* texture;
  const TileMatrix& tileMatrix;
  std::map<Direction, SDL_Rect> sourceRects;
  AgentType agentType;
  bool isMoving = false;
  bool isAttacking = false;
  Direction dirFacing = DOWN;
  Direction dirMoving = DOWN;

  Sprite(int tX, int tY, SDL_Texture* txtr, const TileMatrix& tileMtrx,
      std::map<Direction, SDL_Rect> sourceRects, AgentType agntType);
  bool move(Direction, bool faceDirection = true);
  bool moveToTile(const vertex& dest);
  bool moveRand();
  void face(Direction);
  void draw(SDL_Renderer*);
  void advanceAnimation(int frameCounter);
  void setTilePosition(int, int);
  inline vertex coords() const { return vertex(tileX, tileY); }
  vertex getTileFacing() const;
};
