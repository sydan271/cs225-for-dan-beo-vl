// tests_student.cpp (custom, not graded)
#include <catch2/catch_approx.hpp>
// Comprehensive tests that avoid doing -- on end() (nullptr end-iterator)

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

#include "List.h"

// ---------- helpers ----------
static std::string toString(List<int> &l) {
  std::stringstream ss;
  l.print(ss);
  return ss.str();
}

static List<int> makeList(std::initializer_list<int> vals) {
  List<int> l;
  for (int v : vals)
    l.insertBack(v);
  return l;
}

static std::vector<int> toVector(List<int> &l) {
  std::vector<int> out;
  for (auto it = l.begin(); it != l.end(); ++it) out.push_back(*it);
  return out;
}

static bool isNonDecreasing(List<int> &l) {
  auto it = l.begin();
  auto ed = l.end();
  if (it == ed) return true;
  int prev = *it;
  ++it;
  for (; it != ed; ++it) {
    if (*it < prev) return false;
    prev = *it;
  }
  return true;
}

// ---------- Part 1: iterator basics, insert, split, waterfall, Big Three ----------
TEST_CASE("Iterator: empty list begin==end and forward iteration yields nothing",
          "[custom][part=1][valgrind]") {
  List<int> l;
  REQUIRE(l.size() == 0);
  REQUIRE(l.begin() == l.end());

  std::vector<int> v = toVector(l);
  REQUIRE(v.empty());
}

TEST_CASE("InsertBack: forward iteration preserves order",
          "[custom][part=1][valgrind]") {
  List<int> l;
  l.insertBack(10);
  l.insertBack(20);
  l.insertBack(30);

  REQUIRE(l.size() == 3);
  REQUIRE(toVector(l) == std::vector<int>({10, 20, 30}));
  REQUIRE(toString(l) == "< 10 20 30 >");
}

TEST_CASE("InsertFront: forward iteration preserves order",
          "[custom][part=1][valgrind]") {
  List<int> l;
  l.insertFront(10);
  l.insertFront(20);
  l.insertFront(30);

  REQUIRE(l.size() == 3);
  REQUIRE(toVector(l) == std::vector<int>({30, 20, 10}));
  REQUIRE(toString(l) == "< 30 20 10 >");
}

TEST_CASE("Iterator ++: prefix and postfix semantics (no decrement-from-end)",
          "[custom][part=1][valgrind]") {
  auto l = makeList({1, 2, 3, 4});
  auto it = l.begin();

  // postfix returns old value
  auto old = it++;
  REQUIRE(*old == 1);
  REQUIRE(*it == 2);

  // prefix advances and returns reference to new position
  auto &ref = ++it;
  REQUIRE(&ref == &it);
  REQUIRE(*it == 3);

  ++it;
  REQUIRE(*it == 4);

  ++it;
  REQUIRE(it == l.end()); // ok: we compare to end, but don't -- from it
}

TEST_CASE("Iterator --: works from interior positions (never from end())",
          "[custom][part=1][valgrind]") {
  auto l = makeList({5, 6, 7, 8});
  auto it = l.begin();

  ++it; // now at 6
  REQUIRE(*it == 6);

  // prefix --
  --it; // back to 5
  REQUIRE(*it == 5);

  // move to 7
  ++it; ++it;
  REQUIRE(*it == 7);

  // postfix -- returns old value, then moves back
  auto old = it--;
  REQUIRE(*old == 7);
  REQUIRE(*it == 6);
}

TEST_CASE("Iterator comparisons: == and != behave by position",
          "[custom][part=1][valgrind]") {
  auto l = makeList({1, 2, 3});
  auto a = l.begin();
  auto b = l.begin();
  REQUIRE(a == b);
  REQUIRE_FALSE(a != b);

  ++b;
  REQUIRE(a != b);
  REQUIRE_FALSE(a == b);

  ++a;
  REQUIRE(a == b);
}

