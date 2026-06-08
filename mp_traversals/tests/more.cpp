
/**
 * @file more.cpp
 * Extra/stronger tests for MP Traversals Part 1.
 *
 * Goals:
 * - No std::set or std::map (per request)
 * - Catch common landmines:
 *   * delta must be computed from START pixel (not current)
 *   * strict "< tolerance" (not <=)
 *   * neighbor add order: Right, Down, Left, Up
 *   * BFS vs DFS semantics (queue vs stack)
 *   * never visit a point twice (even if added multiple ways)
 *   * iterator begin/end sanity and range-for compatibility
 */

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"

#include "ImageTraversal.h"
#include "Point.h"

using namespace cs225;

/*******************************************************
 * Helpers (no set/map)
 *******************************************************/

static std::vector<Point> collectTraversal(Traversals::ImageTraversal &t) {
   std::vector<Point> out;
   for (auto it = t.begin(); it != t.end(); ++it) {
      out.push_back(*it);
   }
   return out;
}

static std::vector<std::pair<unsigned, unsigned>> toPairs(const std::vector<Point> &pts) {
   std::vector<std::pair<unsigned, unsigned>> v;
   v.reserve(pts.size());
   for (const auto &p : pts) {
      v.emplace_back(p.x, p.y);
   }
   return v;
}

static void requireAllUnique(const std::vector<Point> &pts) {
   auto v = toPairs(pts);
   std::sort(v.begin(), v.end());
   auto it = std::unique(v.begin(), v.end());
   REQUIRE(it == v.end());
}

/*******************************************************
 * BFS/DFS function packs
 *******************************************************/
static Traversals::TraversalFunctions BFS_FNS() {
   return {Traversals::bfs_add, Traversals::bfs_pop, Traversals::bfs_peek};
}
static Traversals::TraversalFunctions DFS_FNS() {
   return {Traversals::dfs_add, Traversals::dfs_pop, Traversals::dfs_peek};
}

/*******************************************************
 * Image builders
 *******************************************************/

static PNG make1x1(double h = 0, double s = 0, double l = 0.5) {
   PNG png(1, 1);
   png.getPixel(0, 0) = HSLAPixel(h, s, l);
   return png;
}

static PNG makeSolid(unsigned w, unsigned h, const HSLAPixel &px) {
   PNG png(w, h);
   for (unsigned x = 0; x < w; x++) {
      for (unsigned y = 0; y < h; y++) {
         png.getPixel(x, y) = px;
      }
   }
   return png;
}

/**
 * Border black, interior white.
 * black = (180,1,0), white = (0,0,1)
 */
static PNG makeBorder(unsigned w, unsigned h) {
   PNG png(w, h);
   HSLAPixel black(180, 1, 0);
   HSLAPixel white(0, 0, 1);

   for (unsigned x = 0; x < w; x++) {
      for (unsigned y = 0; y < h; y++) {
         bool edge = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
         png.getPixel(x, y) = edge ? black : white;
      }
   }
   return png;
}

/**
 * Hue ramp row to trap "delta from current" bug.
 * 1 x n, hues: startHue + step*x
 */
static PNG makeHueRampRow(unsigned n, double startHue, double step) {
   PNG png(n, 1);
   for (unsigned x = 0; x < n; x++) {
      png.getPixel(x, 0) = HSLAPixel(startHue + step * x, 1.0, 0.5);
   }
   return png;
}

/**
 * 3x3 all white except a black blocker at (1,1).
 */
static PNG makeAsymBlocker3x3() {
   PNG png(3, 3);
   HSLAPixel white(0, 0, 1);
   HSLAPixel black(180, 1, 0);
   for (unsigned x = 0; x < 3; x++) {
      for (unsigned y = 0; y < 3; y++) {
         png.getPixel(x, y) = white;
      }
   }
   png.getPixel(1, 1) = black;
   return png;
}

/*******************************************************
 * Tests: Iterator semantics
 *******************************************************/

TEST_CASE("Extra: begin() dereferences to start (BFS/DFS)", "[part=1][extra]") {
   PNG png = makeBorder(4, 4);
   Point start(1, 1);

   Traversals::ImageTraversal bfs(png, start, 0.2, BFS_FNS());
   Traversals::ImageTraversal dfs(png, start, 0.2, DFS_FNS());

   REQUIRE(*bfs.begin() == start);
   REQUIRE(*dfs.begin() == start);
}

