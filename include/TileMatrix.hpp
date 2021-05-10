#pragma once
#include <list>
#include <vector>
#include <string>
#include <utility> // pair
typedef std::pair<int, int> vertex;

class TileMatrix {
public:
  std::vector<std::vector<int>> data;
  TileMatrix(const std::string& filename);
  int inline width() const { return data.at(0).size(); }
  int inline height() const { return data.size(); }
  void print();
  void printPath(const std::list<vertex>& path);
  int at(const vertex& vrt);
  bool isTraversableTile(const vertex& vrt) const;
};