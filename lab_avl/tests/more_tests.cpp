#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "avltree.h"

// ------------------------------
// Helpers
// ------------------------------
template <typename K>
static void REQUIRE_sorted(const std::vector<K> &v) {
   REQUIRE(std::is_sorted(v.begin(), v.end()));
}

template <typename K, typename V>
static void REQUIRE_inorder_equals_keys(const AVLTree<K, V> &tree, const std::set<K> &keys) {
   std::vector<K> inorder = tree.getInorderTraversal();
   REQUIRE(inorder.size() == keys.size());
   REQUIRE_sorted(inorder);

   size_t i = 0;
   for (const auto &k : keys) {
      REQUIRE(inorder[i] == k);
      i++;
   }
}

template <typename K, typename V>
static void REQUIRE_find_matches(const AVLTree<K, V> &tree, const std::map<K, V> &ref) {
   for (auto const &[k, v] : ref) {
      REQUIRE(tree.find(k) == v);
   }
}

// verifies exact shape (preorder) + function calls,
// and also verifies inorder is sorted (BST property).
template <typename K, typename V>
static void verify_exact(const AVLTree<K, V> &tree,
                         const std::vector<K> &expectedPreorder,
                         const std::vector<std::string> &expectedCalls) {
   REQUIRE(tree.getFunctionOrder() == expectedCalls);

   std::vector<K> preorder = tree.getPreorderTraversal();
   std::vector<K> inorder = tree.getInorderTraversal();

   REQUIRE(preorder == expectedPreorder);
   REQUIRE_sorted(inorder);
}

// ------------------------------
// INSERT rotation cases (deterministic)
// ------------------------------

TEST_CASE("AVL insert: no rotations (already balanced)", "[avl][insert][part=1]") {
   AVLTree<int, int> t;
   t.insert(2, 2);
   t.insert(1, 1);
   t.insert(3, 3);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {});
}

TEST_CASE("AVL insert: LL case -> rotateRight", "[avl][insert][part=1]") {
   AVLTree<int, int> t;
   t.insert(3, 3);
   t.insert(2, 2);
   t.insert(1, 1);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {"rotateRight"});
}

TEST_CASE("AVL insert: RR case -> rotateLeft", "[avl][insert][part=1]") {
   AVLTree<int, int> t;
   t.insert(1, 1);
   t.insert(2, 2);
   t.insert(3, 3);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {"rotateLeft"});
}

TEST_CASE("AVL insert: LR case -> rotateLeftRight", "[avl][insert][part=1]") {
   AVLTree<int, int> t;
   t.insert(3, 3);
   t.insert(1, 1);
   t.insert(2, 2);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {"rotateLeftRight", "rotateLeft", "rotateRight"});
}

TEST_CASE("AVL insert: RL case -> rotateRightLeft", "[avl][insert][part=1]") {
   AVLTree<int, int> t;
   t.insert(1, 1);
   t.insert(3, 3);
   t.insert(2, 2);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {"rotateRightLeft", "rotateRight", "rotateLeft"});
}

TEST_CASE("AVL insert: duplicate key updates value and does not add a node",
          "[avl][insert][part=1]") {
   AVLTree<int, std::string> t;
   t.insert(5, "a");
   t.insert(5, "b"); // should overwrite

   REQUIRE(t.find(5) == "b");

   // should not create duplicates:
   std::vector<int> inorder = t.getInorderTraversal();
   REQUIRE(inorder.size() == 1);
   REQUIRE(inorder[0] == 5);

   // no rotations should be needed in this tiny case
   REQUIRE(t.getFunctionOrder().empty());
}

// ------------------------------
// REMOVE rotation cases (deterministic)
// These are small constructions where a single remove triggers exactly one (double) rotation.
// ------------------------------

TEST_CASE("AVL remove: LL case after delete -> rotateRight", "[avl][remove][part=1]") {
   // Build:   3
   //        / \
   //       2   4
   //      /
   //     1
   // Remove 4 => root becomes left-heavy (LL), rotateRight at 3.
   AVLTree<int, int> t;
   t.insert(3, 3);
   t.insert(2, 2);
   t.insert(4, 4);
   t.insert(1, 1);

   t.remove(4);

   verify_exact(t,
                /*pre*/ {2, 1, 3},
                /*calls*/ {"rotateRight"} // only rotation caused by remove here
   );
}

