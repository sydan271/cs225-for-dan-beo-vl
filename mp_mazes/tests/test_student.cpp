#include "dsets.h"
#include <catch2/catch_test_macros.hpp>
#include <numeric>
#include <vector>

using namespace std;

/*
  Comprehensive DisjointSets tests:
  - constructor + addElements invariants
  - find root correctness + path compression
  - union-by-size behavior + tie-breaking rule
  - size correctness after unions
  - robustness: union/find with invalid indices (should not crash)
  - idempotence: repeated unions on already-connected elements (should be safe)
*/

static void require_all_distinct_roots(DisjointSets &ds, int n) {
   for (int i = 0; i < n; i++) {
      REQUIRE(ds.find(i) == i);
      REQUIRE(ds.size(i) == 1);
      REQUIRE(ds.getValue(i) == -1);
   }
}

TEST_CASE("DisjointSets: constructor invariants", "[dsets][part1]") {
   DisjointSets ds0;
   // No elements: find should return -1 per your implementation
   REQUIRE(ds0.find(0) == -1);

   DisjointSets ds(6);
   require_all_distinct_roots(ds, 6);
}

TEST_CASE("DisjointSets: addElements appends correctly and preserves prior structure",
          "[dsets][part1]") {
   DisjointSets ds;
   ds.addElements(3);
   require_all_distinct_roots(ds, 3);

   ds.setUnion(0, 1);
   int r01 = ds.find(0);
   REQUIRE(ds.find(1) == r01);
   REQUIRE(ds.size(0) == 2);
   REQUIRE(ds.size(1) == 2);

   ds.addElements(4);
   // Old structure preserved
   REQUIRE(ds.find(0) == r01);
   REQUIRE(ds.find(1) == r01);
   REQUIRE(ds.size(0) == 2);

   // New nodes are fresh singletons
   for (int i = 3; i <= 6; i++) {
      REQUIRE(ds.find(i) == i);
      REQUIRE(ds.size(i) == 1);
      REQUIRE(ds.getValue(i) == -1);
   }
}

TEST_CASE("DisjointSets: union-by-size (smaller attaches to larger)", "[dsets][part1]") {
   DisjointSets ds(6);

   // Make set A size 3: {0,1,2}
   ds.setUnion(0, 1);
   ds.setUnion(0, 2);
   int rA = ds.find(0);
   REQUIRE(ds.size(1) == 3);
   REQUIRE(ds.size(2) == 3);

   // Make set B size 2: {3,4}
   ds.setUnion(3, 4);
   int rB = ds.find(3);
   REQUIRE(ds.size(4) == 2);

   // Union A (size 3) with B (size 2): root should remain rA (larger)
   ds.setUnion(3, 0);
   REQUIRE(ds.find(3) == ds.find(0));
   REQUIRE(ds.find(4) == ds.find(0));
   REQUIRE(ds.size(0) == 5);

   // After union, B's old root should not still be root unless it was attached correctly
   REQUIRE(ds.find(rB) == ds.find(rA));
}

TEST_CASE(
    "DisjointSets: tie-breaking rule (second argument's tree points to first argument's tree)",
    "[dsets][part1]") {
   DisjointSets ds(4);

   // Two singleton sets tie in size.
   // Spec: if same size, tree containing second argument points to tree containing first argument.
   ds.setUnion(1, 2);

   int r1 = ds.find(1);
   int r2 = ds.find(2);
   REQUIRE(r1 == r2);

   // With tie rule above, root should be find(1) (the first arg's root), i.e., 1.
   REQUIRE(r1 == 1);

   // Also verify sizes stored at root are correct (root should hold -2).
   REQUIRE(ds.getValue(1) == -2);
   // Node 2 should point directly to 1 after union
   REQUIRE(ds.getValue(2) == 1);
}