TEST_CASE("Split: split in the middle makes two correct lists",
          "[custom][part=1][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5});
  List<int> r = l.split(2);

  REQUIRE(toString(l) == "< 1 2 >");
  REQUIRE(toString(r) == "< 3 4 5 >");

  // forward iteration correctness
  REQUIRE(toVector(l) == std::vector<int>({1, 2}));
  REQUIRE(toVector(r) == std::vector<int>({3, 4, 5}));

  // independence: mutating one doesn't touch the other
  l.insertBack(99);
  r.insertFront(77);
  REQUIRE(toString(l) == "< 1 2 99 >");
  REQUIRE(toString(r) == "< 77 3 4 5 >");
}

TEST_CASE("Split: split at size-1 leaves one element in returned list",
          "[custom][part=1][valgrind]") {
  auto l = makeList({9, 8, 7, 6});
  List<int> r = l.split(3);

  REQUIRE(toString(l) == "< 9 8 7 >");
  REQUIRE(toString(r) == "< 6 >");
}

TEST_CASE("Waterfall: empty/1/2 are no-ops",
          "[custom][part=1][valgrind]") {
  {
    List<int> l;
    l.waterfall();
    REQUIRE(toString(l) == "< >");
  }
  {
    auto l = makeList({1});
    l.waterfall();
    REQUIRE(toString(l) == "< 1 >");
  }
  {
    auto l = makeList({1, 2});
    l.waterfall();
    REQUIRE(toString(l) == "< 1 2 >");
  }
}

TEST_CASE("Waterfall: small sizes match expected outputs",
          "[custom][part=1][valgrind]") {
  {
    auto l = makeList({1, 2, 3});
    l.waterfall();
    REQUIRE(toString(l) == "< 1 3 2 >");
  }
  {
    auto l = makeList({1, 2, 3, 4});
    l.waterfall();
    REQUIRE(toString(l) == "< 1 3 2 4 >");
  }
  {
    auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8});
    l.waterfall();
    REQUIRE(toString(l) == "< 1 3 5 7 2 6 4 8 >");
  }
}

TEST_CASE("Copy ctor: deep copy (mutating original doesn't change copy)",
          "[custom][part=1][valgrind]") {
  auto a = makeList({1, 2, 3});
  List<int> b(a);

  a.insertBack(99);
  REQUIRE(toString(a) == "< 1 2 3 99 >");
  REQUIRE(toString(b) == "< 1 2 3 >");

  // ensure iterator traversal still works on copy
  REQUIRE(toVector(b) == std::vector<int>({1, 2, 3}));
}

TEST_CASE("Assignment: deep copy + safe self-assignment (no compiler warning)",
          "[custom][part=1][valgrind]") {
  auto a = makeList({4, 5, 6});
  List<int> b;
  b = a;

  a.insertFront(3);
  REQUIRE(toString(a) == "< 3 4 5 6 >");
  REQUIRE(toString(b) == "< 4 5 6 >");

  // self-assignment test without b=b warning
  List<int>* p = &b;
  b = *p;
  REQUIRE(toString(b) == "< 4 5 6 >");
}

TEST_CASE("Destructor: deleting non-empty list doesn't crash",
          "[custom][part=1][valgrind]") {
  auto *l = new List<int>();
  for (int i = 0; i < 200; i++) l->insertBack(i);
  delete l;
  SUCCEED();
}

// ---------- Part 2: reverse, reverseNth, mergeWith, sort ----------
TEST_CASE("Reverse: empty and size-1 are no-ops",
          "[custom][part=2][valgrind]") {
  {
    List<int> l;
    l.reverse();
    REQUIRE(toString(l) == "< >");
  }
  {
    auto l = makeList({7});
    l.reverse();
    REQUIRE(toString(l) == "< 7 >");
  }
}

TEST_CASE("Reverse: even and odd lengths reverse correctly",
          "[custom][part=2][valgrind]") {
  {
    auto l = makeList({1, 2, 3, 4});
    l.reverse();
    REQUIRE(toString(l) == "< 4 3 2 1 >");
  }
  {
    auto l = makeList({1, 2, 3, 4, 5});
    l.reverse();
    REQUIRE(toString(l) == "< 5 4 3 2 1 >");
  }
}

