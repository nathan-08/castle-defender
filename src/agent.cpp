#include "agent.hpp"
#include "const.hpp"
#include <iostream>
using namespace std;

Agent::Agent(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix * tileMtrx,
    map<Direction, SDL_Rect> rects, SDL_Texture* txt,
    map<Direction, SDL_Rect> weap_rects)
  : tileX(_tileX)
  , tileY(_tileY)
  , x(tileX * 8)
  , y(tileY * 8)
  , color(c)
  , tileMatrix(tileMtrx)
  , source_rects(rects)
  , weapon_rects(weap_rects)
  , texture(txt)
{}

bool Agent::moveToVertex(const vertex& vrt) {
  int x = vrt.first;
  int y = vrt.second;
  if (x < tileX) 
    return move(LEFT);
  if (x > tileX)
    return move(RIGHT);
  if (y < tileY)
    return move(UP);
  if (y > tileY)
    return move(DOWN);
  throw runtime_error("Agent::moveToVertex failed");
}

bool Agent::face(Direction d) {
  dirFacing = d;
  return true;
}

bool Agent::move(Direction d, bool faceDirection) {
  if (actions.blocking) return face(d);
  if (faceDirection) {
    dirFacing = d;
  }
  vertex dest(tileX, tileY);
  if (d==UP) --dest.second;
  else if (d==DOWN) ++dest.second;
  else if (d==LEFT) --dest.first;
  else if (d==RIGHT) ++dest.first;
  if (!(tileMatrix->isTraversableTile(dest))) {
    return false;
  }
  if (frameCount > 0) return true;
  step = !step;
  actions.moving = true;
  frameCount = 8;
  dirMoving = d;
  switch (d) {
    case UP: --tileY;
      break;
    case DOWN: ++tileY;
      break;
    case LEFT: --tileX;
      break;
    case RIGHT: ++tileX;
      break;
    default:
      throw runtime_error("unknown direction");
  }
  return true;
}

void Agent::moveRand() {
  int x = rand() % 4;
  move((Direction)x);
}

void Agent::wait(int n) {
  frameCount = n*8;
}

void Agent::moveBackInstant() {
  Direction newDirection;
  if (dirMoving==UP)    { ++tileY; newDirection = DOWN; }
  if (dirMoving==DOWN)  { --tileY; newDirection = UP; }
  if (dirMoving==LEFT)  { ++tileX; newDirection = RIGHT; }
  if (dirMoving==RIGHT) { --tileX; newDirection = LEFT; }
  y = tileY*8;
  x = tileX*8;
  frameCount = 0;
}

void Agent::moveBack() {
  Direction newDirection;
  if (dirMoving==UP)    { ++tileY; newDirection = DOWN; }
  if (dirMoving==DOWN)  { --tileY; newDirection = UP; }
  if (dirMoving==LEFT)  { ++tileX; newDirection = RIGHT; }
  if (dirMoving==RIGHT) { --tileX; newDirection = LEFT; }
  y = tileY*8;
  x = tileX*8;
  frameCount = 0;
  move(newDirection, false);
}

void Agent::stun(int n) {
  moveBackInstant();
  actions.setAll(false);
  actions.stunned = true;
  frameCount = n*8;
}

void Agent::setTilePosition(int _x, int _y) {
  tileX = _x;
  tileY = _y;
  x = 8*tileX;
  y = 8*tileY;
}

vertex Agent::getFacingVertex() {
  if (dirFacing==UP) return vertex{tileX, tileY-1};
  if (dirFacing==DOWN) return vertex{tileX, tileY+1};
  if (dirFacing==LEFT) return vertex{tileX-1, tileY};
  if (dirFacing==RIGHT) return vertex{tileX+1, tileY};
  throw runtime_error("not facing any direction");
}

int Agent::takeDamage(Agent& enemy) {
  if (actions.blocking) {
    // check if enemy is attack from blocked direction
    vertex blkVert = getFacingVertex();
    int enemyX = enemy.tileX;
    int enemyY = enemy.tileY;
    if (enemy.dirMoving == UP) { ++enemyY; }
    if (enemy.dirMoving == DOWN) { --enemyY; }
    if (enemy.dirMoving == LEFT) { ++enemyX; }
    if (enemy.dirMoving == RIGHT) { --enemyX; }
    if (enemyX == blkVert.first && enemyY == blkVert.second) {
      enemy.stun(4);
      return -1;
    }
    else {
      actions.blocking = false;
    }
  }
  --health;
  frameCount = 0;
  actions.takingDamage = true;
  if (actions.moving) {
    // ran into enemy
    moveBack();
  } else {
    // enemy hit us, move away from enemy
    move(enemy.dirFacing, false);
  }
  return health;
}

