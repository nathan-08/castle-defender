#include <iostream>
#include <utility>
#include <vector>
using namespace std;
typedef pair<int,int> vertex;

vector<vertex> drawline(int x0, int y0, int x1, int y1) {
  int dx, dy, p, x, y;
  vector<vertex> result;

  dx=x1-x0;
  dy=y1-y0;

  x=x0;
  y=y0;

  p=2*dy-dx;

  while (x < x1) {
    if (p >= 0) {
      result.push_back(vertex(x,y));
      ++y;
      p = p + 2*dy - 2*dx;
    }
    else {
      result.push_back(vertex(x,y));
      p = p + 2*dy;
    }
    ++x;
  }
  return result;
}
int main() {
  int x0(0), y0(0), x1(8), y1(2);
  auto vertices = drawline(x0,y0,x1,y1);
  for (const auto& vrt: vertices)
    cout << vrt.first << "," << vrt.second << endl;
}