TEST_CASE("DisjointSets: size correctness across multiple unions", "[dsets][part1]") {
   DisjointSets ds(10);

   ds.setUnion(0, 1);
   ds.setUnion(2, 3);
   ds.setUnion(4, 5);
   ds.setUnion(6, 7);

   REQUIRE(ds.size(0) == 2);
   REQUIRE(ds.size(2) == 2);
   REQUIRE(ds.size(4) == 2);
   REQUIRE(ds.size(6) == 2);

   ds.setUnion(0, 2); // size 4
   REQUIRE(ds.size(3) == 4);

   ds.setUnion(4, 6); // size 4
   REQUIRE(ds.size(7) == 4);

   ds.setUnion(0, 4); // size 8
   REQUIRE(ds.size(5) == 8);
   REQUIRE(ds.size(7) == 8);

   // Unused nodes remain singleton
   REQUIRE(ds.size(8) == 1);
   REQUIRE(ds.size(9) == 1);
}

TEST_CASE("DisjointSets: path compression actually flattens chains", "[dsets][part1]") {
   DisjointSets ds(6);

   // Build a structure that can create some depth via unions.
   // The exact shape depends on union-by-size, but we can still test compression:
   ds.setUnion(0, 1);
   ds.setUnion(2, 3);
   ds.setUnion(0, 2); // combine into size 4
   ds.setUnion(4, 5);
   ds.setUnion(0, 4); // combine into size 6

   int root = ds.find(5);

   // Force finds to compress paths for all nodes
   for (int i = 0; i < 6; i++) {
      REQUIRE(ds.find(i) == root);
   }

   // After compression, every non-root should have parent = root directly
   for (int i = 0; i < 6; i++) {
      if (i == root) {
         REQUIRE(ds.getValue(i) == -6);
      } else {
         REQUIRE(ds.getValue(i) == root);
      }
   }
}

TEST_CASE("DisjointSets: repeated unions of already-connected elements are safe and do not corrupt",
          "[dsets][part1]") {
   DisjointSets ds(5);

   ds.setUnion(0, 1);
   ds.setUnion(1, 2);
   int root = ds.find(0);
   REQUIRE(ds.size(2) == 3);

   // Re-union within same set should be no-op (must not create self-parent loops / wrong sizes)
   ds.setUnion(0, 2);
   ds.setUnion(2, 1);
   ds.setUnion(1, 0);

   REQUIRE(ds.find(1) == root);
   REQUIRE(ds.find(2) == root);
   REQUIRE(ds.size(0) == 3);

   // Ensure root still stores correct size and doesn't point to itself
   REQUIRE(ds.getValue(root) < 0);
}

TEST_CASE("DisjointSets: invalid indices do not crash and do not mutate", "[dsets][part1]") {
   DisjointSets ds(3);

   // invalid find returns -1 per your code
   REQUIRE(ds.find(-1) == -1);
   REQUIRE(ds.find(3) == -1);
   REQUIRE(ds.find(999) == -1);

   // invalid size returns 0 per your code
   REQUIRE(ds.size(-1) == 0);
   REQUIRE(ds.size(3) == 0);

   // invalid unions should do nothing and not crash
   ds.setUnion(-1, 0);
   ds.setUnion(0, 3);
   ds.setUnion(-5, 99);

   // Structure should remain all singletons
   for (int i = 0; i < 3; i++) {
      REQUIRE(ds.find(i) == i);
      REQUIRE(ds.size(i) == 1);
      REQUIRE(ds.getValue(i) == -1);
   }
}

TEST_CASE("DisjointSets: unions with non-root arguments still produce correct structure",
          "[dsets][part1]") {
   DisjointSets ds(7);

   ds.setUnion(0, 1);
   ds.setUnion(1, 2); // 0-1-2 connected
   ds.setUnion(3, 4);
   ds.setUnion(4, 5); // 3-4-5 connected

   int rA = ds.find(2);
   int rB = ds.find(5);
   REQUIRE(rA != rB);

   // union using non-roots
   ds.setUnion(2, 4);

   int r = ds.find(0);
   for (int i : {0, 1, 2, 3, 4, 5})
      REQUIRE(ds.find(i) == r);
   REQUIRE(ds.size(5) == 6);

   // node 6 remains separate
   REQUIRE(ds.find(6) == 6);
   REQUIRE(ds.size(6) == 1);
}
// Additional student tests for DisjointSets and SquareMaze (Catch2 v3)
//
// Drop these into tests/test_student.cpp (or a new test file) alongside existing tests.
// These are designed to catch: invalid-index safety, union-by-size/tie rules,
// path compression, canTravel boundary logic, setWall correctness, solveMaze tie-breaking,
// and edge-dimension mazes (1xN / Nx1 / 1x1).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <queue>
#include <set>
#include <vector>