bool Agent::attack() {
  if (actions.blocking) actions.blocking=false;
  if (!ready()) return false;
  frameCount = 8;
  actions.attacking = true;
  return true;
}

bool Agent::block() {
  if (!ready()) return false;
  actions.blocking = true;
  return true;
}

void Agent::draw(SDL_Renderer *r) {
  SDL_Rect agentDest{x,y,8,8};
  if (actions.takingDamage) {
    SDL_SetTextureColorMod(texture, 0xff,0x80,0x80);
  } else if(actions.stunned) {
    SDL_SetTextureColorMod(texture, 0xff,0xff,0x00);
  } else {
    SDL_SetTextureColorMod(texture, 0xff,0xff,0xff);
  }
  SDL_Rect agentSrc = source_rects[dirFacing];
  if (actions.moving) {
    agentSrc.y += 8;
    if (step) {
      agentSrc.x += 8;
    }
  }
  if (actions.attacking && !weapon_rects.empty()) {
  //draw weapon
    vertex weapCoords = weaponCoords();
    int weapx = weapCoords.first * 8;
    int weapy = weapCoords.second * 8;
    SDL_Rect dest{weapx,weapy,8,8};
    SDL_SetTextureColorMod(texture, 0xff,0xff,0xff);
    SDL_Rect src = weapon_rects[dirFacing];
    if (frameCount > 6) src.y += 8;
    if (frameCount < 3) src.y += 16;
    SDL_RenderCopy(r, texture, &src, &dest);
  }
  // draw shield
  if (actions.blocking && !weapon_rects.empty()) {
    SDL_Rect src = weapon_rects[dirFacing];
    src.y+=8*3;
    int destX=tileX*8;
    int destY=tileY*8;
    if (dirFacing==UP) destY-=4;
    if (dirFacing==DOWN) destY+=4;
    if (dirFacing==LEFT) destX-=2;
    if (dirFacing==RIGHT) destX+=2;
    SDL_Rect dest{destX, destY,8,8};
    if (dirFacing==DOWN) {
      SDL_RenderCopy(r, texture, &agentSrc, &agentDest);
      SDL_RenderCopy(r, texture, &src, &dest);
      return;
    } else {
      SDL_RenderCopy(r, texture, &src, &dest);
    }
  }
  SDL_RenderCopy(r, texture, &agentSrc, &agentDest);
}

pair<int, int> Agent::weaponCoords() {
  int weapx, weapy;
  if (dirFacing == UP) { weapx = tileX; weapy = tileY-1; }
  if (dirFacing == DOWN) { weapx = tileX; weapy = tileY+1; }
  if (dirFacing == LEFT) { weapx = tileX-1; weapy = tileY; }
  if (dirFacing == RIGHT) { weapx = tileX+1; weapy = tileY; }
  return {weapx, weapy};
}

pair<int, int> Agent::act() {
  if (actions.moving) {
    switch (dirMoving) {
      case UP: --y;
        break;
      case DOWN: ++y;
        break;
      case LEFT: --x;
        break;
      case RIGHT: ++x;
        break;
    }
  }
  if (--frameCount <= 0) {
    frameCount = 0;
    if (actions.attacking) {
      frameCount = 4; //cooldown
      //wait(1); // cooldown
    }
    actions.moving = false;
    actions.attacking = false;
    actions.takingDamage = false;
    actions.stunned = false;
  }
  return { tileX, tileY };
}

bool Agent::ready() {
  return (!actions.any() && frameCount == 0);
}

Item::Item(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix * tileMtrx,
    map<Direction, SDL_Rect> rects, SDL_Texture* txt, int _id,
    map<Direction, SDL_Rect> weap_rects)
  : Agent(c, _tileX, _tileY, tileMtrx, rects, txt, weap_rects)
  , id(_id)
{}

Projectile::Projectile(SDL_Color c, int _tileX, int _tileY,
    const TileMatrix * tileMtrx,
    map<Direction, SDL_Rect> rects, SDL_Texture* txt,
    Direction dir)
  : Agent(c, _tileX, _tileY, tileMtrx, rects, txt)
{
  move(dir, true);
}

vertex Projectile::act() {
  vertex result = Agent::act();
  if (frameCount == 0) {
    if (!move(dirFacing, true))
      return {-1,-1};
  }
  return result;
}

void Projectile::draw(SDL_Renderer *r) {
  SDL_Rect agentDest{x,y,8,8};
  if (actions.takingDamage) {
    SDL_SetTextureColorMod(texture, 0xff,0x80,0x80);
  } else if(actions.stunned) {
    SDL_SetTextureColorMod(texture, 0xff,0xff,0x00);
  } else {
    SDL_SetTextureColorMod(texture, 0xff,0xff,0xff);
  }
  SDL_Rect agentSrc = source_rects[dirFacing];
  SDL_RenderCopy(r, texture, &agentSrc, &agentDest);
}