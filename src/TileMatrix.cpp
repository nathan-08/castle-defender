#include "TileMatrix.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cctype>
using namespace std;

TileMatrix::TileMatrix(const string& filename) {
  ifstream file(filename);
  if (!file) {
    throw runtime_error("failed to read file.");
  }

  string line;
  for (int counter(0); getline(file, line); ++counter) {
    if (line.empty()) {
        --counter;
        continue;
    }
    data.push_back(vector<int>());
    for (const char c: line) {
      if (!isdigit(c)) continue;
      int x = (c - 0x30); // atoi
      data[counter].push_back(x);
    }
  }
  // populate occupied map
  for (int h = 0; h < data.size(); ++h) {
    for (int w = 0; w < data.at(h).size(); ++w) {
      occupiedMap.emplace(vertex(w,h), NONE);
    }
  }
  occupiedMap.emplace(vertex(-1,-1), NONE); // void tile
}

list<vertex> TileMatrix::bresenham(const vertex& p1, const vertex& p2) {
  list<vertex> result;
  int x1 = p1.first, y1 = p1.second;
  int x2 = p2.first, y2 = p2.second;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int x = x1;
  int y = y1;
  bool x1Greater = x1 > x2;
  bool y1Greater = y1 > y2;

  if (abs(dx) > abs(dy)) {
    // slope < 1
    result.emplace_back(x,y);

    int pk = (2*abs(dy)) - abs(dx);
    for (int i = 0; i < abs(dx); ++i) {
      if (x1Greater) --x;
      else ++x;
      if (pk < 0)
        pk += 2 * abs(dy);
      else {
        if (y1Greater) --y;
        else ++y;
        pk += (2 * abs(dy)) - (2 * abs(dx));
      }
      result.emplace_back(x,y);
    }
  }
  else {
    // slope >= 1
    result.emplace_back(x,y);

    int pk = (2 * abs(dx)) - abs(dy);
    for (int i = 0; i < abs(dy); ++i) {
      if (y1Greater) --y;
      else ++y;
      if (pk < 0)
        pk += 2 * abs(dx);
      else {
        if (x1Greater) --x;
        else ++x;
        pk += (2 * abs(dx)) - (2 * abs(dy));
      }
      result.emplace_back(x,y);
    }
  }
  return result;
}

bool TileMatrix::isTraversableTile(const vertex& vrt) const {
  int x = vrt.first;
  int y = vrt.second;
  if (y < 0 || y > height() - 1 || x < 0 || x > width() - 1) return false;
  if (data.at(y).at(x) == 1) return false;
  return true;
}

void TileMatrix::print() {
  for (const auto& vect: data) {
    for (int x: vect) {
      cout << x;
    }
    cout << endl;
  }
}

void TileMatrix::printPath(const list<vertex>& path) {
  for (int y = 0; y < data.size(); ++y) {
    for (int x = 0; x < data.at(y).size(); ++x) {
      // if path includes vertex(x,y), print '.'
      auto found = find(path.begin(), path.end(), vertex(x,y));
      if (found == path.end()) {
        int code = data.at(y).at(x);
        if (code == 0)
            cout << " ";
        else cout << code;
      }
      else
        cout << ".";
    }
    cout << endl;
  }
}

int TileMatrix::at(const vertex& vrt) {
    int x = vrt.first;
    int y = vrt.second;
    return data.at(y).at(x);
}

void TileMatrix::visMapUtil(const vertex& playerCoords, int x, int y) {
  auto lineOfSight = bresenham(playerCoords, vertex(x,y));
  bool blocked(false);
  for (const auto& vrt: lineOfSight) {
    if (!isTraversableTile(vrt)) {
      blocked = true;
    }
    else if (blocked) {
      shadowMap.emplace(vrt, true);
    }
  }
}

list<vertex> TileMatrix::updateVisibilityMap(vertex playerCoords) {
  // run bresenham for all edges of map (from player) and search for obstacles in lines
  shadowMap.clear();
  list<vertex> result;
  // top side
  for (int x = 0, y = 0; x < width(); ++x) {
    visMapUtil(playerCoords, x, y);
  }
  // left side
  for (int x = 0, y = 1; y < height(); ++y) {
    visMapUtil(playerCoords, x, y);
  }
  // right side
  for (int x = width() - 1, y = 1; y < height(); ++y) {
    visMapUtil(playerCoords, x, y);
  }
  // bottom side
  for (int x = 1, y = height() - 1; x < width() - 1; ++x) {
    visMapUtil(playerCoords, x, y);
  }
  return result; // testing
}

bool TileMatrix::isVisibleTile(const vertex& vrt) const {
  return !(shadowMap.count(vrt) && shadowMap.at(vrt));
}

bool TileMatrix::isOccupiedTile(const vertex& tile) const {
  return (occupiedMap.count(tile) && occupiedMap.at(tile) != NONE);
}

void TileMatrix::registerMovement(vertex from, vertex to, AgentType type) const {
  occupiedMap[from] = NONE;
  occupiedMap[to] = type;
}


