
/**
 * @file heap.cpp
 * Implementation of a heap class.
 */
#pragma once
#include "heap.h"
#include <cstddef>

template <class T, class Compare>
class heap;

template <class T, class Compare>
size_t heap<T, Compare>::root() const {
   // @TODO Update to return the index you are choosing to be your root.
   return 1;
}

template <class T, class Compare>
size_t heap<T, Compare>::leftChild(size_t currentIdx) const {
   // @TODO Update to return the index of the left child.
   return currentIdx * 2;
}

template <class T, class Compare>
size_t heap<T, Compare>::rightChild(size_t currentIdx) const {
   // @TODO Update to return the index of the right child.
   return currentIdx * 2 + 1;
}

template <class T, class Compare>
size_t heap<T, Compare>::parent(size_t currentIdx) const {
   // @TODO Update to return the index of the parent.
   return currentIdx / 2;
}

template <class T, class Compare>
bool heap<T, Compare>::hasAChild(size_t currentIdx) const {
   // @TODO Update to return whether the given node has a child
   return currentIdx * 2 < _elems.size();
}

template <class T, class Compare>
size_t heap<T, Compare>::maxPriorityChild(size_t currentIdx) const {
   // @TODO Update to return the index of the child with highest priority
   ///   as defined by higherPriority()
   // function assumes currentIdx has child, specifically leftchild as the tree is a complete tree
   //
   // if no right child, always return left child idx
   if (currentIdx * 2 + 1 >= _elems.size())
      return leftChild(currentIdx);

   // case has 2 children
   if (higherPriority(_elems[leftChild(currentIdx)], _elems[rightChild(currentIdx)]))
      return leftChild(currentIdx);
   return rightChild(currentIdx);
}

template <class T, class Compare>
void heap<T, Compare>::heapifyDown(size_t currentIdx) {
   // @TODO Implement the heapifyDown algorithm.
   //
   if (hasAChild(currentIdx)) {
      size_t priorityChildIdx = maxPriorityChild(currentIdx);
      if (higherPriority(_elems[priorityChildIdx], _elems[currentIdx])) {
         std::swap(_elems[priorityChildIdx], _elems[currentIdx]);
         heapifyDown(priorityChildIdx);
      }
   }
}

template <class T, class Compare>
void heap<T, Compare>::heapifyUp(size_t currentIdx) {
   if (currentIdx == root())
      return;
   size_t parentIdx = parent(currentIdx);
   if (higherPriority(_elems[currentIdx], _elems[parentIdx])) {
      std::swap(_elems[currentIdx], _elems[parentIdx]);
      heapifyUp(parentIdx);
   }
}

template <class T, class Compare>
heap<T, Compare>::heap()
    : _elems(1) {
   // @TODO Depending on your implementation, this function may or may
   ///   not need modifying
}

template <class T, class Compare>
heap<T, Compare>::heap(const std::vector<T> &elems)
    : _elems(elems.size() + 1) {
   // @TODO Construct a heap using the buildHeap algorithm
   for (size_t i = 0; i < elems.size(); ++i) {
      _elems[i + 1] = elems[i];
   }

   for (size_t i = parent(_elems.size() - 1); i >= root(); --i) {
      heapifyDown(i);
   }
}

template <class T, class Compare>
T heap<T, Compare>::pop() {
   // @TODO Remove, and return, the element with highest priority
   if (empty())
      return T();

   std::swap(_elems[root()], _elems.back());

   T temp = _elems.back();
   _elems.pop_back();

   heapifyDown(root());

   return temp;
}

template <class T, class Compare>
T heap<T, Compare>::peek() const {
   // @TODO Return, but do not remove, the element with highest priority
   if (empty())
      return T();
   return _elems[root()];
}

template <class T, class Compare>
void heap<T, Compare>::push(const T &elem) {
   // @TODO Add elem to the heap
   _elems.push_back(elem);
   heapifyUp(_elems.size() - 1);
}

template <class T, class Compare>
void heap<T, Compare>::updateElem(const size_t &idx, const T &elem) {
   // @TODO In-place updates the value stored in the heap array at idx
   // Corrects the heap to remain as a valid heap even after update
   if (root() > idx || idx >= _elems.size())
      return;

   _elems[idx] = elem;
   if (higherPriority(elem, _elems[parent(idx)]))
      heapifyUp(idx);
   else if (hasAChild(idx) && higherPriority(_elems[maxPriorityChild(idx)], elem))
      heapifyDown(idx);
}

template <class T, class Compare>
bool heap<T, Compare>::empty() const {
   // @TODO Determine if the heap is empty
   return _elems.size() <= 1;
}

template <class T, class Compare>
void heap<T, Compare>::getElems(std::vector<T> &heaped) const {
   for (size_t i = root(); i < _elems.size(); i++) {
      heaped.push_back(_elems[i]);
   }
}
