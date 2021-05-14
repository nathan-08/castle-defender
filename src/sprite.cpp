#include "sprite.hpp"
using namespace std;

Sprite::Sprite(int tX, int tY, SDL_Texture* txtr, const TileMatrix& tileMtrx, map<Direction, SDL_Rect> srcRects)
  : x(tX*8)
  , y(tY*8)
  , tileX(tX)
  , tileY(tY)
  , texture(txtr)
  , tileMatrix(tileMtrx)
  , sourceRects(srcRects)
{}

vertex Sprite::getTileFacing() {
  vertex result(tileX, tileY);
  switch (dirFacing) {
    case UP:    --result.second; break;
    case DOWN:  ++result.second; break;
    case LEFT:  --result.first;  break;
    case RIGHT: ++result.first;  break;
  }
  return result;
}

bool Sprite::move(Direction d, bool faceDirection) {
  if (faceDirection) {
    dirFacing = d;
  }
  vertex dest = getTileFacing();
  if (!(tileMatrix.isTraversableTile(dest))) {
    return false;
  }
  isMoving = true;
  dirMoving = d;
  tileX = dest.first;
  tileY = dest.second;
  return true;
}

void Sprite::face(Direction d) {
  dirFacing = d;
}

void Sprite::draw(SDL_Renderer *r) {
  SDL_Rect agentDest{x,y,8,8};
  if (colorModActive) {
    SDL_SetTextureColorMod(texture, colorMod.r, colorMod.b, colorMod.g);
  }
  SDL_Rect agentSrc = sourceRects[dirFacing];
  if (isMoving) {
    //agentSrc.y += 8;
    //if (step) {
      //agentSrc.x += 8;
    //}
  }
  SDL_RenderCopy(r, texture, &agentSrc, &agentDest);
}

void Sprite::advanceAnimation(int frameCount) {
  if (isMoving) {
    switch (dirMoving) {
      case UP:    --y; break;
      case DOWN:  ++y; break;
      case LEFT:  --x; break;
      case RIGHT: ++x; break;
    }
    if (frameCount == 1) {
      isMoving = false;
    }
  }
}

void Sprite::setTilePosition(int _x, int _y) {
  tileX = _x;
  tileY = _y;
  x = 8*tileX;
  y = 8*tileY;
}



