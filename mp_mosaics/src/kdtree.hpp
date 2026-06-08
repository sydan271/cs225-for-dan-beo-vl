/**
 * @file kdtree.cpp
 * Implementation of KDTree class.
 */

#pragma once
#include "kdtree.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <utility>

using namespace std;

template <int Dim>
bool smallerDimVal(const Point<Dim> &first, const Point<Dim> &second, int curDim) {
   /**
    * @todo Implement this function!
    */
   if (curDim >= Dim) return false;
   if (first[curDim] < second[curDim]) return true;
   if (first[curDim] > second[curDim]) return false;
   return first < second;
}

template <int Dim>
bool shouldReplace(const Point<Dim> &target, const Point<Dim> &currentBest, const Point<Dim> &potential) {
   /**
    * @todo Implement this function!
    */
   double disCur = 0, disPotential = 0;
   for (int i = 0; i < Dim; ++i) {
      disCur += (target[i] - currentBest[i]) * (target[i] - currentBest[i]);
      disPotential += (target[i] - potential[i]) * (target[i] - potential[i]);
   }
   if (disCur == disPotential) return potential < currentBest;

   return disPotential < disCur;
}

template <int Dim>
void KDTree<Dim>::makeTreeRecursively(
    KDTreeNode *&subroot, int dimension, std::vector<Point<Dim>> &points, int left, int right) {
   if (left > right) {
      subroot = nullptr;
      return;
   }

   auto cmp = [dimension](const Point<Dim> first, const Point<Dim> second) {
      return smallerDimVal(first, second, dimension);
   };

   int medianIdx = (left + right) / 2;
   select(points.begin() + left, points.begin() + right + 1, points.begin() + medianIdx, cmp);

   subroot = new KDTreeNode(points[medianIdx]);

   int newDim = (dimension + 1) % Dim;
   makeTreeRecursively(subroot->left, newDim, points, left, medianIdx - 1);
   makeTreeRecursively(subroot->right, newDim, points, medianIdx + 1, right);
}

template <int Dim>
KDTree<Dim>::KDTree(const vector<Point<Dim>> &newPoints)
    : root(nullptr)
    , size(newPoints.size()) {
   /**
    * @todo Implement this function!
    */
   if (newPoints.empty()) return;
   vector<Point<Dim>> points(newPoints);

   makeTreeRecursively(root, 0, points, 0, (int)points.size() - 1);
}

template <int Dim>
KDTree<Dim>::KDTree(const KDTree<Dim> &other)
    : size(other.size) {
   /**
    * @todo Implement this function!
    */
   root = copy(other.root);
}

template <int Dim>
const KDTree<Dim> &KDTree<Dim>::operator=(const KDTree<Dim> &rhs) {
   /**
    * @todo Implement this function!
    */
   if (this == &rhs) return *this;

   clear();
   size = rhs.size;

   root = copy(rhs.root);

   return *this;
}

template <int Dim>
KDTree<Dim>::~KDTree() {
   /**
    * @todo Implement this function!
    */
   clear();
}

template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighbor(const Point<Dim> &query) const {
   /**
    * @todo Implement this function!
    */
   if (!root) return Point<Dim>();
   Point<Dim> curBest = root->point;
   findNearestVer2(root, 0, curBest, query);

   return curBest;
}

// update curbest inplace
template <int Dim>
void KDTree<Dim>::findNearestVer2(KDTreeNode *subroot,
                                  int dimension,
                                  Point<Dim> &curBest,
                                  const Point<Dim> &query) const {
   if (!subroot) return;

   int nextDimension = (dimension + 1) % Dim;

   KDTreeNode *near, *far;

   if (smallerDimVal(query, subroot->point, dimension)) {
      near = subroot->left;
      far = subroot->right;
   } else {
      near = subroot->right;
      far = subroot->left;
   }

   findNearestVer2(near, nextDimension, curBest, query);

   if (shouldReplace(query, curBest, subroot->point)) curBest = subroot->point;

   double disToPlane =
       (subroot->point[dimension] - query[dimension]) * (subroot->point[dimension] - query[dimension]);
   if (calulateDis(curBest, query) >= disToPlane) findNearestVer2(far, nextDimension, curBest, query);
}

