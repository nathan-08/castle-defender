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
