#include "agent.hpp"
#include "const.hpp"
#include <iostream>
using namespace std;

Agent::Agent(
    string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
    SDL_Texture* txtr, const TileMatrix& tileMtrx, map<Direction, SDL_Rect> srcRects,
    AgentType agntType
)
  : id(Agent::nextId++)
  , name(nm)
  , currentHp(mxHp)
  , currentMp(mxMp)
  , maxHp(mxHp)
  , maxMp(mxMp)
  , baseStats(stats)
  , sprite(tX, tY, txtr, tileMtrx, srcRects, agntType)
{};

bool Agent::attack(const Agent& target) {
  sprite.isAttacking = true;
}

Player::Player(
  string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
  SDL_Texture* txtr, const TileMatrix& tileMtrx, map<Direction, SDL_Rect> srcRects
)
: Agent(nm, tX, tY, mxHp, mxMp, stats, txtr, tileMtrx, srcRects, PLAYER)
{}

Enemy::Enemy(
  string nm, int tX, int tY, int mxHp, int mxMp, Stats stats,
  SDL_Texture* txtr, const TileMatrix& tileMtrx, map<Direction, SDL_Rect> srcRects
)
: Agent(nm, tX, tY, mxHp, mxMp, stats, txtr, tileMtrx, srcRects, ENEMY)
{}