TEST_CASE("Extra: range-for visits same count as manual iteration (BFS)", "[part=1][extra]") {
   PNG png = makeBorder(4, 4);
   Point start(1, 1);

   Traversals::ImageTraversal bfs(png, start, 0.2, BFS_FNS());

   unsigned manual = 0;
   for (auto it = bfs.begin(); it != bfs.end(); ++it)
      manual++;

   unsigned ranged = 0;
   for (const Point &p : bfs) {
      (void)p;
      ranged++;
   }

   REQUIRE(manual == ranged);
   REQUIRE(manual == 4);
}

TEST_CASE("Extra: 1x1 image ends after one element, ++end is safe", "[part=1][extra]") {
   PNG png = make1x1();
   Point start(0, 0);

   Traversals::ImageTraversal bfs(png, start, 1.0, BFS_FNS());

   auto it = bfs.begin();
   REQUIRE(it != bfs.end());
   REQUIRE(*it == start);

   ++it;
   REQUIRE(!(it != bfs.end()));

   ++it; // should remain end, not crash
   REQUIRE(!(it != bfs.end()));
}

/*******************************************************
 * Tests: Tolerance strictness and extremes
 *******************************************************/

TEST_CASE("Extra: tolerance 0 includes only start due to strict < (even if identical pixels)",
          "[part=1][extra]") {
   PNG png = makeSolid(2, 1, HSLAPixel(10, 1, 0.5)); // delta==0 between all pixels
   Point start(0, 0);

   Traversals::ImageTraversal bfs(png, start, 0.0, BFS_FNS());
   auto pts = collectTraversal(bfs);

   REQUIRE(pts.size() == 1);
   REQUIRE(pts[0] == start);
}

TEST_CASE("Extra: huge tolerance visits whole image and never duplicates (BFS/DFS)",
          "[part=1][extra][valgrind]") {
   PNG png = makeSolid(5, 4, HSLAPixel(200, 0.7, 0.4));
   Point start(2, 2);

   Traversals::ImageTraversal bfs(png, start, 10.0, BFS_FNS());
   Traversals::ImageTraversal dfs(png, start, 10.0, DFS_FNS());

   auto b = collectTraversal(bfs);
   auto d = collectTraversal(dfs);

   REQUIRE(b.size() == 20);
   REQUIRE(d.size() == 20);
   requireAllUnique(b);
   requireAllUnique(d);
}

/*******************************************************
 * Tests: Delta computed from START (not current)
 *******************************************************/

TEST_CASE("Extra: delta must be computed from start pixel (BFS chain trap)", "[part=1][extra]") {
   PNG png = makeHueRampRow(6, 0, 10);
   Point start(0, 0);

   // With s=1,l=0.5 constant, delta approx = |hueDiff|/360.
   // tolerance 0.07 allows hueDiff < 25.2 => only x=0,1,2 allowed.
   double tolerance = 0.07;

   Traversals::ImageTraversal bfs(png, start, tolerance, BFS_FNS());
   auto pts = collectTraversal(bfs);

   REQUIRE(pts.size() == 3);
   REQUIRE(pts[0] == Point(0, 0));
   REQUIRE(pts[1] == Point(1, 0));
   REQUIRE(pts[2] == Point(2, 0));
   requireAllUnique(pts);
}

TEST_CASE("Extra: delta must be computed from start pixel (DFS chain trap)", "[part=1][extra]") {
   PNG png = makeHueRampRow(6, 0, 10);
   Point start(0, 0);
   double tolerance = 0.07;

   Traversals::ImageTraversal dfs(png, start, tolerance, DFS_FNS());
   auto pts = collectTraversal(dfs);

   REQUIRE(pts.size() == 3);
   REQUIRE(pts[0] == Point(0, 0));
   REQUIRE(pts[1] == Point(1, 0));
   REQUIRE(pts[2] == Point(2, 0));
   requireAllUnique(pts);
}

/*******************************************************
 * Tests: Neighbor add order and BFS/DFS semantics
 *******************************************************/

