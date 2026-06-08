
#include <catch2/catch_test_macros.hpp>

#include "InorderTraversal.h"
#include "binarytree.h"
#include "binarytree.hpp"

#include <algorithm>
#include <initializer_list>
#include <queue>
#include <random>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace {

// ---------- helpers (new, no duplicates of your existing ones) ----------

template <typename T> using Node = typename BinaryTree<T>::Node;

template <typename T> BinaryTree<T> makeManualTree(Node<T> *root) {
   // Wrap raw nodes so BinaryTree destructor frees them.
   return BinaryTree<T>(root);
}

template <typename T> std::vector<T> inorderViaVector(BinaryTree<T> &t) {
   std::vector<T> v;
   t.inOrder(v);
   return v;
}

template <typename T> std::vector<T> inorderViaTraversal(Node<T> *root) {
   std::vector<T> v;
   InorderTraversal<T> trav(root);

   for (auto it = trav.begin(); it != trav.end(); ++it) {
      Node<T> *n = *it;
      // Defensive: in a correct traversal, n should never be null while iterating.
      REQUIRE(n != nullptr);
      v.push_back(n->elem);
   }
   return v;
}

template <typename T> bool isNondecreasing(const std::vector<T> &v) {
   for (size_t i = 1; i < v.size(); i++) {
      if (v[i] < v[i - 1])
         return false;
   }
   return true;
}

template <typename T> bool isNonincreasing(const std::vector<T> &v) {
   for (size_t i = 1; i < v.size(); i++) {
      if (v[i] > v[i - 1])
         return false;
   }
   return true;
}

template <typename T> std::vector<T> sortedCopy(std::vector<T> v) {
   std::sort(v.begin(), v.end());
   return v;
}

// Counts nodes and detects cycles by pointer identity.
// If a node address is seen twice, returns SIZE_MAX to signal a cycle.
template <typename T> size_t countNodesNoCycle(Node<T> *root, size_t hardLimit = 100000) {
   if (root == nullptr)
      return 0;

   std::unordered_set<Node<T> *> seen;
   std::queue<Node<T> *> q;
   q.push(root);

   while (!q.empty()) {
      Node<T> *cur = q.front();
      q.pop();

      if (cur == nullptr)
         continue;

      auto [it, inserted] = seen.insert(cur);
      if (!inserted)
         return SIZE_MAX; // cycle detected

      if (seen.size() > hardLimit)
         return SIZE_MAX; // runaway / suspicious

      q.push(cur->left);
      q.push(cur->right);
   }

   return seen.size();
}

BinaryTree<int> buildBST(std::initializer_list<int> xs) {
   BinaryTree<int> t;
   for (int x : xs)
      t.insert(x);
   return t;
}

} // namespace

// ---------------------------- tests ----------------------------

TEST_CASE("Empty tree: mirror no-op, ordered true, traversal empty", "[edge][empty][mirror][ordered][traversal]") {
   BinaryTree<int> t;

   REQUIRE(t.getRoot() == nullptr);
   REQUIRE(t.height() == -1);

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   // InorderTraversal on empty should have begin == end (use != only).
   InorderTraversal<int> trav(t.getRoot());
   auto b = trav.begin();
   auto e = trav.end();
   REQUIRE_FALSE(b != e);

   // mirror should not crash and should keep empty
   t.mirror();
   REQUIRE(t.getRoot() == nullptr);
   REQUIRE(t.height() == -1);
   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);
}

TEST_CASE("Single node: mirror no-op, ordered true, traversal yields one element",
          "[edge][single][mirror][ordered][traversal]") {
   auto *r = new Node<int>(42);
   BinaryTree<int> t = makeManualTree<int>(r);

   REQUIRE(t.height() == 0);
   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   auto v1 = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(v1 == std::vector<int>({42}));

   auto v2 = inorderViaVector<int>(t);
   REQUIRE(v2 == std::vector<int>({42}));

   t.mirror();
   auto v3 = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(v3 == std::vector<int>({42}));

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);
}

TEST_CASE("Two nodes (left child): ordered before mirror, unordered after mirror", "[edge][two][mirror][ordered]") {
   // root 2 with left 1
   auto *r = new Node<int>(2);
   r->left = new Node<int>(1);
   BinaryTree<int> t = makeManualTree<int>(r);

   auto before = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(before == std::vector<int>({1, 2}));
   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   t.mirror();

   auto after = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(after == std::vector<int>({2, 1}));
   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);
}

TEST_CASE("Two nodes (right child): ordered before mirror, unordered after mirror", "[edge][two][mirror][ordered]") {
   // root 1 with right 2
   auto *r = new Node<int>(1);
   r->right = new Node<int>(2);
   BinaryTree<int> t = makeManualTree<int>(r);

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   t.mirror();

   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);
}

TEST_CASE("Mirror invariants: node multiset preserved, height preserved, no cycle, mirror twice restores inorder",
          "[mirror][invariants]") {
   auto t = buildBST({6, 10, 7, 4, 0, 8, 9, 11, 1, 3, 2, 5});

   auto root0 = t.getRoot();
   REQUIRE(root0 != nullptr);

   auto inorder0 = inorderViaVector<int>(t);
   auto sorted0 = sortedCopy(inorder0);

   int h0 = t.height();
   size_t n0 = countNodesNoCycle<int>(t.getRoot());
   REQUIRE(n0 != SIZE_MAX);

   t.mirror();

   // Still same number of nodes, no cycles
   size_t n1 = countNodesNoCycle<int>(t.getRoot());
   REQUIRE(n1 != SIZE_MAX);
   REQUIRE(n1 == n0);

   // Height should not change under mirroring
   REQUIRE(t.height() == h0);

   // Multiset of values should match exactly
   auto inorder1 = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(sortedCopy(inorder1) == sorted0);

   // For a BST built via insert, mirroring should usually make inorder nonincreasing
   REQUIRE(isNonincreasing(inorder1) == true);
   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);

   // Mirror twice restores original inorder sequence
   t.mirror();
   auto inorder2 = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(inorder2 == inorder0);
   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);
}