TEST_CASE("ReverseNth: n=1 no-op; n>len behaves like reverse",
          "[custom][part=2][valgrind]") {
  {
    auto l = makeList({1, 2, 3, 4, 5});
    l.reverseNth(1);
    REQUIRE(toString(l) == "< 1 2 3 4 5 >");
  }
  {
    auto l = makeList({1, 2, 3, 4, 5});
    l.reverseNth(999);
    REQUIRE(toString(l) == "< 5 4 3 2 1 >");
  }
}

TEST_CASE("ReverseNth: exact blocks and leftover block",
          "[custom][part=2][valgrind]") {
  {
    auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8});
    l.reverseNth(2);
    REQUIRE(toString(l) == "< 2 1 4 3 6 5 8 7 >");
  }
  {
    auto l = makeList({1, 2, 3, 4, 5, 6});
    l.reverseNth(4);
    REQUIRE(toString(l) == "< 4 3 2 1 6 5 >");
  }
}

TEST_CASE("MergeWith: merging empty/non-empty and interleaved lists",
          "[custom][part=2][valgrind]") {
  {
    List<int> a;
    auto b = makeList({1, 3, 5});
    a.mergeWith(b);
    REQUIRE(toString(a) == "< 1 3 5 >");
    REQUIRE(toString(b) == "< >");
  }
  {
    auto a = makeList({1, 2, 3});
    List<int> b;
    a.mergeWith(b);
    REQUIRE(toString(a) == "< 1 2 3 >");
    REQUIRE(toString(b) == "< >");
  }
  {
    auto a = makeList({1, 3, 4, 6});
    auto b = makeList({2, 5, 7});
    a.mergeWith(b);
    REQUIRE(toString(a) == "< 1 2 3 4 5 6 7 >");
    REQUIRE(toString(b) == "< >");
  }
}

TEST_CASE("Sort: empty/single ok; random + duplicates sorted nondecreasing",
          "[custom][part=2][valgrind]") {
  {
    List<int> l;
    l.sort();
    REQUIRE(toString(l) == "< >");
  }
  {
    auto l = makeList({42});
    l.sort();
    REQUIRE(toString(l) == "< 42 >");
  }
  {
    auto l = makeList({9, 8, 7, 6, 5, 4, 3, 2, 1});
    l.sort();
    REQUIRE(isNonDecreasing(l));
    REQUIRE(toString(l) == "< 1 2 3 4 5 6 7 8 9 >");
  }
  {
    auto l = makeList({3, 1, 2, 3, 2, 1, 1});
    l.sort();
    REQUIRE(isNonDecreasing(l));
    REQUIRE(toString(l) == "< 1 1 1 2 2 3 3 >");
  }
}

// ---------- Optional: operator-> sanity without touching end() ----------
struct Pair {
  int a;
  int b;
};

TEST_CASE("Iterator operator->: access members through iterator",
          "[custom][part=1][valgrind]") {
  List<Pair> l;
  l.insertBack(Pair{1, 10});
  l.insertBack(Pair{2, 20});

  auto it = l.begin();
  REQUIRE(it->a == 1);
  REQUIRE(it->b == 10);

  ++it;
  REQUIRE(it->a == 2);
  REQUIRE(it->b == 20);
}

// tests_student_pixels.cpp
#include "tests_helper.h"
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"
#include "List.h"

using cs225::HSLAPixel;
using cs225::PNG;

// ---------- helpers ----------
static HSLAPixel P(double h, double s, double l, double a = 1.0) {
  return HSLAPixel(h, s, l, a);
}

static std::vector<HSLAPixel> toVector(List<HSLAPixel> &l) {
  std::vector<HSLAPixel> out;
  for (auto it = l.begin(); it != l.end(); ++it) out.push_back(*it);
  return out;
}

