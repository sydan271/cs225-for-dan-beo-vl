#include <catch2/catch_test_macros.hpp>
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#include "bst.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// ---------- small helpers ----------
template <class T> static void requireVecEq(const std::vector<T> &a, const std::vector<T> &b) {
   REQUIRE(a.size() == b.size());
   for (size_t i = 0; i < a.size(); i++) {
      INFO("Mismatch at i=" << i);
      REQUIRE(a[i] == b[i]);
   }
}

template <class K, class V> static BST<K, V> makeTree(const std::vector<std::pair<K, V>> &items) {
   BST<K, V> t;
   for (auto const &kv : items)
      t.insert(kv.first, kv.second);
   return t;
}

// ---------- tests ----------

TEST_CASE("BST::height empty tree is -1", "[extra][bst][valgrind]") {
   BST<int, int> t;
   REQUIRE(t.height() == -1);
   REQUIRE(t.getInorderTraversal().empty());
   REQUIRE(t.getPreorderTraversal().empty());
}

TEST_CASE("BST::clear empties the tree and allows reuse", "[extra][bst][valgrind]") {
   BST<int, int> t;
   t.insert(2, 20);
   t.insert(1, 10);
   t.insert(3, 30);
   REQUIRE(t.height() == 1);

   t.clear();
   REQUIRE(t.height() == -1);
   REQUIRE(t.getInorderTraversal().empty());

   // reuse after clear
   t.insert(5, 50);
   REQUIRE(t.height() == 0);
   requireVecEq(t.getInorderTraversal(), std::vector<int>{5});
   REQUIRE(t.find(5) == 50);
}

TEST_CASE("BST::insert does not overwrite existing key/value", "[extra][bst][valgrind]") {
   BST<int, int> t;
   t.insert(10, 111);
   t.insert(10, 999); // should be ignored
   REQUIRE(t.find(10) == 111);

   // structure should still be a single node
   REQUIRE(t.height() == 0);
   requireVecEq(t.getInorderTraversal(), std::vector<int>{10});
}

TEST_CASE("BST::remove on single-node tree makes it empty", "[extra][bst][valgrind]") {
   BST<int, int> t;
   t.insert(7, 70);
   REQUIRE(t.height() == 0);

   t.remove(7);
   REQUIRE(t.height() == -1);
   REQUIRE(t.getInorderTraversal().empty());
   REQUIRE(t.getPreorderTraversal().empty());
}

TEST_CASE("BST::remove root (2 children), deep IOP with a left child", "[extra][bst][valgrind]") {
   // Build a tree where root's IOP is deep (rightmost of left subtree)
   // and that IOP has a left child:
   //
   //            50
   //          /    \
   //        30      70
   //       /  \    /  \
   //     20   40  60  80
   //            \
   //            48
   //           /
   //         47
   //
   // remove(50): IOP is 48, which has left child 47.
   // After correct remove, preorder should be:
   // 48, 30, 20, 40, 47, 70, 60, 80
   BST<int, int> t;
   for (auto [k, v] : std::vector<std::pair<int, int>>{
            {50, 500}, {30, 300}, {70, 700}, {20, 200}, {40, 400}, {60, 600}, {80, 800}, {48, 480}, {47, 470}}) {
      t.insert(k, v);
   }

   t.remove(50);

   requireVecEq(t.getInorderTraversal(), std::vector<int>{20, 30, 40, 47, 48, 60, 70, 80});
   requireVecEq(t.getPreorderTraversal(), std::vector<int>{48, 30, 20, 40, 47, 70, 60, 80});

   // Also verify values still match their keys after the remove/swaps:
   REQUIRE(t.find(48) == 480);
   REQUIRE(t.find(47) == 470);
   REQUIRE(t.find(70) == 700);
}

TEST_CASE("BST copy constructor makes a deep copy (mutating original doesn't affect copy)", "[extra][bst][valgrind]") {
   BST<int, std::string> a;
   a.insert(2, "two");
   a.insert(1, "one");
   a.insert(3, "three");

   BST<int, std::string> b(a); // copy

   // mutate original
   a.remove(2);

   // copy should remain unchanged
   requireVecEq(b.getInorderTraversal(), std::vector<int>{1, 2, 3});
   REQUIRE(b.find(2) == "two");

   // original changed
   requireVecEq(a.getInorderTraversal(), std::vector<int>{1, 3});
}

TEST_CASE("BST assignment operator deep copies, and self-assignment is safe", "[extra][bst][valgrind]") {
   BST<int, int> a;
   a.insert(10, 1);
   a.insert(5, 2);
   a.insert(15, 3);

   BST<int, int> b;
   b.insert(999, 999);

   b = a; // assign

   requireVecEq(b.getInorderTraversal(), std::vector<int>{5, 10, 15});
   REQUIRE(b.find(10) == 1);

   // mutate a, b should not change
   a.remove(5);
   requireVecEq(a.getInorderTraversal(), std::vector<int>{10, 15});
   requireVecEq(b.getInorderTraversal(), std::vector<int>{5, 10, 15});

   // self assignment should not change or crash
   b = b;
   requireVecEq(b.getInorderTraversal(), std::vector<int>{5, 10, 15});
}

TEST_CASE("listBuild handles duplicates by keeping the first inserted value", "[extra][bst][valgrind]") {
   // duplicate key 7 appears twice with different values
   std::vector<std::pair<int, int>> items = {{7, 70}, {3, 30}, {7, 7000}, {9, 90}};
   BST<int, int> t = listBuild(items);

   // keys unique in BST
   requireVecEq(t.getInorderTraversal(), std::vector<int>{3, 7, 9});
   // value should be from the first insert of key=7
   REQUIRE(t.find(7) == 70);
}

TEST_CASE("allBuild for n=2 yields {0, 2}", "[extra][bst][valgrind]") {
   std::vector<std::pair<int, int>> items = {{1, 0}, {2, 0}};
   auto out = allBuild(items);
   requireVecEq(out, std::vector<int>{0, 2});
   REQUIRE(std::accumulate(out.begin(), out.end(), 0) == 2);
}

TEST_CASE("allBuild for n=4 yields {0, 0, 16, 8}", "[extra][bst][valgrind]") {
   // For keys 1..4, there are 24 insertion orders.
   // Heights distribution (empty=-1, single=0):
   // height 2: 16 permutations, height 3: 8 permutations
   std::vector<std::pair<int, int>> items = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
   auto out = allBuild(items);

   requireVecEq(out, std::vector<int>{0, 0, 16, 8});
   REQUIRE(std::accumulate(out.begin(), out.end(), 0) == 24);
}