TEST_CASE("AVL remove: RR case after delete -> rotateLeft", "[avl][remove][part=1]") {
   // Build:   2
   //        / \
   //       1   3
   //            \
   //             4
   // Remove 1 => root becomes right-heavy (RR), rotateLeft at 2.
   AVLTree<int, int> t;
   t.insert(2, 2);
   t.insert(1, 1);
   t.insert(3, 3);
   t.insert(4, 4);

   t.remove(1);

   verify_exact(t,
                /*pre*/ {3, 2, 4},
                /*calls*/ {"rotateLeft"});
}

TEST_CASE("AVL remove: LR case after delete -> rotateLeftRight", "[avl][remove][part=1]") {
   // Build:    4
   //         /   \
   //        2     5
   //         \
   //          3
   // Remove 5 => root becomes left-heavy, left child is right-heavy => LR.
   AVLTree<int, int> t;
   t.insert(4, 4);
   t.insert(2, 2);
   t.insert(5, 5);
   t.insert(3, 3);

   t.remove(5);

   verify_exact(t,
                /*pre*/ {3, 2, 4},
                /*calls*/ {"rotateLeftRight", "rotateLeft", "rotateRight"});
}

TEST_CASE("AVL remove: RL case after delete -> rotateRightLeft", "[avl][remove][part=1]") {
   // Build:   2
   //        / \
   //       1   5
   //          /
   //         4
   // Remove 1 => root becomes right-heavy, right child left-heavy => RL.
   AVLTree<int, int> t;
   t.insert(2, 2);
   t.insert(1, 1);
   t.insert(5, 5);
   t.insert(4, 4);

   t.remove(1);

   verify_exact(t,
                /*pre*/ {4, 2, 5},
                /*calls*/ {"rotateRightLeft", "rotateRight", "rotateLeft"});
}

// ------------------------------
// REMOVE correctness (leaf / one-child / two-child) via inorder-set property
// ------------------------------

TEST_CASE("AVL remove: leaf / one-child / two-child all keep inorder == set",
          "[avl][remove][part=1]") {
   AVLTree<int, int> t;
   std::set<int> keys;

   // Build a decent tree
   for (int k : {5, 3, 7, 2, 4, 6, 8}) {
      t.insert(k, k);
      keys.insert(k);
   }
   REQUIRE_inorder_equals_keys(t, keys);

   // remove leaf
   t.remove(2);
   keys.erase(2);
   REQUIRE_inorder_equals_keys(t, keys);

   // remove node with one child (after removing 2, node 3 has right child 4 only)
   t.remove(3);
   keys.erase(3);
   REQUIRE_inorder_equals_keys(t, keys);

   // remove node with two children (root-ish case)
   t.remove(5);
   keys.erase(5);
   REQUIRE_inorder_equals_keys(t, keys);

   // check find still works for remaining keys
   for (int k : keys) {
      REQUIRE(t.find(k) == k);
   }
}

TEST_CASE("AVL remove: remove until empty should not crash", "[avl][remove][part=1]") {
   AVLTree<int, int> t;
   std::vector<int> ks = {10, 5, 15, 2, 7, 12, 17, 1, 3};
   for (int k : ks)
      t.insert(k, k);

   for (int k : ks)
      t.remove(k);

   REQUIRE(t.getInorderTraversal().empty());
   REQUIRE(t.getPreorderTraversal().empty());
   // functionCalls can be non-empty depending on rotations, but should exist and not crash.
}

// ------------------------------
// Randomized stress test vs std::map (catches subtle bugs)
// ------------------------------

TEST_CASE("AVL randomized stress vs std::map", "[avl][stress][part=1]") {
   AVLTree<int, int> t;
   std::map<int, int> ref;
   std::set<int> keys;

   std::mt19937 rng(12345);
   std::uniform_int_distribution<int> keyDist(-200, 200);
   std::uniform_int_distribution<int> opDist(0, 99);

   // Do a bunch of ops
   for (int step = 0; step < 2000; step++) {
      int k = keyDist(rng);
      int op = opDist(rng);

      if (op < 60) {
         // insert/update
         int v = step; // deterministic value
         t.insert(k, v);
         ref[k] = v;
         keys.insert(k);
      } else {
         // remove only if exists (public remove assumes key exists)
         auto it = ref.find(k);
         if (it != ref.end()) {
            t.remove(k);
            ref.erase(it);
            keys.erase(k);
         }
      }

      // validate BST-key correctness frequently
      REQUIRE_inorder_equals_keys(t, keys);
      REQUIRE_find_matches(t, ref);
   }
}