static bool isSortedByLess(const std::vector<HSLAPixel> &v) {
  for (size_t i = 1; i < v.size(); i++) {
    if (v[i] < v[i - 1]) return false;
  }
  return true;
}

// Compares two PNGs pixel-by-pixel (exact HSLAPixel equality).
static void requireSameImage(const PNG &a, const PNG &b) {
  REQUIRE(a.width() == b.width());
  REQUIRE(a.height() == b.height());
  for (unsigned x = 0; x < a.width(); x++) {
    for (unsigned y = 0; y < a.height(); y++) {
      REQUIRE(a.getPixel(x, y) == b.getPixel(x, y));
    }
  }
}

// Build a small image with unique pixels in (x-major, then y) order.
// This matches tests_helper::imageToList scanning: for x in [0..w), for y in [0..h). 
static PNG makePatternImage(unsigned w, unsigned h) {
  PNG img;
  img.resize(w, h);

  // Unique pixels by coordinate, stable exact values.
  for (unsigned x = 0; x < w; x++) {
    for (unsigned y = 0; y < h; y++) {
      // hue encodes x,y uniquely; s/l/a fixed but nontrivial
      double hue = 10.0 * x + 1.0 * y;     // small integers
      img.getPixel(x, y) = P(hue, 0.6, 0.4, 1.0);
    }
  }
  return img;
}

static std::vector<HSLAPixel> expectedScanOrder(const PNG &img) {
  std::vector<HSLAPixel> v;
  for (unsigned x = 0; x < img.width(); x++) {
    for (unsigned y = 0; y < img.height(); y++) {
      v.push_back(img.getPixel(x, y));
    }
  }
  return v;
}

// ---------- tests ----------

TEST_CASE("HSLAPixel: equality and inequality basics", "[custom][pixel][part=1]") {
  HSLAPixel a = P(120, 0.5, 0.25, 0.75);
  HSLAPixel b = P(120, 0.5, 0.25, 0.75);
  HSLAPixel c = P(121, 0.5, 0.25, 0.75);

  REQUIRE(a == b);
  REQUIRE_FALSE(a != b);
  REQUIRE(a != c);
}

TEST_CASE("List<HSLAPixel>: insertBack preserves order via forward iteration",
          "[custom][pixel][part=1][valgrind]") {
  List<HSLAPixel> l;
  l.insertBack(P(10, 0.1, 0.2));
  l.insertBack(P(20, 0.2, 0.3));
  l.insertBack(P(30, 0.3, 0.4));

  auto v = toVector(l);
  REQUIRE(v.size() == 3);
  REQUIRE(v[0] == P(10, 0.1, 0.2));
  REQUIRE(v[1] == P(20, 0.2, 0.3));
  REQUIRE(v[2] == P(30, 0.3, 0.4));
}

TEST_CASE("List<HSLAPixel>: insertFront reverses order via forward iteration",
          "[custom][pixel][part=1][valgrind]") {
  List<HSLAPixel> l;
  l.insertFront(P(10, 0.1, 0.2));
  l.insertFront(P(20, 0.2, 0.3));
  l.insertFront(P(30, 0.3, 0.4));

  auto v = toVector(l);
  REQUIRE(v.size() == 3);
  REQUIRE(v[0] == P(30, 0.3, 0.4));
  REQUIRE(v[1] == P(20, 0.2, 0.3));
  REQUIRE(v[2] == P(10, 0.1, 0.2));
}

TEST_CASE("List<HSLAPixel>: iterator operator-> accesses HSLAPixel fields",
          "[custom][pixel][part=1]") {
  List<HSLAPixel> l;
  l.insertBack(P(123, 0.7, 0.2, 0.9));

  auto it = l.begin();
  REQUIRE(it->h == 123);
  REQUIRE(it->s == Catch::Approx(0.7));
  REQUIRE(it->l == Catch::Approx(0.2));
  REQUIRE(it->a == Catch::Approx(0.9));
}

