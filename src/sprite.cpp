#include "sprite.hpp"
#include <iostream>
using namespace std;

Sprite::Sprite(int tX, int tY, SDL_Texture* txtr, const TileMatrix& tileMtrx, map<Direction, SDL_Rect> srcRects, AgentType agntType)
  : x(tX*8)
  , y(tY*8)
  , tileX(tX)
  , tileY(tY)
  , texture(txtr)
  , tileMatrix(tileMtrx)
  , sourceRects(srcRects)
  , agentType(agntType)
{
  tileMatrix.registerMovement(vertex(-1,-1), vertex(tileX, tileY), agentType);
}

vertex Sprite::getTileFacing() const {
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
  if (!(tileMatrix.isTraversableTile(dest)) || tileMatrix.isOccupiedTile(dest)) {
    return false;
  }
  isMoving = true;
  tileMatrix.registerMovement(vertex(tileX, tileY), dest, agentType);
  dirMoving = d;
  tileX = dest.first;
  tileY = dest.second;
  return true;
}

bool Sprite::moveToTile(const vertex& dest) {
  if (dest.first == tileX - 1) return move(LEFT);
  if (dest.first == tileX + 1) return move(RIGHT);
  if (dest.second == tileY - 1) return move(UP);
  if (dest.second == tileY + 1) return move(DOWN);
  cout << "Sprite::moveToTile - not adjacent: " << dest.first << "," << dest.second << endl;
}

bool Sprite::moveRand() {
  return move(static_cast<Direction>(rand() % 4));
}

void Sprite::face(Direction d) {
  dirFacing = d;
}

void Sprite::draw(SDL_Renderer *r) {
  SDL_Rect agentDest{x,y,8,8};
  if (!tileMatrix.isVisibleTile(vertex(tileX,tileY))) return;
  if (colorModActive) {
    SDL_SetTextureColorMod(texture, colorMod.r, colorMod.b, colorMod.g);
  }
  SDL_Rect agentSrc = sourceRects[dirFacing];
  if (isMoving) {
    agentSrc.x += 8;
    //if (step) {
      //agentSrc.x += 8;
    //}
  }
  else if (isAttacking) {
    agentSrc.x += 16;
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
  }
  if (frameCount == 1) {
    isMoving = false;
    isAttacking = false;
    x = tileX *8;
    y = tileY *8;
  }
}

void Sprite::setTilePosition(int _x, int _y) {
  tileX = _x;
  tileY = _y;
  x = 8*tileX;
  y = 8*tileY;
}