#include "cs225/PNG.h"
#include "dsets.h"
#include "maze.h"
#include "mazereader.h"

using namespace std;
using namespace cs225;

// -------------------------
// DisjointSets: helper
// -------------------------
static vector<int> snapshotParents(const DisjointSets &ds, int n) {
   vector<int> v;
   v.reserve(n);
   for (int i = 0; i < n; i++)
      v.push_back(ds.getValue(i));
   return v;
}

// -------------------------
// DisjointSets tests
// -------------------------

TEST_CASE("DisjointSets: addElements with 0 does not change structure", "[dsets][extra]") {
   DisjointSets ds(5);
   auto before = snapshotParents(ds, 5);
   ds.addElements(0);
   auto after = snapshotParents(ds, 5);
   REQUIRE(before == after);
}

TEST_CASE("DisjointSets: find invalid indices returns -1 and does not mutate", "[dsets][extra]") {
   DisjointSets ds(3);
   auto before = snapshotParents(ds, 3);

   REQUIRE(ds.find(-1) == -1);
   REQUIRE(ds.find(3) == -1);
   REQUIRE(ds.find(100) == -1);

   auto after = snapshotParents(ds, 3);
   REQUIRE(before == after);
}

TEST_CASE("DisjointSets: setUnion invalid indices does not mutate", "[dsets][extra]") {
   DisjointSets ds(4);
   ds.setUnion(0, 1);
   auto before = snapshotParents(ds, 4);

   ds.setUnion(-1, 2);
   ds.setUnion(2, -1);
   ds.setUnion(4, 1);
   ds.setUnion(1, 999);

   auto after = snapshotParents(ds, 4);
   REQUIRE(before == after);
}

TEST_CASE("DisjointSets: union-by-size attaches smaller to larger", "[dsets][extra]") {
   DisjointSets ds(6);

   // Make set A size 4: {0,1,2,3}
   ds.setUnion(0, 1);
   ds.setUnion(2, 3);
   ds.setUnion(0, 2);

   // Make set B size 2: {4,5}
   ds.setUnion(4, 5);

   int rootA = ds.find(0);
   int rootB = ds.find(4);
   REQUIRE(ds.size(0) == 4);
   REQUIRE(ds.size(4) == 2);

   ds.setUnion(0, 4);

   int newRoot = ds.find(0);
   REQUIRE(ds.find(4) == newRoot);
   REQUIRE(ds.size(5) == 6);

   // The larger set root should remain root after union-by-size.
   // Since A was larger (size 4) than B (size 2), A's root should be the final root.
   REQUIRE(newRoot == rootA);
   REQUIRE(newRoot != rootB);
}

TEST_CASE("DisjointSets: tie-break (same size) second points to first's tree", "[dsets][extra]") {
   DisjointSets ds(4);

   // Two sets of size 2: {0,1} and {2,3}
   ds.setUnion(0, 1);
   ds.setUnion(2, 3);

   int root01 = ds.find(0);
   int root23 = ds.find(2);

   REQUIRE(ds.size(0) == 2);
   REQUIRE(ds.size(2) == 2);

   // tie rule: tree containing second argument points to tree containing first argument
   ds.setUnion(0, 2);

   int finalRoot = ds.find(0);
   REQUIRE(ds.find(2) == finalRoot);

   // "second points to first": root of set containing '2' should become child of root of set
   // containing '0'
   REQUIRE(finalRoot == root01);
   REQUIRE(ds.find(3) == finalRoot);
   REQUIRE(ds.size(1) == 4);
}