TEST_CASE("List<HSLAPixel>: -- works from interior (never from end())",
          "[custom][pixel][part=1]") {
  List<HSLAPixel> l;
  l.insertBack(P(1, 0.2, 0.2));
  l.insertBack(P(2, 0.2, 0.2));
  l.insertBack(P(3, 0.2, 0.2));

  auto it = l.begin();
  ++it; // now at "2"
  REQUIRE((*it).h == 2);

  --it; // back to "1"
  REQUIRE((*it).h == 1);
}

TEST_CASE("imageToList/listToImage round-trip preserves pixels and order",
          "[custom][pixel][part=1][valgrind]") {
  PNG img = makePatternImage(3, 2);

  // to list in scan order (x-major then y)
  List<HSLAPixel> l = imageToList(img, false);
  auto expected = expectedScanOrder(img);
  REQUIRE(toVector(l) == expected);

  // back to image should match exactly
  PNG out = listToImage(l, img.width(), img.height());
  requireSameImage(img, out);
}

TEST_CASE("imageToList(reverse=true) reverses scan order (insertFront path)",
          "[custom][pixel][part=1][valgrind]") {
  PNG img = makePatternImage(3, 2);
  auto expected = expectedScanOrder(img);
  std::reverse(expected.begin(), expected.end());

  List<HSLAPixel> l = imageToList(img, true);
  REQUIRE(toVector(l) == expected);
}

TEST_CASE("Waterfall: HSLAPixel list rearranges like integer example",
          "[custom][pixel][part=1][valgrind]") {
  List<HSLAPixel> l;
  for (int i = 1; i <= 8; i++) {
    l.insertBack(P(i, 0.5, 0.5));
  }

  l.waterfall();
  auto v = toVector(l);

  std::vector<int> got;
  for (auto &px : v) got.push_back((int)px.h);

  REQUIRE(got == std::vector<int>({1, 3, 5, 7, 2, 6, 4, 8}));
}

TEST_CASE("Split: HSLAPixel list splits and remains independent",
          "[custom][pixel][part=1][valgrind]") {
  List<HSLAPixel> l;
  for (int i = 1; i <= 5; i++) l.insertBack(P(i, 0.1 * i, 0.2));

  List<HSLAPixel> r = l.split(2);

  // left: 1 2
  auto lv = toVector(l);
  REQUIRE(lv.size() == 2);
  REQUIRE((int)lv[0].h == 1);
  REQUIRE((int)lv[1].h == 2);

  // right: 3 4 5
  auto rv = toVector(r);
  REQUIRE(rv.size() == 3);
  REQUIRE((int)rv[0].h == 3);
  REQUIRE((int)rv[1].h == 4);
  REQUIRE((int)rv[2].h == 5);

  // independence
  l.insertBack(P(99, 0.9, 0.9));
  REQUIRE(toVector(r).size() == 3);
}

TEST_CASE("Reverse: HSLAPixel list reverses correctly (forward traversal check)",
          "[custom][pixel][part=2][valgrind]") {
  List<HSLAPixel> l;
  for (int i = 1; i <= 5; i++) l.insertBack(P(i, 0.2, 0.2));

  l.reverse();
  auto v = toVector(l);

  std::vector<int> got;
  for (auto &px : v) got.push_back((int)px.h);

  REQUIRE(got == std::vector<int>({5, 4, 3, 2, 1}));
}

TEST_CASE("ReverseNth: HSLAPixel blocks reverse correctly",
          "[custom][pixel][part=2][valgrind]") {
  List<HSLAPixel> l;
  for (int i = 1; i <= 6; i++) l.insertBack(P(i, 0.2, 0.2));

  l.reverseNth(4);
  auto v = toVector(l);

  std::vector<int> got;
  for (auto &px : v) got.push_back((int)px.h);

  REQUIRE(got == std::vector<int>({4, 3, 2, 1, 6, 5}));
}

