/**
 * @file dsets.h
 * Disjoint Sets implementation.
 *
 * @date 12/02/2009
 */
#pragma once

#include <vector>

/**
 * Each DisjointSets object represents a family of disjoint sets, where each
 * element has an integer index. It is implemented with the optimizations
 * discussed in class, as up-trees stored in a single vector of ints.
 * Specifically, path compression and union-by-size are used. Each place in the
 * vector represents a node. (Note that this means that the elements in our
 * universe are indexed starting at 0.) A nonnegative number is the index of the
 * parent of the current node; a negative number in a root node is the negative
 * of the set size.
 *
 * Note that the default compiler-supplied Big Three will work flawlessly
 * because the only member data is a vector of integers and this vector should
 * initially be empty.
 */
class DisjointSets {
   std::vector<int> container_;
   bool checkValidIdx(int i) const;

public:
   /**
    * DisjointSets constructor, creates an empty disjoint set
    */
   DisjointSets() {}

   /**
    * DisjointSets constructor, creates n unconnected root nodes
    * @param num The number of nodes to create
    */
   DisjointSets(int num)
       : container_(num, -1) {}

   /**
    * Adds n additional unconnected root nodes to the disjoint set
    * @param num The number of nodes to create
    */
   void addElements(int num);

   /**
    * This function should compress paths and works as described in lecture.
    * @return the index of the root of the up-tree in which the parameter
    *  element resides.
    */
   int find(int elem);

   /**
    * This function should be implemented as union-by-size. That is, when you
    * setunion two disjoint sets, the smaller (in terms of number of nodes)
    * should point at the larger. This function works as described in lecture,
    * except that you should not assume that the arguments to setunion are
    * roots of existing uptrees.
    *
    * Your setunion function SHOULD find the roots of its arguments before
    * combining the trees. If the two sets are the same size, make the tree
    * containing the second argument point to the tree containing the first.
    * (Note that normally we could break this tie arbitrarily, but in this case
    * we want to control things for grading.)
    *
    * @param a Index of the first element to union
    * @param b Index of the second element to union
    */
   void setUnion(int a, int b);

   /**
    * This function should return the number of nodes in the up-tree containing
    * the element.
    * @return number of nodes in the up-tree containing the element
    */
   int size(int elem);

   /**
    * This function returns the direct parent of the node in the vector
    * For example, if the vector has structure
    * `Index: 0  |  1 |  2 |  3 |  4 |  5 |  6 |  7`
    * `Value: -1 | -1 |  4 |  6 |  7 | -1 | -2 | -3`
    * (corresponding to `{0}, {1}, {2, 4, 7}, {3, 6}`)
    * Then `getValue(2)` should return 4, NOT 7
    */
   int getValue(int elem) const;
};
