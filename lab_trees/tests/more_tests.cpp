#include <catch2/catch_test_macros.hpp>

#include "InorderTraversal.h"
#include "TreeTraversal.h"
#include "binarytree.h"

#include <algorithm>
#include <random>
#include <vector>

namespace {

// Collect inorder values using the provided BinaryTree::inOrder(vector&)
static std::vector<int> collectInOrderVec(BinaryTree<int> &tree) {
   std::vector<int> v;
   tree.inOrder(v);
   return v;
}

// Collect inorder values using your InorderTraversal iterator
static std::vector<int> collectInorderTraversal(BinaryTree<int> &tree) {
   std::vector<int> v;
   InorderTraversal<int> trav(tree.getRoot());
   for (auto it = trav.begin(); it != trav.end(); ++it) {
      REQUIRE((*it) != nullptr);
      v.push_back((*it)->elem);
   }
   return v;
}

static bool isNondecreasing(const std::vector<int> &v) {
   for (size_t i = 1; i < v.size(); i++) {
      if (v[i] < v[i - 1])
         return false;
   }
   return true;
}

// reverse-copy utility (no duplication of your assertMirror helper)
static std::vector<int> reversed(std::vector<int> v) {
   std::reverse(v.begin(), v.end());
   return v;
}

// Build a small manual (non-BST) tree to test traversal correctness beyond BST insert shape
static BinaryTree<int> makeManualTreeShapeA() {
   //        1
   //      /   \
   //     2     3
   //      \
   //       4
   //
   // inorder should be: 2,4,1,3
   using Node = BinaryTree<int>::Node;
   Node *r = new Node(1);
   r->left = new Node(2);
   r->right = new Node(3);
   r->left->right = new Node(4);
   return BinaryTree<int>(r);
}

// Make a BST via insert in ascending order (right-skewed)
static BinaryTree<int> makeRightSkewBST(int n) {
   BinaryTree<int> t;
   for (int i = 1; i <= n; i++)
      t.insert(i);
   return t;
}

// Make a BST via insert in descending order (left-skewed)
static BinaryTree<int> makeLeftSkewBST(int n) {
   BinaryTree<int> t;
   for (int i = n; i >= 1; i--)
      t.insert(i);
   return t;
}

// Make a random-path (NOT necessarily ordered) tree via insertRandom
static BinaryTree<int> makeRandomPathTree(int n, uint32_t seed) {
   BinaryTree<int> t;
   std::mt19937 rng(seed);
   for (int i = 0; i < n; i++)
      t.insertRandom(i, rng);
   return t;
}

} // namespace

// ------------------------ mirror() tests ------------------------

TEST_CASE("mirror: empty tree is no-op", "[mirror][tree]") {
   BinaryTree<int> t;
   // Should not crash:
   t.mirror();

   auto v = collectInOrderVec(t);
   REQUIRE(v.empty());

   // Empty tree should be ordered by the standard definition (nondecreasing inorder).
   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);
}

TEST_CASE("mirror: single node unchanged", "[mirror][tree]") {
   BinaryTree<int> t;
   t.insert(42);

   auto before = collectInOrderVec(t);
   t.mirror();
   auto after = collectInOrderVec(t);

   REQUIRE(before == after);
   REQUIRE(after == std::vector<int>({42}));
}

TEST_CASE("mirror: inorder becomes reversed inorder for any tree", "[mirror][tree]") {
   // Use a non-BST shape to ensure we are testing mirror structure, not BST properties.
   BinaryTree<int> t = makeManualTreeShapeA();

   auto before = collectInOrderVec(t);
   t.mirror();
   auto after = collectInOrderVec(t);

   REQUIRE(after == reversed(before));
}

TEST_CASE("mirror: applying mirror twice restores original inorder", "[mirror][tree]") {
   BinaryTree<int> t = makeManualTreeShapeA();

   auto before = collectInOrderVec(t);
   t.mirror();
   t.mirror();
   auto after = collectInOrderVec(t);

   REQUIRE(before == after);
}

