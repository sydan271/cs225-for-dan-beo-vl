
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include "heap.h"

using std::vector;

// ------------------------------------------
// Helpers
// ------------------------------------------

// Convert heap internal array (via getElems) into 1-indexed array for easy parent/child math.
template <typename T, typename Compare>
static vector<T> heapArray1Indexed(const heap<T, Compare> &h) {
   vector<T> arr;
   h.getElems(arr);              // arr is _elems[1..]
   arr.insert(arr.begin(), T()); // dummy at index 0 so arr[1] is root
   return arr;
}

template <typename T, typename Compare>
static void requireHeapInvariant(const heap<T, Compare> &h, Compare cmp = Compare()) {
   auto a = heapArray1Indexed<T, Compare>(h);
   // For every child i, parent = i/2 must have >= priority than child.
   // "cmp(x,y) == true" means x has higher priority than y.
   for (size_t i = 2; i < a.size(); i++) {
      size_t p = i / 2;
      // child should NOT have higher priority than parent
      REQUIRE_FALSE(cmp(a[i], a[p]));
   }
}

template <typename T, typename Compare>
static vector<T> popAll(heap<T, Compare> &h) {
   vector<T> out;
   while (!h.empty())
      out.push_back(h.pop());
   return out;
}

template <typename T, typename Compare>
static void requirePopOrderMatchesSorted(vector<T> vals, Compare cmp = Compare()) {
   heap<T, Compare> h;
   for (auto &x : vals)
      h.push(x);

   requireHeapInvariant<T, Compare>(h, cmp);

   vector<T> popped = popAll<T, Compare>(h);

   // "sorted by priority" means: if cmp(a,b) => a before b in pop order.
   // For std::less: ascending. For std::greater: descending.
   std::sort(vals.begin(), vals.end(), [&](const T &a, const T &b) { return cmp(a, b); });

   REQUIRE(popped == vals);
}

// ------------------------------------------
// Core behavior tests
// ------------------------------------------

TEST_CASE("heap: empty behavior (peek/pop/empty)", "[heap]") {
   heap<int> h;
   REQUIRE(h.empty());

   // Your implementation returns default T() on empty pop/peek.
   REQUIRE(h.peek() == int());
   REQUIRE(h.pop() == int());
   REQUIRE(h.empty());
}

TEST_CASE("heap: push/peek doesn't remove, pop removes", "[heap]") {
   heap<int> h;
   h.push(5);
   h.push(3);
   h.push(7);

   REQUIRE_FALSE(h.empty());
   REQUIRE(h.peek() == 3); // min-heap
   REQUIRE(h.peek() == 3); // still there
   REQUIRE(h.pop() == 3);  // removed
   REQUIRE(h.peek() == 5);
   requireHeapInvariant<int, std::less<int>>(h);
}

TEST_CASE("heap: duplicates + negatives pop in ascending order", "[heap]") {
   vector<int> vals = {5, -1, 5, 0, -1, 2, 2, 2, -10};
   requirePopOrderMatchesSorted<int, std::less<int>>(vals);
}

// ------------------------------------------
// BuildHeap constructor tests
// ------------------------------------------

TEST_CASE("heap(buildHeap): empty vector", "[heap][buildheap]") {
   vector<int> vals;
   heap<int> h(vals);
   REQUIRE(h.empty());
}

TEST_CASE("heap(buildHeap): invariant + correct pop order (small)", "[heap][buildheap]") {
   vector<int> vals = {5, 7, 2, 9, 8, 1};
   heap<int> h(vals);

   requireHeapInvariant<int, std::less<int>>(h);

   auto popped = popAll<int, std::less<int>>(h);
   auto sorted = vals;
   std::sort(sorted.begin(), sorted.end());
   REQUIRE(popped == sorted);
}

TEST_CASE("heap(buildHeap): randomized stress", "[heap][buildheap]") {
   std::mt19937 rng(225);
   std::uniform_int_distribution<int> dist(-100000, 100000);

   for (int trial = 0; trial < 200; trial++) {
      int n = 1 + (rng() % 200);
      vector<int> vals;
      vals.reserve(n);
      for (int i = 0; i < n; i++)
         vals.push_back(dist(rng));

      heap<int> h(vals);
      requireHeapInvariant<int, std::less<int>>(h);

      auto popped = popAll<int, std::less<int>>(h);
      auto sorted = vals;
      std::sort(sorted.begin(), sorted.end());
      REQUIRE(popped == sorted);
   }
}

// ------------------------------------------
// updateElem tests
// ------------------------------------------

TEST_CASE("heap(updateElem): decrease a deep element -> must bubble up", "[heap][updateElem]") {
   vector<int> vals = {50, 40, 30, 20, 10, 60, 70, 80, 90};
   heap<int> h(vals);
   requireHeapInvariant<int, std::less<int>>(h);

   // Pick a non-root valid index and decrease it a lot.
   // We don't know exact layout, but index 2 exists if size >= 2.
   auto a = heapArray1Indexed<int, std::less<int>>(h);
   REQUIRE(a.size() > 3); // ensure index 3 exists

   // Make node at index 3 the new minimum
   h.updateElem(3, -999999);
   requireHeapInvariant<int, std::less<int>>(h);
   REQUIRE(h.peek() == -999999);

   // Verify full pop order matches "vals with that slot replaced"
   // We can't know which value was at index 3 originally, so instead:
   // Just ensure popped is sorted and contains -999999.
   auto popped = popAll<int, std::less<int>>(h);
   REQUIRE(std::is_sorted(popped.begin(), popped.end()));
   REQUIRE(std::find(popped.begin(), popped.end(), -999999) != popped.end());
}

TEST_CASE("heap(updateElem): increase root -> must heapify down", "[heap][updateElem]") {
   vector<int> vals = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   heap<int> h(vals);
   requireHeapInvariant<int, std::less<int>>(h);

   REQUIRE(h.peek() == 1);
   h.updateElem(h.root(), 1000000); // make root huge
   requireHeapInvariant<int, std::less<int>>(h);

   // new min should be 2 now (since 1 got increased)
   REQUIRE(h.peek() == 2);

   auto popped = popAll<int, std::less<int>>(h);
   REQUIRE(std::is_sorted(popped.begin(), popped.end()));
   REQUIRE(popped.back() == 1000000);
}

TEST_CASE("heap(updateElem): out-of-range index should not crash", "[heap][updateElem]") {
   heap<int> h;
   h.updateElem(0, 123);   // invalid
   h.updateElem(999, 123); // invalid
   REQUIRE(h.empty());

   h.push(5);
   h.push(1);
   h.updateElem(999, 123); // invalid, should do nothing
   requireHeapInvariant<int, std::less<int>>(h);
   REQUIRE(h.pop() == 1);
   REQUIRE(h.pop() == 5);
}

// ------------------------------------------
// Type + comparator coverage
// ------------------------------------------

TEST_CASE("heap: strings with default comparator (min-heap lexicographic)", "[heap]") {
   vector<std::string> vals = {"c", "c++", "java", "python", "go", "rust", "c"};
   requirePopOrderMatchesSorted<std::string, std::less<std::string>>(vals);
}

TEST_CASE("heap: max-heap via std::greater pops descending", "[heap]") {
   vector<int> vals = {5, 1, 9, 9, -3, 7, 0};
   requirePopOrderMatchesSorted<int, std::greater<int>>(vals, std::greater<int>{});
}
