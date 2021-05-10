// Heap order property: children always less than or equal to parent
// Heap structure property: (balanced)
#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility> // pair
#include <stdexcept>

// ListNode -> VertexWrapper
// ipair -> vertex
template<typename T>
class PriorityQueue {
private:

  std::vector<std::pair<int, T>> data;

  int getLeftChildIdx(int idx) {
    int result = idx * 2 + 1;
    if (result < data.size()) {
      return result;
    }
    return -1;
  }
  int getRightChildIdx(int idx) {
    int result = idx * 2 + 2;
    if (result < data.size()) {
      return result;
    }
    return -1;
  }
  int getParentIdx(int idx) {
    int result = (idx - 1)/2;
    if (result >= 0) {
      return result;
    }
    return -1;
  }
  void percUp(int idx) {
    while (1) {
      int parentIdx = (idx-1)/2;
      if (idx == parentIdx) break;
      if (data.at(idx).first < data.at(parentIdx).first) {
        // swap data at idx and parentIdx
        auto temp = data.at(idx);
        data.at(idx) = data.at(parentIdx);
        data.at(parentIdx) = temp;
      }
      idx = parentIdx;
    }
  }
  void percDown(int idx) {
    while (getLeftChildIdx(idx) > -1) {
      int minChildIdx = getMinChildIdx(idx);
      if (data.at(idx).first > data.at(minChildIdx).first) {
        // swap data at idx and minChildIdx
        auto temp = data.at(idx);
        data.at(idx) = data.at(minChildIdx);
        data.at(minChildIdx) = temp;
      }
      else return;
      idx = minChildIdx;
    }
  }
  int getMinChildIdx(int idx) {
    int leftChildIdx = getLeftChildIdx(idx);
    int rightChildIdx = getRightChildIdx(idx);
    if (rightChildIdx == -1) {
      return leftChildIdx;
    }
    if (data.at(leftChildIdx).first < data.at(rightChildIdx).first) {
      return leftChildIdx;
    }
    return rightChildIdx;
  }
public:
  PriorityQueue() {}
  T pop() {
    if (data.empty())
      throw std::runtime_error("heap is empty");
    // swap first and last
    auto result = data.at(0);
    data.at(0) = data.back();
    data.pop_back();

    // reorder
    percDown(0);
    return result.second;
  }
  bool empty() { return data.empty(); }
  void insert(std::pair<int, T> element)  {
    data.push_back(element);
    percUp(data.size() - 1);
  }
  void decreaseKey(T key, int amt) {
    // find key element
    auto found = find_if(data.begin(), data.end(), [key](const auto& dpair){
        return dpair.second == key;
    });
    if (found == data.end()) return;
    if (found->first < amt) throw std::runtime_error("decreaseKey-- key was less than amt");
    found->first = amt;
    percUp(found - data.begin());
  }
};