TEST_CASE("mirror: mirroring a BST usually breaks isOrdered", "[mirror][tree][ordered]") {
   BinaryTree<int> t;
   for (int x : {6, 10, 7, 4, 0, 8, 9, 11, 1, 3, 2, 5})
      t.insert(x);

   REQUIRE(t.isOrderedRecursive() == true);
   REQUIRE(t.isOrderedIterative() == true);

   t.mirror();

   // After mirroring, inorder should be decreasing (reverse of sorted),
   // so it should NOT be nondecreasing anymore.
   REQUIRE(t.isOrderedRecursive() == false);
   REQUIRE(t.isOrderedIterative() == false);
}

// -------------------- TreeTraversal / InorderTraversal tests --------------------

TEST_CASE("InorderTraversal: empty traversal begin == end and yields nothing", "[traversal][inorder]") {
   BinaryTree<int> t;

   InorderTraversal<int> trav(t.getRoot());
   auto b = trav.begin();
   auto e = trav.end();

   REQUIRE_FALSE(b != e); // equivalent to b == e

   std::vector<int> got;
   for (auto it = trav.begin(); it != trav.end(); ++it) {
      got.push_back((*it)->elem);
   }
   REQUIRE(got.empty());
}

TEST_CASE("InorderTraversal: single node yields that node then ends", "[traversal][inorder]") {
   BinaryTree<int> t;
   t.insert(7);

   auto got = collectInorderTraversal(t);
   REQUIRE(got == std::vector<int>({7}));
}

TEST_CASE("InorderTraversal matches BinaryTree::inOrder for BST (right-skew)", "[traversal][inorder]") {
   BinaryTree<int> t = makeRightSkewBST(8);

   auto v1 = collectInOrderVec(t);
   auto v2 = collectInorderTraversal(t);

   REQUIRE(v1 == v2);
   REQUIRE(isNondecreasing(v2) == true);
}

TEST_CASE("InorderTraversal matches BinaryTree::inOrder for BST (left-skew)", "[traversal][inorder]") {
   BinaryTree<int> t = makeLeftSkewBST(8);

   auto v1 = collectInOrderVec(t);
   auto v2 = collectInorderTraversal(t);

   REQUIRE(v1 == v2);
   REQUIRE(isNondecreasing(v2) == true);
}

TEST_CASE("InorderTraversal: manual non-BST shape yields expected order", "[traversal][inorder]") {
   BinaryTree<int> t = makeManualTreeShapeA();

   auto got = collectInorderTraversal(t);
   REQUIRE(got == std::vector<int>({2, 4, 1, 3}));
}

// ---------------------- isOrdered + inOrder family tests ----------------------

TEST_CASE("isOrdered: empty and single-node trees are ordered (recursive + iterative)", "[ordered][tree]") {
   BinaryTree<int> empty;
   REQUIRE(empty.isOrderedRecursive() == true);
   REQUIRE(empty.isOrderedIterative() == true);

   BinaryTree<int> single;
   single.insert(123);
   REQUIRE(single.isOrderedRecursive() == true);
   REQUIRE(single.isOrderedIterative() == true);
}

TEST_CASE("isOrdered: recursive and iterative agree on many random-path trees", "[ordered][tree]") {
   // Random-path insertion does not guarantee BST; we only require both implementations agree.
   for (uint32_t seed = 1; seed <= 20; seed++) {
      BinaryTree<int> t = makeRandomPathTree(50, seed);
      bool r = t.isOrderedRecursive();
      bool i = t.isOrderedIterative();
      REQUIRE(r == i);

      // Cross-check against definition using inOrder vector:
      auto v = collectInOrderVec(t);
      REQUIRE(r == isNondecreasing(v));
   }
}

TEST_CASE("inOrder: mirror reverses the inOrder vector", "[inorder][mirror][tree]") {
   BinaryTree<int> t;
   for (int x : {52, 39, 71, 17, 47, 69, 80, 90, 0, 24})
      t.insert(x);

   auto before = collectInOrderVec(t);
   t.mirror();
   auto after = collectInOrderVec(t);

   REQUIRE(after == reversed(before));
}