TEST_CASE("MergeWith: HSLAPixel merge keeps sorted order by operator<",
          "[custom][pixel][part=2][valgrind]") {
  // Construct two sorted lists under operator< by using increasing hues
  List<HSLAPixel> a;
  a.insertBack(P(10, 0.2, 0.2));
  a.insertBack(P(30, 0.2, 0.2));
  a.insertBack(P(50, 0.2, 0.2));

  List<HSLAPixel> b;
  b.insertBack(P(20, 0.2, 0.2));
  b.insertBack(P(40, 0.2, 0.2));

  a.mergeWith(b);

  auto v = toVector(a);
  REQUIRE(v.size() == 5);
  REQUIRE(isSortedByLess(v));   // doesn't assume exact comparison rule, just consistency

  REQUIRE(toVector(b).empty()); // other should be emptied per MP mergeWith contract
}

TEST_CASE("Sort: HSLAPixel list sorts consistently with operator<",
          "[custom][pixel][part=2][valgrind]") {
  List<HSLAPixel> l;
  std::vector<HSLAPixel> orig;

  // Exact values (integers/clean decimals) so == is stable
  orig.push_back(P(300, 0.1, 0.1));
  orig.push_back(P(10,  0.9, 0.2));
  orig.push_back(P(200, 0.2, 0.8));
  orig.push_back(P(10,  0.1, 0.1));  // duplicate hue but different s/l
  orig.push_back(P(150, 0.5, 0.5));

  for (auto &px : orig) l.insertBack(px);

  l.sort();
  auto got = toVector(l);

  // Compare to std::sort under the same operator<
  auto expected = orig;
  std::sort(expected.begin(), expected.end(),
            [](const HSLAPixel &x, const HSLAPixel &y) { return x < y; });

  REQUIRE(got == expected);
}

// --- new helper (only one) ---
static void requireNoCycleAndCountMatchesSize(List<int>& l) {
  int n = l.size();
  auto it = l.begin();
  int count = 0;

  // If there's a cycle, this would exceed n before reaching end()
  for (; it != l.end() && count <= n; ++it) {
    count++;
  }

  REQUIRE(count == n);
  REQUIRE(it == l.end());
}

// --- additional reverse() tests ---