// template <int Dim>
// Point<Dim> KDTree<Dim>::findNearestNeighbor(const Point<Dim> &query) const {
//    /**
//     * @todo Implement this function!
//     */
//    if (!root) return Point<Dim>();
//    Point<Dim> curBest = findNearestNeighborRecursive(root, 0, root->point, query);
//
//    return curBest;
// }
//
// template <int Dim>
// Point<Dim> KDTree<Dim>::findNearestNeighborRecursive(KDTreeNode *subroot,
//                                                      int dimension,
//                                                      Point<Dim> currentBest,
//                                                      const Point<Dim> &query) const {
//    if (!subroot) return currentBest;
//
//    KDTreeNode *next = subroot->right;
//    KDTreeNode *remaining = subroot->left;
//    if (smallerDimVal(query, subroot->point, dimension)) {
//       next = subroot->left;
//       remaining = subroot->right;
//    }
//    Point<Dim> potential = findNearestNeighborRecursive(next, (dimension + 1) % Dim, currentBest, query);
//    if (shouldReplace(query, currentBest, potential)) currentBest = potential;
//
//    if (shouldReplace(query, currentBest, subroot->point)) currentBest = subroot->point;
//
//    // check other side
//    double disToPlane =
//        (subroot->point[dimension] - query[dimension]) * (subroot->point[dimension] - query[dimension]);
//    if (calulateDis(currentBest, query) >= disToPlane) {
//       Point<Dim> potential =
//           findNearestNeighborRecursive(remaining, (dimension + 1) % Dim, currentBest, query);
//       if (shouldReplace(query, currentBest, potential)) currentBest = potential;
//    }
//    return currentBest;
// }

template <int Dim>
double KDTree<Dim>::calulateDis(const Point<Dim> &point, const Point<Dim> &query) const {
   double dis = 0;
   for (int i = 0; i < Dim; ++i)
      dis += (point[i] - query[i]) * (point[i] - query[i]);
   return dis;
}

template <typename RandIter, typename Comparator>
void select(RandIter start, RandIter end, RandIter k, Comparator cmp) {
   /**
    * @todo Implement this function!
    */
   if (end - start <= 1) return; // assume end is one past the end

   RandIter pivot = start + rand() % (end - start);
   RandIter iter = partition(start, end - 1, pivot, cmp);
   if (iter == k) return;
   if (k < iter) select(start, iter, k, cmp);
   else select(iter + 1, end, k, cmp);
}

template <typename RandIter, typename Comparator>
RandIter partition(RandIter left, RandIter right, RandIter pivot, Comparator cmp) {
   if (right == left) return right;

   std::swap(*pivot, *right);
   RandIter i = left; // earliest position to put number < pivot
   for (RandIter j = left; j != right; ++j) {
      if (cmp(*j, *right)) {
         std::swap(*i, *j);
         ++i;
      }
   }
   std::swap(*right, *i);
   return i;
}

template <int Dim>
void KDTree<Dim>::clear() {
   size = 0;
   clear(root);
   root = nullptr;
}

template <int Dim>
void KDTree<Dim>::clear(KDTreeNode *subroot) {
   if (!subroot) return;

   clear(subroot->left);
   clear(subroot->right);

   delete subroot;
}

template <int Dim>
typename KDTree<Dim>::KDTreeNode *KDTree<Dim>::copy(KDTreeNode *otherSubroot) {
   if (!otherSubroot) return nullptr;
   KDTreeNode *newNode = new KDTreeNode(otherSubroot->point);
   newNode->left = copy(otherSubroot->left);
   newNode->right = copy(otherSubroot->right);
   return newNode;
}
