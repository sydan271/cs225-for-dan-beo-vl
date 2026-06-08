
#include <catch2/catch_test_macros.hpp>

#include "btree.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------- Helpers ----------

template <class K, class V>
static void insert_all(BTree<K, V> &b, const std::vector<std::pair<K, V>> &data, unsigned order) {
   for (const auto &kv : data) {
      b.insert(kv.first, kv.second);
      REQUIRE(b.is_valid(order)); // validate after EACH insert (catches transient bugs)
   }
}

template <class K, class V>
static void expect_all_found(BTree<K, V> &b, const std::vector<std::pair<K, V>> &data) {
   for (const auto &kv : data) {
      REQUIRE(b.find(kv.first) == kv.second);
   }
}

template <class K, class V>
static std::vector<std::pair<K, V>> make_seq_data(int n) {
   std::vector<std::pair<K, V>> out;
   out.reserve(n);
   for (int i = 0; i < n; i++)
      out.emplace_back((K)i, (V)i);
   return out;
}

static std::vector<int> make_perm(int n, uint32_t seed) {
   std::vector<int> v(n);
   std::iota(v.begin(), v.end(), 0);
   std::mt19937 rng(seed);
   std::shuffle(v.begin(), v.end(), rng);
   return v;
}

static std::vector<int> make_desc(int n) {
   std::vector<int> v(n);
   for (int i = 0; i < n; i++)
      v[i] = n - 1 - i;
   return v;
}

// ---------- insertion_idx tests ----------

TEST_CASE("insertion_idx: empty vector returns 0", "[btree][part=1][insertion_idx]") {
   std::vector<int> v;
   REQUIRE(insertion_idx(v, 123) == 0);
}

TEST_CASE("insertion_idx: single element", "[btree][part=1][insertion_idx]") {
   std::vector<int> v = {10};
   REQUIRE(insertion_idx(v, 5) == 0);
   REQUIRE(insertion_idx(v, 10) == 0);
   REQUIRE(insertion_idx(v, 99) == 1);
}

TEST_CASE("insertion_idx: returns lower_bound position on typical inputs",
          "[btree][part=1][insertion_idx]") {
   std::vector<int> v = {1, 3, 5, 7};
   REQUIRE(insertion_idx(v, -100) == 0);
   REQUIRE(insertion_idx(v, 1) == 0);
   REQUIRE(insertion_idx(v, 2) == 1);
   REQUIRE(insertion_idx(v, 3) == 1);
   REQUIRE(insertion_idx(v, 4) == 2);
   REQUIRE(insertion_idx(v, 7) == 3);
   REQUIRE(insertion_idx(v, 100) == 4);
}

TEST_CASE("insertion_idx: duplicates should return index of an existing equal key",
          "[btree][part=1][insertion_idx]") {
   // Your spec: "If val occurs in elements, return the index of val in elements."
   // For duplicates, any index containing val is acceptable, but we can still check:
   // returned index must be in-range and equal, OR be the insertion position among equals.
   std::vector<int> v = {1, 2, 2, 2, 5};

   size_t idx = insertion_idx(v, 2);
   REQUIRE(idx < v.size());
   REQUIRE(v[idx] == 2);

   REQUIRE(insertion_idx(v, 0) == 0);
   REQUIRE(insertion_idx(v, 6) == v.size());
}

TEST_CASE("insertion_idx: all equal elements", "[btree][part=1][insertion_idx]") {
   std::vector<int> v(20, 7);
   size_t idx = insertion_idx(v, 7);
   REQUIRE(idx < v.size());
   REQUIRE(v[idx] == 7);

   REQUIRE(insertion_idx(v, 6) == 0);
   REQUIRE(insertion_idx(v, 8) == v.size());
}

// ---------- BTree basic behavior ----------

TEST_CASE("BTree: find on empty tree returns default V()", "[btree][part=1]") {
   BTree<int, int> b(3);
   REQUIRE(b.find(12345) == int());
   REQUIRE(b.is_valid(3));
}

TEST_CASE("BTree: inserting same key twice does nothing (keeps first value)", "[btree][part=1]") {
   BTree<int, int> b(3);
   b.insert(5, 111);
   REQUIRE(b.is_valid(3));
   REQUIRE(b.find(5) == 111);

   // spec says: "If key is already in the tree do nothing."
   b.insert(5, 999);
   REQUIRE(b.is_valid(3));
   REQUIRE(b.find(5) == 111);
}

TEST_CASE("BTree: works with negative and extreme int keys", "[btree][part=1]") {
   BTree<int, int> b(3);
   std::vector<std::pair<int, int>> data = {
       {0, 0},
       {-1, 7},
       {std::numeric_limits<int>::min(), 42},
       {std::numeric_limits<int>::max(), 99},
       {-999999, 123},
       {999999, 456},
   };
   insert_all(b, data, 3);
   expect_all_found(b, data);
   REQUIRE(b.find(123456789) == 0);
}

// ---------- Order 3: split + cascading split stress (deterministic) ----------

