/**
 * @file kdtree.cpp
 * Implementation of KDTree class.
 */
#pragma once
#include "cs225/point.h"
#include "kdtree.h"

#include <algorithm>
#include <deque>
#include <limits>
#include <utility>

using namespace std;

template <int Dim>
bool smallerDimVal(const Point<Dim> &first, const Point<Dim> &second, int curDim) {
   /**
    * @todo Implement this function!
    */
   if (first[curDim] == second[curDim]) return first < second;

   return first[curDim] < second[curDim];
}

template <int Dim>
bool shouldReplace(const Point<Dim> &target,
                   const Point<Dim> &currentBest,
                   const Point<Dim> &potential) {
   /**
    * @todo Implement this function!
    */
   double disCur = 0;
   double disPos = 0;
   for (unsigned i = 0; i < Dim; ++i) {
      disCur += (currentBest[i] - target[i]) * (currentBest[i] - target[i]);
      disPos += (potential[i] - target[i]) * (potential[i] - target[i]);
   }
   if (disPos < disCur) return true;
   if (disCur == disPos) return potential < currentBest;

   return false;
}

template <int Dim>
KDTree<Dim>::KDTree(const vector<Point<Dim>> &newPoints)
    : root(nullptr)
    , size(newPoints.size()) {
   /**
    * @todo Implement this function!
    */
   if (newPoints.size() == 0) return;
   std::vector<Point<Dim>> nonConstV = newPoints;
   makeTree(root, nonConstV, 0, (int)nonConstV.size() - 1, 0);
}

template <int Dim>
void KDTree<Dim>::makeTree(
    KDTreeNode *&subroot, std::vector<Point<Dim>> &Points, int left, int right, int curDim) {
   if (left > right) {
      subroot = nullptr;
      return;
   }
   auto cmp = [curDim](const Point<Dim> &first, const Point<Dim> &second) {
      return smallerDimVal(first, second, curDim);
   };

   int medianIndex = (left + right) / 2;
   select(Points.begin() + left, Points.begin() + right + 1, Points.begin() + medianIndex, cmp);

   subroot = new KDTreeNode(Points[medianIndex]);

   int nextDim = (curDim + 1) % Dim;
   makeTree(subroot->left, Points, left, medianIndex - 1, nextDim);
   makeTree(subroot->right, Points, medianIndex + 1, right, nextDim);
}

template <int Dim>
KDTree<Dim>::KDTree(const KDTree<Dim> &other)
    : root(nullptr)
    , size(other.size) {
   /**
    * @todo Implement this function!
    */
   copyTree(root, other.root);
}

template <int Dim>
void KDTree<Dim>::copyTree(KDTreeNode *&subroot, const KDTreeNode *otherSubroot) {
   if (!otherSubroot) return;
   subroot = new KDTreeNode(otherSubroot->point);
   copyTree(subroot->left, otherSubroot->left);
   copyTree(subroot->right, otherSubroot->right);
}

template <int Dim>
const KDTree<Dim> &KDTree<Dim>::operator=(const KDTree<Dim> &rhs) {
   /**
    * @todo Implement this function!
    */
   if (this == &rhs) return *this;

   clear(root);

   size = rhs.size;
   copyTree(root, rhs.root);

   return *this;
}

template <int Dim>
KDTree<Dim>::~KDTree() {
   /**
    * @todo Implement this function!
    */
   clear(root);
   size = 0;
}

template <int Dim>
void KDTree<Dim>::clear(KDTreeNode *&subroot) {
   if (!subroot) return;

   clear(subroot->left);
   clear(subroot->right);

   delete subroot;
   subroot = nullptr;
}

template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighbor(const Point<Dim> &query) const {
   /**
    * @todo Implement this function!
    */
   if (!root) return Point<Dim>();

   Point<Dim> curBest = root->point;

   findNearestNeighborRecursive(root, query, curBest, 0);

   return curBest;
}

template <int Dim>
void KDTree<Dim>::findNearestNeighborRecursive(KDTreeNode *subroot,
                                               const Point<Dim> &query,
                                               Point<Dim> &curBest,
                                               int curDim) const {
   if (!subroot) return;

   int nextDim = (curDim + 1) % Dim;

   KDTreeNode *near;
   KDTreeNode *far;

   if (smallerDimVal(query, subroot->point, curDim)) {
      near = subroot->left;
      far = subroot->right;
   } else {
      far = subroot->left;
      near = subroot->right;
   }

   findNearestNeighborRecursive(near, query, curBest, nextDim);

   if (shouldReplace(query, curBest, subroot->point)) {
      curBest = subroot->point;
   }

   if (calDis(query, curBest) == 0) return;

   double disToPlane =
       (subroot->point[curDim] - query[curDim]) * (subroot->point[curDim] - query[curDim]);
   if (calDis(query, curBest) >= disToPlane)
      findNearestNeighborRecursive(far, query, curBest, nextDim);
}

template <int Dim>
double KDTree<Dim>::calDis(const Point<Dim> &target, const Point<Dim> &point) const {
   double dis = 0;
   for (unsigned i = 0; i < Dim; ++i) {
      dis += (point[i] - target[i]) * (point[i] - target[i]);
   }
   return dis;
}

template <typename RandIter, typename Comparator>
void select(RandIter start, RandIter end, RandIter k, Comparator cmp) {
   /**
    * @todo Implement this function!
    */

   if (end - start <= 1) return;

   RandIter pivotIter = start + (end - start) / 2;
   pivotIter = partitionAtPivot(start, end, pivotIter, cmp);
   if (k == pivotIter) return;
   else if (k < pivotIter) select(start, pivotIter, k, cmp);
   else select(pivotIter + 1, end, k, cmp);
}

template <typename RandIter, typename Comparator>
RandIter partitionAtPivot(RandIter start, RandIter end, RandIter pivot, Comparator cmp) {
   if (end - start <= 1) return start;
   RandIter right = end - 1;
   std::iter_swap(pivot, right);

   RandIter storeIter = start;
   for (RandIter i = start; i != right; ++i) {
      if (cmp(*i, *right)) {
         std::iter_swap(storeIter, i);
         ++storeIter;
      }
   }

   std::iter_swap(right, storeIter);
   return storeIter;
}