TEST_CASE("Duplicates: nondecreasing allowed (ordered should be true)", "[ordered][duplicates]") {
   // Manual tree: 5 with left 3 and right 5
   auto *r = new Node<int>(5);
   r->left = new Node<int>(3);
   r->right = new Node<int>(5);
   BinaryTree<int> t = makeManualTree<int>(r);

   auto in = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(in == std::vector<int>({3, 5, 5}));
   REQUIRE(isNondecreasing(in) == true);

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   // Mirroring should usually break orderedness
   t.mirror();
   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);
}

TEST_CASE("Non-BST manual trees: ordered should be false (both recursive and iterative)", "[ordered][nonbst]") {
   // Root 5, left 9 (violates), right 1 (also violates)
   auto *r = new Node<int>(5);
   r->left = new Node<int>(9);
   r->right = new Node<int>(1);
   BinaryTree<int> t = makeManualTree<int>(r);

   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);

   // Mirror should not crash
   t.mirror();
   // still likely false, but the key is "doesn't crash" and functions agree
   REQUIRE(t.isOrderedRecursive() == t.isOrderedIterative());
}

TEST_CASE("Traversal vs inOrder(vector&): same sequence on several shapes", "[traversal][inorder][consistency]") {
   SECTION("Balanced-ish BST") {
      auto t = buildBST({8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15});
      auto vA = inorderViaVector<int>(t);
      auto vB = inorderViaTraversal<int>(t.getRoot());
      REQUIRE(vA == vB);
      REQUIRE(isNondecreasing(vA) == true);
   }

   SECTION("Right-skewed (ascending inserts)") {
      BinaryTree<int> t;
      for (int i = 0; i < 50; i++)
         t.insert(i);

      auto vA = inorderViaVector<int>(t);
      auto vB = inorderViaTraversal<int>(t.getRoot());
      REQUIRE(vA == vB);
      REQUIRE(isNondecreasing(vA) == true);

      // Mirror becomes left-skewed; traversal still must visit all nodes
      t.mirror();
      auto vC = inorderViaTraversal<int>(t.getRoot());
      REQUIRE(vC.size() == vA.size());
      REQUIRE(sortedCopy(vC) == vA); // same multiset
   }

   SECTION("Left-skewed (descending inserts)") {
      BinaryTree<int> t;
      for (int i = 50; i >= 0; i--)
         t.insert(i);

      auto vA = inorderViaTraversal<int>(t.getRoot());
      REQUIRE(isNondecreasing(vA) == true);

      // Mirror should flip skew direction and break orderedness
      t.mirror();
      REQUIRE(t.isOrderedRecursive() == false);
      REQUIRE(t.isOrderedIterative() == false);
   }
}

TEST_CASE("Ordered check agreement: recursive and iterative always match on random BSTs",
          "[ordered][random][agreement]") {
   std::mt19937 rng(1337);
   std::uniform_int_distribution<int> dist(-1000, 1000);

   for (int trial = 0; trial < 25; trial++) {
      BinaryTree<int> t;
      for (int i = 0; i < 100; i++)
         t.insert(dist(rng));

      bool r = t.isOrderedRecursive();
      bool it = t.isOrderedIterative();
      REQUIRE(r == it);

      // Because insert makes a BST, it should be ordered
      REQUIRE(r == true);

      t.mirror();
      // After mirror, should typically be not ordered; at least the two should agree
      REQUIRE(t.isOrderedRecursive() == t.isOrderedIterative());
   }
}

TEST_CASE("Stress: deep tree should not crash inorder traversal or ordered checks",
          "[stress][deep][traversal][ordered]") {
   BinaryTree<int> t;
   // Make a deep-ish tree: ascending inserts -> height ~ N-1
   for (int i = 0; i < 200; i++)
      t.insert(i);

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   auto v = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(v.size() == 200);
   REQUIRE(isNondecreasing(v) == true);

   t.mirror();
   // should still not crash
   auto v2 = inorderViaTraversal<int>(t.getRoot());
   REQUIRE(v2.size() == 200);
   REQUIRE(sortedCopy(v2) == v);
}

TEST_CASE("Mirror on empty/single/various trees never introduces cycles", "[mirror][cycle]") {
   SECTION("empty") {
      BinaryTree<int> t;
      t.mirror();
      REQUIRE(countNodesNoCycle<int>(t.getRoot()) != SIZE_MAX);
   }

   SECTION("single") {
      auto *r = new Node<int>(1);
      BinaryTree<int> t = makeManualTree<int>(r);
      t.mirror();
      REQUIRE(countNodesNoCycle<int>(t.getRoot()) != SIZE_MAX);
   }

   SECTION("bigger") {
      auto t = buildBST({10, 5, 15, 3, 7, 12, 18, 1, 4, 6, 9});
      t.mirror();
      REQUIRE(countNodesNoCycle<int>(t.getRoot()) != SIZE_MAX);
   }
}
