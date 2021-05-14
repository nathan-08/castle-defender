#pragma once
#include <list>
#include <map>
#include <vector>
#include <string>
#include <utility> // pair
#include "vertex.hpp"
#include "AgentType.hpp"

class TileMatrix {
private:
  void visMapUtil(const vertex&, int, int);
public:
  std::vector<std::vector<int>> data;
  std::map<vertex, bool> shadowMap;
  mutable std::map<vertex, AgentType> occupiedMap;

  TileMatrix(const std::string& filename);
  int inline width() const { return data.at(0).size(); }
  int inline height() const { return data.size(); }
  void print();
  void printPath(const std::list<vertex>& path);
  int at(const vertex& vrt); // tileCode
  bool isTraversableTile(const vertex& vrt) const;
  std::list<vertex>bresenham(const vertex& p1, const vertex& p2);
  std::list<vertex> updateVisibilityMap(vertex playerCoords);
  bool isVisibleTile(const vertex&) const;
  bool isOccupiedTile(const vertex&) const;
  void registerMovement(vertex from, vertex to, AgentType type) const;
};
