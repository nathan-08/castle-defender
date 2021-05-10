#pragma once
#include <utility> // pair
#include <map>
#include <list>
#include "TileMatrix.hpp"

typedef std::pair<int, int> vertex;

struct VertexWrapper {
  vertex dest;
  int weight;
  VertexWrapper(vertex d, int w): dest(d), weight(w) {}
};

class Graph {
private:
    std::map<vertex, std::list<VertexWrapper>> adjLists;
public:
    Graph(const TileMatrix& tm);
    void addEdge(vertex src, vertex dest, int weight);
    std::list<vertex> getVertices();
    std::list<VertexWrapper> getConnections(const vertex& vrt);
    int getWeight(const vertex& src, const vertex& dest);
    std::pair<int, std::list<vertex>> dijkstra(const vertex& src, const vertex& dest);
};