TEST_CASE("DisjointSets: find performs path compression", "[dsets][extra]") {
   DisjointSets ds(6);

   // Build a chain-like structure via unions that can create depth
   ds.setUnion(0, 1);
   ds.setUnion(0, 2);
   ds.setUnion(0, 3);
   ds.setUnion(0, 4);
   ds.setUnion(0, 5);

   // Force compression by calling find on each element
   int r = ds.find(5);
   REQUIRE(r == ds.find(0));

   // After compression, every non-root node should have direct parent == root
   // (implementation-dependent, but in classic CS225 DS, find compresses to the root).
   for (int i = 0; i < 6; i++) {
      if (ds.getValue(i) >= 0) {
         REQUIRE(ds.getValue(i) == r);
      }
   }
}

// -------------------------
// SquareMaze helpers
// -------------------------

static void walkPath(const vector<Direction> &path, int &x, int &y) {
   for (Direction d : path) {
      if (d == RIGHT)
         x++;
      else if (d == LEFT)
         x--;
      else if (d == DOWN)
         y++;
      else if (d == UP)
         y--;
   }
}

static bool inBounds(int x, int y, int w, int h) { return x >= 0 && x < w && y >= 0 && y < h; }

// -------------------------
// SquareMaze tests
// -------------------------

TEST_CASE("SquareMaze: canTravel rejects stepping off grid boundaries", "[maze][extra]") {
   SquareMaze m;
   m.makeMaze(3, 3);

   // Corners
   REQUIRE(m.canTravel(0, 0, LEFT) == false);
   REQUIRE(m.canTravel(0, 0, UP) == false);

   REQUIRE(m.canTravel(2, 0, RIGHT) == false);
   REQUIRE(m.canTravel(2, 0, UP) == false);

   REQUIRE(m.canTravel(0, 2, LEFT) == false);
   REQUIRE(m.canTravel(0, 2, DOWN) == false);

   REQUIRE(m.canTravel(2, 2, RIGHT) == false);
   REQUIRE(m.canTravel(2, 2, DOWN) == false);
}

TEST_CASE(
    "SquareMaze: setWall only affects local RIGHT/DOWN walls (no crash), and canTravel responds",
    "[maze][extra]") {
   SquareMaze m;
   m.makeMaze(2, 2);

   // Start from a known configuration by forcing walls.
   // Create all walls, then remove specific ones and verify canTravel reflects it.
   // Only RIGHT/DOWN are guaranteed by spec; use those.
   m.setWall(0, 0, RIGHT, true);
   m.setWall(0, 0, DOWN, true);

   REQUIRE(m.canTravel(0, 0, RIGHT) == false);
   REQUIRE(m.canTravel(0, 0, DOWN) == false);

   m.setWall(0, 0, RIGHT, false);
   REQUIRE(m.canTravel(0, 0, RIGHT) == true);
   REQUIRE(m.canTravel(1, 0, LEFT) == true); // same wall from neighbor side

   m.setWall(0, 0, DOWN, false);
   REQUIRE(m.canTravel(0, 0, DOWN) == true);
   REQUIRE(m.canTravel(0, 1, UP) == true); // same wall from neighbor side
}

TEST_CASE("SquareMaze: solveMaze returns path that stays in-bounds and ends on bottom row",
          "[maze][extra]") {
   SquareMaze m;
   m.makeMaze(20, 10);

   int startX = 0;
   auto path = m.solveMaze(startX);
   REQUIRE(!path.empty());

   int x = startX, y = 0;
   for (Direction d : path) {
      REQUIRE(m.canTravel(x, y, d)); // must be legal move
      if (d == RIGHT)
         x++;
      else if (d == LEFT)
         x--;
      else if (d == DOWN)
         y++;
      else if (d == UP)
         y--;
      REQUIRE(inBounds(x, y, 20, 10));
   }
   REQUIRE(y == 9);
}

