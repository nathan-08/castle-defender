#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include "priorityQueue.hpp"
#include "TileMatrix.hpp"
#include "Graph.hpp"

using namespace std;

Graph::Graph(const TileMatrix& tm) {
  auto getTileCode = [tm](int x, int y){
    return tm.data.at(y).at(x);
  };
  auto isValidCoord = [&tm, &getTileCode](int x, int y) {
    bool result = tm.isTraversableTile(vertex(x,y));
    return result;
  };

  for (int y = 0; y < tm.data.size(); ++y) {
    for (int x = 0; x < tm.data.at(y).size(); ++x) {
      adjLists.emplace(vertex(x,y), list<VertexWrapper>());
    }
  }
  for (auto& vvwlPair: adjLists) {
    const vertex& vtx = vvwlPair.first;
    if (!isValidCoord(vtx.first, vtx.second)) continue;
    list<VertexWrapper>& vwList = vvwlPair.second;
    int x = vtx.first;
    int y = vtx.second;
    // possible neighbors: (x-1,y) (x+1,y) (x,y-1) (x,y+1)
    if (isValidCoord(x-1,y)) vwList.emplace_back(vertex(x-1,y), 1);
    if (isValidCoord(x+1,y)) vwList.emplace_back(vertex(x+1,y), 1);
    if (isValidCoord(x,y-1)) vwList.emplace_back(vertex(x,y-1), 1);
    if (isValidCoord(x,y+1)) vwList.emplace_back(vertex(x,y+1), 1);
  }
}
void Graph::addEdge(vertex srcId, vertex destId, int weight) {
  adjLists[srcId].emplace_back(destId, weight);
  adjLists[destId].emplace_back(srcId, weight); // (non directional graph)
}
list<vertex> Graph::getVertices() {
  list<vertex> result;
  for (auto p : adjLists) {
    result.push_back(p.first);
  }
  return result;
}
list<VertexWrapper> Graph::getConnections(const vertex& vrt) {
  if (adjLists.count(vrt) > 0)
    return adjLists.at(vrt);
  throw runtime_error("not in graph.");
}
int Graph::getWeight(const vertex& src, const vertex& dest) {
  if (adjLists.count(src)) {
    auto& adjList = adjLists[src];
    auto found = find_if(adjList.begin(), adjList.end(), [dest](const VertexWrapper& n){
        return n.dest == dest;
    });
    if (found != adjList.end()) {
      return found->weight;
    }
  }
  throw runtime_error("not in graph");
}
pair<int, list<vertex>> Graph::dijkstra(const vertex& src, const vertex& dest) {
  // returns pair(distance (-1 if no path), path from src to dest inclusive (empty of no path))
  if (adjLists.count(src) == 0) {
    throw runtime_error("not in graph");
  }
  map<vertex, int> distanceMap;
  map<vertex, vertex> predecessorMap;
  PriorityQueue<vertex> pq; // <- priority queue
  for (const auto& vertexListPair: adjLists) {
    vertex vrt = vertexListPair.first;
    distanceMap.emplace(vrt, INT_MAX);
    vertex nullPredecessor(-1,-1);
    predecessorMap.emplace(vrt, nullPredecessor); // <-"null predecessor"
    int priority = vrt == src ? 0 : INT_MAX;
    pq.insert(pair(priority, vrt));
  }
  distanceMap.at(src) = 0;

  while(!pq.empty()) {
    vertex currentVertex = pq.pop();
    if (distanceMap[currentVertex] == INT_MAX) break; // <- this line fixed a bug! \0/
    list<VertexWrapper> connections = adjLists.at(currentVertex);
    for (const auto& connection : connections) {
      int newDist = distanceMap[currentVertex] + connection.weight;
      if (newDist < distanceMap[connection.dest]) {
        distanceMap[connection.dest] = newDist;
        predecessorMap[connection.dest] = currentVertex;
        pq.decreaseKey(connection.dest, newDist);
      }
    }
    // exit loop if currentVertex == dest
    if (currentVertex == dest) break;
  }

  int dist = distanceMap[dest];
  list<vertex> path;
  if (dist == INT_MAX)
    return {-1, path};
  // populate path
  vertex currentVertex = dest;
  while (currentVertex.first > -1) {
    path.push_front(currentVertex);
    currentVertex = predecessorMap[currentVertex];
  }
  return {dist, path};
}

/*
int main() {
  TileMatrix tileMatrix("a.map");
  tileMatrix.print();
  cout << endl;
  Graph graph(tileMatrix);
  pair<int, list<vertex>> result = graph.dijkstra(vertex(0,4), vertex(8,0));
  if (result.first == -1)
    cout << "no path" << endl;
  cout << "distance: " << result.first << endl;
  tileMatrix.printPath(result.second);
}
*/