TEST_CASE("BTree order=3: ascending inserts cause many splits and should remain valid",
          "[btree][part=1][split][order3]") {
   constexpr unsigned ORDER = 3;
   BTree<int, int> b(ORDER);

   // 0..199 causes lots of splits, including height increases
   std::vector<std::pair<int, int>> data = make_seq_data<int, int>(200);

   insert_all(b, data, ORDER);
   expect_all_found(b, data);
   REQUIRE(b.find(-123) == 0);
}

TEST_CASE("BTree order=3: descending inserts stress child-index logic",
          "[btree][part=1][split][order3]") {
   constexpr unsigned ORDER = 3;
   BTree<int, int> b(ORDER);

   auto keys = make_desc(200);
   std::vector<std::pair<int, int>> data;
   data.reserve(keys.size());
   for (int k : keys)
      data.emplace_back(k, k * 10);

   insert_all(b, data, ORDER);
   expect_all_found(b, data);
   REQUIRE(b.find(201) == 0);
}

TEST_CASE("BTree order=3: zig-zag inserts stress insertion_idx and bar/child alignment",
          "[btree][part=1][split][order3]") {
   constexpr unsigned ORDER = 3;
   BTree<int, int> b(ORDER);

   // Zig-zag: 0, 199, 1, 198, 2, 197, ...
   std::vector<std::pair<int, int>> data;
   data.reserve(200);
   int lo = 0, hi = 199;
   while (lo <= hi) {
      data.emplace_back(lo, lo);
      if (lo != hi)
         data.emplace_back(hi, hi);
      lo++;
      hi--;
   }

   insert_all(b, data, ORDER);
   expect_all_found(b, data);
}

TEST_CASE("BTree order=3: permutation inserts (seeded) to catch rare split_child bugs",
          "[btree][part=1][split][order3]") {
   constexpr unsigned ORDER = 3;
   BTree<int, int> b(ORDER);

   auto perm = make_perm(500, 1337);
   std::vector<std::pair<int, int>> data;
   data.reserve(perm.size());
   for (int k : perm)
      data.emplace_back(k, k + 1);

   insert_all(b, data, ORDER);
   expect_all_found(b, data);
   REQUIRE(b.find(999999) == 0);
}

// ---------- Higher orders: still should behave (less splitting, but still possible) ----------

TEST_CASE("BTree order=5: deterministic mixed inserts", "[btree][part=1][order5]") {
   constexpr unsigned ORDER = 5;
   BTree<int, int> b(ORDER);

   std::vector<int> keys = {10, 2, 14, 1, 7, 5, 8, 3, 12, 0, 6, 4, 9, 11, 13};
   std::vector<std::pair<int, int>> data;
   data.reserve(keys.size());
   for (int k : keys)
      data.emplace_back(k, k * k);

   insert_all(b, data, ORDER);
   expect_all_found(b, data);
}

TEST_CASE("BTree order=64: large sequential inserts, validate periodically",
          "[btree][part=1][order64]") {
   constexpr unsigned ORDER = 64;
   BTree<int, int> b(ORDER);

   // 0..49999
   for (int i = 0; i < 50000; i++) {
      b.insert(i, i);
      if (i % 997 == 0) { // periodic validation
         REQUIRE(b.is_valid(ORDER));
      }
   }
   REQUIRE(b.is_valid(ORDER));

   // spot checks
   REQUIRE(b.find(0) == 0);
   REQUIRE(b.find(12345) == 12345);
   REQUIRE(b.find(49999) == 49999);
   REQUIRE(b.find(-1) == 0);
}

// ---------- Types: strings/floats ----------

TEST_CASE("BTree: strings keys/values, order=3, deterministic", "[btree][part=1][types]") {
   constexpr unsigned ORDER = 3;
   BTree<std::string, std::string> b(ORDER);

   std::vector<std::pair<std::string, std::string>> data = {
       {"delta", "D"}, {"alpha", "A"},   {"charlie", "C"}, {"bravo", "B"},
       {"echo", "E"},  {"foxtrot", "F"}, {"golf", "G"},
   };

   insert_all(b, data, ORDER);
   expect_all_found(b, data);

   REQUIRE(b.find("not-present") == std::string());
}

TEST_CASE("BTree: float keys, order=5, handles negative and non-integer",
          "[btree][part=1][types]") {
   constexpr unsigned ORDER = 5;
   BTree<float, float> b(ORDER);

   std::vector<std::pair<float, float>> data = {
       {3.14f, 1.0f}, {-2.5f, 2.0f}, {0.0f, 3.0f}, {1.25f, 4.0f}, {9.75f, 5.0f}, {-100.0f, 6.0f},
   };

   insert_all(b, data, ORDER);
   expect_all_found(b, data);

   REQUIRE(b.find(123.0f) == float());
}

// ---------- Regression-style test: validate find on keys not inserted ----------

TEST_CASE("BTree: missing keys always return default", "[btree][part=1][missing]") {
   constexpr unsigned ORDER = 3;
   BTree<int, int> b(ORDER);

   std::vector<std::pair<int, int>> data = make_seq_data<int, int>(100);
   insert_all(b, data, ORDER);

   for (int k : {-1000, -1, 100, 101, 9999}) {
      REQUIRE(b.find(k) == 0);
   }
}