TEST_CASE("Extra: BFS first layer from center is Right,Down,Left,Up", "[part=1][extra]") {
   PNG png = makeSolid(3, 3, HSLAPixel(0, 0, 1));
   Point start(1, 1);

   Traversals::ImageTraversal bfs(png, start, 1.0, BFS_FNS());
   auto pts = collectTraversal(bfs);

   REQUIRE(pts.size() == 9);
   REQUIRE(pts[0] == Point(1, 1));
   REQUIRE(pts[1] == Point(2, 1)); // Right
   REQUIRE(pts[2] == Point(1, 2)); // Down
   REQUIRE(pts[3] == Point(0, 1)); // Left
   REQUIRE(pts[4] == Point(1, 0)); // Up
   requireAllUnique(pts);
}

TEST_CASE("Extra: DFS visits Up first from center due to stack + add order R,D,L,U",
          "[part=1][extra]") {
   PNG png = makeSolid(3, 3, HSLAPixel(0, 0, 1));
   Point start(1, 1);

   Traversals::ImageTraversal dfs(png, start, 1.0, DFS_FNS());
   auto pts = collectTraversal(dfs);

   REQUIRE(pts.size() == 9);
   REQUIRE(pts[0] == Point(1, 1));
   REQUIRE(pts[1] == Point(1, 0)); // Up is last pushed => first popped
   requireAllUnique(pts);
}

/*******************************************************
 * Tests: Asymmetric shape (BFS vs DFS should diverge)
 *******************************************************/

TEST_CASE("Extra: BFS vs DFS produce different order on asymmetric reachable region",
          "[part=1][extra]") {
   PNG png = makeAsymBlocker3x3();
   Point start(0, 1);
   double tol = 0.5; // enough for whites, excludes black (max delta)

   Traversals::ImageTraversal bfs(png, start, tol, BFS_FNS());
   Traversals::ImageTraversal dfs(png, start, tol, DFS_FNS());

   auto b = collectTraversal(bfs);
   auto d = collectTraversal(dfs);

   REQUIRE(b.size() == d.size());
   requireAllUnique(b);
   requireAllUnique(d);

   // They should not have identical visit order.
   REQUIRE(b != d);
}

/*******************************************************
 * Tests: Border-only vs interior-only reachability and bounds safety
 *******************************************************/

TEST_CASE("Extra: border image from interior visits only interior pixels", "[part=1][extra]") {
   PNG png = makeBorder(6, 6);
   Point start(2, 2);

   Traversals::ImageTraversal bfs(png, start, 0.2, BFS_FNS());
   auto pts = collectTraversal(bfs);

   // interior = (w-2)*(h-2) = 16
   REQUIRE(pts.size() == 16);
   requireAllUnique(pts);

   for (const auto &p : pts) {
      REQUIRE(p.x > 0);
      REQUIRE(p.y > 0);
      REQUIRE(p.x < 5);
      REQUIRE(p.y < 5);
   }
}

TEST_CASE("Extra: border image from corner visits only border pixels", "[part=1][extra]") {
   PNG png = makeBorder(6, 6);
   Point start(0, 0);

   Traversals::ImageTraversal dfs(png, start, 0.2, DFS_FNS());
   auto pts = collectTraversal(dfs);

   // border count = 2w + 2h - 4 = 20
   REQUIRE(pts.size() == 20);
   requireAllUnique(pts);

   for (const auto &p : pts) {
      bool edge = (p.x == 0 || p.y == 0 || p.x == 5 || p.y == 5);
      REQUIRE(edge);
   }
}

/*******************************************************
 * Tests: Iterator copy/assignment sanity
 *******************************************************/

TEST_CASE("Extra: copying iterator preserves position (basic)", "[part=1][extra]") {
   PNG png = makeSolid(3, 3, HSLAPixel(0, 0, 1));
   Point start(1, 1);

   Traversals::ImageTraversal bfs(png, start, 1.0, BFS_FNS());

   auto it1 = bfs.begin();
   auto it2 = it1; // copy

   REQUIRE(*it1 == start);
   REQUIRE(*it2 == start);

   ++it1;
   // it2 should still be at start
   REQUIRE(*it2 == start);
}
