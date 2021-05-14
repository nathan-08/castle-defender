#pragma once
#include <map>
#include "vertex.hpp"
#include "direction.hpp"
#include "TileMatrix.hpp"
#include "SDL.h"

struct Sprite {
private:
  vertex getTileFacing();
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
  bool isMoving = false;
  Direction dirFacing = DOWN;
  Direction dirMoving = DOWN;

  Sprite(int tX, int tY, SDL_Texture* txtr, const TileMatrix& tileMtrx, std::map<Direction, SDL_Rect> sourceRects);
  bool move(Direction, bool faceDirection = true);
  void face(Direction);
  void draw(SDL_Renderer*);
  void advanceAnimation(int frameCounter);
  void setTilePosition(int, int);
  inline vertex coords() const { return vertex(tileX, tileY); }
};