TEST_CASE("Reverse: duplicates + negatives reverse correctly",
          "[custom][part=2][reverse][valgrind]") {
  auto l = makeList({0, -1, -1, 2, 2, 7});
  l.reverse();
  REQUIRE(toString(l) == "< 7 2 2 -1 -1 0 >");
  REQUIRE(toVector(l) == std::vector<int>({7, 2, 2, -1, -1, 0}));
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("Reverse: long list (100 elems) has correct endpoints and terminates",
          "[custom][part=2][reverse][valgrind]") {
  List<int> l;
  for (int i = 0; i < 100; i++) l.insertBack(i);

  l.reverse();

  auto v = toVector(l);
  REQUIRE((int)v.size() == 100);
  REQUIRE(v.front() == 99);
  REQUIRE(v.back() == 0);

  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("Reverse: prev links consistent via interior -- (no --end())",
          "[custom][part=2][reverse][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5});
  l.reverse(); // 5 4 3 2 1

  auto it = l.begin();
  ++it; ++it;              // now pointing at 3
  REQUIRE(*it == 3);

  auto left = it;
  --left;                  // should go to 4
  REQUIRE(*left == 4);
  --left;                  // should go to 5
  REQUIRE(*left == 5);

  auto right = it;
  ++right;                 // should go to 2
  REQUIRE(*right == 2);
}

TEST_CASE("Reverse: reverse three times equals single reverse",
          "[custom][part=2][reverse][valgrind]") {
  auto l = makeList({3, 1, 4, 1, 5, 9});
  auto orig = toVector(l);

  l.reverse();
  auto once = toVector(l);

  l.reverse();
  l.reverse();
  auto thrice = toVector(l);

  std::reverse(orig.begin(), orig.end());
  REQUIRE(once == orig);
  REQUIRE(thrice == orig);

  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("Reverse: works on lists built via mixed insertFront/insertBack",
          "[custom][part=2][reverse][valgrind]") {
  List<int> l;
  l.insertBack(2);
  l.insertFront(1); // 1 2
  l.insertBack(3);  // 1 2 3
  l.insertFront(0); // 0 1 2 3
  l.insertBack(4);  // 0 1 2 3 4

  l.reverse();
  REQUIRE(toString(l) == "< 4 3 2 1 0 >");
  REQUIRE(toVector(l) == std::vector<int>({4, 3, 2, 1, 0}));

  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("Reverse: reversing after split doesn't cross-link lists",
          "[custom][part=2][reverse][valgrind]") {
  auto a = makeList({1, 2, 3, 4, 5, 6});
  auto b = a.split(2);              // a: 1 2, b: 3 4 5 6

  a.reverse();                       // 2 1
  b.reverse();                       // 6 5 4 3

  REQUIRE(toString(a) == "< 2 1 >");
  REQUIRE(toString(b) == "< 6 5 4 3 >");

  requireNoCycleAndCountMatchesSize(a);
  requireNoCycleAndCountMatchesSize(b);

  // extra sanity: mutate one list and ensure the other unaffected
  a.insertBack(99);
  REQUIRE(toString(a) == "< 2 1 99 >");
  REQUIRE(toString(b) == "< 6 5 4 3 >");
}

// ---------------- additional reverseNth() tests (no new includes, no duplicate helpers) ----------------

TEST_CASE("ReverseNth: empty list is a no-op for any n",
          "[custom][part=2][reverseNth][valgrind]") {
  List<int> l;
  l.reverseNth(3);
  REQUIRE(toString(l) == "< >");
  REQUIRE(l.begin() == l.end());

  l.reverseNth(1);
  REQUIRE(toString(l) == "< >");
  REQUIRE(l.begin() == l.end());
}

TEST_CASE("ReverseNth: size 1 unchanged for n=1 and n>1",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({42});
  l.reverseNth(1);
  REQUIRE(toString(l) == "< 42 >");

  l.reverseNth(5);
  REQUIRE(toString(l) == "< 42 >");

  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: n == length behaves like reverse",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5});
  l.reverseNth(5);
  REQUIRE(toString(l) == "< 5 4 3 2 1 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: n == length-1 reverses first block, leaves tail element",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5});
  l.reverseNth(4);
  REQUIRE(toString(l) == "< 4 3 2 1 5 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: odd length with n=2 leaves last single element",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5});
  l.reverseNth(2);
  REQUIRE(toString(l) == "< 2 1 4 3 5 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: n=3 with leftover block size 2 is also reversed",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8});
  l.reverseNth(3);
  REQUIRE(toString(l) == "< 3 2 1 6 5 4 8 7 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: n=3 exact blocks (length multiple of n)",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8, 9});
  l.reverseNth(3);
  REQUIRE(toString(l) == "< 3 2 1 6 5 4 9 8 7 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: duplicates + negatives stay intact, blockwise reversed",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({0, -1, -1, 2, 2, 7, 7});
  l.reverseNth(3);
  // blocks: [0 -1 -1] -> [-1 -1 0], [2 2 7] -> [7 2 2], [7] -> [7]
  REQUIRE(toString(l) == "< -1 -1 0 7 2 2 7 >");
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: applying reverseNth twice returns original (even with leftover)",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8});
  auto orig = toVector(l);

  l.reverseNth(3);
  l.reverseNth(3);

  REQUIRE(toVector(l) == orig);
  requireNoCycleAndCountMatchesSize(l);
}

TEST_CASE("ReverseNth: composition sanity (waterfall then reverseNth)",
          "[custom][part=2][reverseNth][valgrind]") {
  auto l = makeList({1, 2, 3, 4, 5, 6, 7, 8});
  l.waterfall();           // < 1 3 5 7 2 6 4 8 >
  l.reverseNth(4);          // reverse blocks of 4: < 7 5 3 1 8 4 6 2 >

  REQUIRE(toString(l) == "< 7 5 3 1 8 4 6 2 >");
  requireNoCycleAndCountMatchesSize(l);
}