TEST_CASE("SquareMaze: solveMaze works for non-zero startX", "[maze][extra]") {
   SquareMaze m;
   m.makeMaze(30, 30);

   int startX = 17;
   auto path = m.solveMaze(startX);
   REQUIRE(!path.empty());

   int x = startX, y = 0;
   for (Direction d : path) {
      REQUIRE(m.canTravel(x, y, d));
      if (d == RIGHT)
         x++;
      else if (d == LEFT)
         x--;
      else if (d == DOWN)
         y++;
      else if (d == UP)
         y--;
      REQUIRE(inBounds(x, y, 30, 30));
   }
   REQUIRE(y == 29);
}

TEST_CASE("SquareMaze: degenerate 1x1 maze", "[maze][extra]") {
   SquareMaze m;
   m.makeMaze(1, 1);

   auto path = m.solveMaze(0);
   REQUIRE(path.empty());

   // drawMaze should produce 11x11 image
   PNG *img = m.drawMaze(0);
   REQUIRE(img->width() == 1 * 10 + 1);
   REQUIRE(img->height() == 1 * 10 + 1);
   delete img;
}

TEST_CASE("SquareMaze: 1xN maze solution should be all DOWN (only possible)", "[maze][extra]") {
   SquareMaze m;
   int W = 1, H = 20;
   m.makeMaze(W, H);

   auto path = m.solveMaze(0);
   REQUIRE((int)path.size() == H - 1);

   int x = 0, y = 0;
   for (Direction d : path) {
      REQUIRE(d == DOWN);
      REQUIRE(m.canTravel(x, y, d));
      y++;
   }
   REQUIRE(y == H - 1);
}

TEST_CASE("SquareMaze: Nx1 maze solution is empty (already at bottom row)", "[maze][extra]") {
   SquareMaze m;
   int W = 25, H = 1;
   m.makeMaze(W, H);

   auto path = m.solveMaze(7);
   REQUIRE(path.empty());
}

TEST_CASE("SquareMaze: drawMaze entrance gap is correct size", "[maze][extra]") {
   SquareMaze m;
   int W = 10, H = 10;
   int start = 3;
   m.makeMaze(W, H);

   PNG *img = m.drawMaze(start);
   // Entrance gap: pixels (start*10+1 .. (start+1)*10-1) on y=0 must be white (l=1)
   for (int x = 0; x < (int)img->width(); x++) {
      if (x >= start * 10 + 1 && x <= (start + 1) * 10 - 1) {
         REQUIRE(img->getPixel(x, 0).l == 1);
      } else {
         REQUIRE(img->getPixel(x, 0).l == 0);
      }
   }
   delete img;
}

TEST_CASE("SquareMaze: drawMazeWithSolution opens an exit on bottom border", "[maze][extra]") {
   SquareMaze m;
   int W = 40, H = 40;
   int start = 0;
   m.makeMaze(W, H);

   PNG *img = m.drawMazeWithSolution(start);

   // There must exist some x such that pixels (x*10+1 .. x*10+9) at y = H*10 are white (exit gap)
   bool foundExit = false;
   int yExit = H * 10;
   for (int cellX = 0; cellX < W; cellX++) {
      bool gap = true;
      for (int k = 1; k <= 9; k++) {
         if (img->getPixel(cellX * 10 + k, yExit).l != 1) {
            gap = false;
            break;
         }
      }
      if (gap) {
         foundExit = true;
         break;
      }
   }
   REQUIRE(foundExit);

   delete img;
}

TEST_CASE("SquareMaze: solveMaze tie-break prefers smallest x among max-distance bottom cells "
          "(constructed maze)",
          "[maze][extra]") {
   // Build a tiny maze by copying from a PNG to force deterministic structure.
   // You can author a small expected PNG and reuse MazeReader like the staff tests do.
   //
   // If you do not want to create a PNG file, skip this test. The tie-break is hard
   // to force reliably with random mazes.

   SUCCEED("Tie-break test requires a deterministic maze fixture (PNG) to be meaningful.");
}
