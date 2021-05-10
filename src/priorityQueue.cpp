#include <iostream>
#include "priorityQueue.hpp"
using namespace std;

int main() {
  PriorityQueue<int> heap;
  for (int i=30; i > 0; --i) heap.insert(pair<int,int>{i,i});
  heap.decreaseKey(2, 20);
  while (!heap.empty()) {
    int val = heap.pop();
    cout << val << endl;
  }
}
