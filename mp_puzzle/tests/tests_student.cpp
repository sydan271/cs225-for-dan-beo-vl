// tests/more.cpp
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "puzzle.h"

using std::array;
using std::set;
using std::string;
using std::vector;

static array<char, 16> allZeroArr() {
   array<char, 16> z{};
   z.fill(0);
   return z;
}

static array<char, 16> solvedArr() {
   return array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
}

static bool isPermutation0to15(const array<char, 16> &a) {
   array<int, 16> seen{};
   seen.fill(0);
   for (char c : a) {
      int v = static_cast<unsigned char>(c);
      if (v < 0 || v > 15) return false;
      if (++seen[v] != 1) return false;
   }
   return true;
}

// Two states are neighbors if they differ in exactly two positions and swap a 0 with some tile.
static bool isOneMoveApart(const PuzzleState &a, const PuzzleState &b) {
   array<char, 16> A = a.asArray();
   array<char, 16> B = b.asArray();

   int diffCount = 0;
   int idx0A = -1, idx0B = -1;
   for (int i = 0; i < 16; i++) {
      if (A[i] != B[i]) diffCount++;
      if (A[i] == 0) idx0A = i;
      if (B[i] == 0) idx0B = i;
   }
   if (diffCount != 2) return false;
   if (idx0A == -1 || idx0B == -1) return false;

   // They must swap 0 with one tile:
   // A[idx0A] == 0, B[idx0B] == 0; the other differing positions are idx0A and idx0B.
   if (A[idx0A] != 0 || B[idx0B] != 0) return false;
   if (B[idx0A] == 0) return false;
   if (A[idx0B] == 0) return false;
   if (B[idx0A] != A[idx0B]) return false;

   // Also ensure both are valid permutations
   return isPermutation0to15(A) && isPermutation0to15(B);
}

static void
checkPathValid(const vector<PuzzleState> &path, const PuzzleState &start, const PuzzleState &goal) {
   REQUIRE(!path.empty());
   REQUIRE(path.front() == start);
   REQUIRE(path.back() == goal);

   for (size_t i = 1; i < path.size(); i++) {
      REQUIRE(isOneMoveApart(path[i - 1], path[i]));
   }
}

static PuzzleState applyMoves(PuzzleState s, const vector<PuzzleState::Direction> &moves) {
   for (auto d : moves) {
      s = s.getNeighbor(d);
   }
   return s;
}

/* =========================
 *  Core representation tests
 * ========================= */

TEST_CASE("Default constructor is the solved puzzle", "[weight=1][part=1]") {
   PuzzleState def;
   PuzzleState solved(solvedArr());
   REQUIRE(def == solved);
   REQUIRE(def.asArray() == solvedArr());
}

TEST_CASE("Custom constructor + asArray round-trip", "[weight=2][part=1]") {
   array<char, 16> state{0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
   REQUIRE(isPermutation0to15(state));

   PuzzleState p(state);
   REQUIRE(p.asArray() == state);
}

TEST_CASE("operator== and operator!= are consistent", "[weight=2][part=1]") {
   array<char, 16> state{0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
   PuzzleState a(state);
   PuzzleState b(state);
   REQUIRE(a == b);
   REQUIRE(!(a != b));

   PuzzleState c(array<char, 16>{0, 8, 1, 9, 2, 3, 10, 11, 4, 12, 5, 13, 6, 14, 7, 15});
   REQUIRE(a != c);
   REQUIRE(!(a == c));
}

TEST_CASE("operator< implements lexicographical order (first differing index)",
          "[weight=2][part=1]") {
   // Differ at index 0
   PuzzleState a(array<char, 16>{0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 1});
   PuzzleState b(array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0});
   REQUIRE(a < b);
   REQUIRE(!(b < a));

   // Equal states: neither less-than holds
   PuzzleState c(solvedArr());
   PuzzleState d(solvedArr());
   REQUIRE(!(c < d));
   REQUIRE(!(d < c));

   // Differ first at a later index (index 6 here)
   PuzzleState e(array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0});
   PuzzleState f(array<char, 16>{1, 2, 3, 4, 5, 6, 8, 7, 9, 10, 11, 12, 13, 14, 15, 0});
   // At index 6: 7 < 8, so e < f
   REQUIRE(e < f);
   REQUIRE(!(f < e));
}

/* =========================
 *  Neighbor generation tests
 * ========================= */

TEST_CASE("getNeighbor returns all-zeros state on invalid moves", "[weight=2][part=1]") {
   PuzzleState solved;
   PuzzleState invalid(allZeroArr());

   // solved has blank at bottom-right:
   // UP is invalid (blank would go further down)
   // LEFT is invalid (blank would go further right)
   REQUIRE(solved.getNeighbor(PuzzleState::Direction::UP) == invalid);
   REQUIRE(solved.getNeighbor(PuzzleState::Direction::LEFT) == invalid);
}

TEST_CASE("getNeighbor returns correct states near edges/corners and does not mutate original",
          "[weight=3][part=1]") {
   PuzzleState solved;
   PuzzleState copy = solved;

   // From solved, valid moves are DOWN (blank up) and RIGHT (blank left)
   PuzzleState down(array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 13, 14, 15, 12});
   PuzzleState right(array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15});

   REQUIRE(solved.getNeighbor(PuzzleState::Direction::DOWN) == down);
   REQUIRE(solved.getNeighbor(PuzzleState::Direction::RIGHT) == right);

   // Original should be unchanged
   REQUIRE(solved == copy);
}

TEST_CASE("getNeighbors returns exactly the valid neighbors and no invalid all-zero states",
          "[weight=3][part=1]") {
   PuzzleState solved;
   vector<PuzzleState> ns = solved.getNeighbors();

   // Blank at corner => 2 neighbors
   REQUIRE(ns.size() == 2);

   PuzzleState invalid(allZeroArr());
   REQUIRE(std::find(ns.begin(), ns.end(), invalid) == ns.end());

   // And each neighbor should be one move away from solved
   for (const auto &n : ns) {
      REQUIRE(isOneMoveApart(solved, n));
   }
}

TEST_CASE("getNeighbors returns 4 neighbors from an interior blank and has no duplicates",
          "[weight=3][part=1]") {
   PuzzleState interior(array<char, 16>{1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
   vector<PuzzleState> ns = interior.getNeighbors();
   REQUIRE(ns.size() == 4);

   // Uniqueness check
   set<array<char, 16>> seen;
   for (const auto &n : ns) {
      REQUIRE(isOneMoveApart(interior, n));
      seen.insert(n.asArray());
   }
   REQUIRE(seen.size() == 4);
}

/* =========================
 *  Manhattan distance tests
 * ========================= */

TEST_CASE("manhattanDistance is 0 iff states are equal", "[weight=2][part=1]") {
   PuzzleState a;
   REQUIRE(a.manhattanDistance(a) == 0);

   PuzzleState b(array<char, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15});
   REQUIRE(b != a);
   REQUIRE(b.manhattanDistance(a) > 0);
}

TEST_CASE("manhattanDistance symmetry: dist(A,B) == dist(B,A) for valid states",
          "[weight=2][part=1]") {
   PuzzleState a(array<char, 16>{6, 7, 15, 13, 12, 10, 4, 0, 3, 5, 14, 1, 8, 9, 2, 11});
   PuzzleState b(array<char, 16>{0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15});
   REQUIRE(a.manhattanDistance(b) == b.manhattanDistance(a));
}

TEST_CASE("manhattanDistance changes by <= 1 per moved tile (blank ignored)",
          "[weight=2][part=1]") {
   // Move around from solved; each move swaps blank with exactly one tile,
   // so the heuristic should change by at most 1 in either direction.
   PuzzleState goal;
   PuzzleState cur = goal;

   vector<PuzzleState::Direction> moves{PuzzleState::Direction::RIGHT, PuzzleState::Direction::DOWN,
                                        PuzzleState::Direction::DOWN, PuzzleState::Direction::LEFT,
                                        PuzzleState::Direction::UP};

   int prev = cur.manhattanDistance(goal);
   for (auto d : moves) {
      PuzzleState next = cur.getNeighbor(d);
      // Should be valid here (we chose moves that stay in-bounds)
      REQUIRE(next.asArray() != allZeroArr());

      int now = next.manhattanDistance(goal);
      int delta = now - prev;
      REQUIRE((delta >= -1 && delta <= 1));
      cur = next;
      prev = now;
   }
}

/* =========================
 *  Solver tests (BFS / A*)
 * ========================= */

TEST_CASE("solveBFS: start==goal returns single-state path", "[weight=2][part=2]") {
   PuzzleState start;
   PuzzleState goal;
   size_t iters = 0;

   vector<PuzzleState> path = solveBFS(start, goal, &iters);
   REQUIRE(path.size() == 1);
   REQUIRE(path.front() == start);
   REQUIRE(path.back() == goal);

   // iterations is pop-count; should include start/goal (same state => 1)
   REQUIRE(iters >= 1);
}

TEST_CASE("solveAstar: start==goal returns single-state path", "[weight=2][part=2]") {
   PuzzleState start;
   PuzzleState goal;
   size_t iters = 0;

   vector<PuzzleState> path = solveAstar(start, goal, &iters);
   REQUIRE(path.size() == 1);
   REQUIRE(path.front() == start);
   REQUIRE(path.back() == goal);
   REQUIRE(iters >= 1);
}

TEST_CASE("solveBFS finds shortest path for 1 move away", "[weight=3][part=2]") {
   PuzzleState goal;
   PuzzleState start = goal.getNeighbor(PuzzleState::Direction::RIGHT); // blank moves left
   REQUIRE(start.asArray() != allZeroArr());

   size_t iters = 0;
   vector<PuzzleState> path = solveBFS(start, goal, &iters);

   REQUIRE(path.size() == 2);
   checkPathValid(path, start, goal);
   REQUIRE(iters >= path.size());
   REQUIRE(iters <= 100); // should be tiny for 1-move case
}

TEST_CASE("solveAstar finds shortest path for 1 move away", "[weight=3][part=2]") {
   PuzzleState goal;
   PuzzleState start = goal.getNeighbor(PuzzleState::Direction::RIGHT);

   size_t iters = 0;
   vector<PuzzleState> path = solveAstar(start, goal, &iters);

   REQUIRE(path.size() == 2);
   checkPathValid(path, start, goal);
   REQUIRE(iters >= path.size());
   REQUIRE(iters <= 100);
}

TEST_CASE("solveBFS finds shortest path for 2 moves away", "[weight=4][part=2]") {
   PuzzleState goal;
   PuzzleState start =
       applyMoves(goal, {PuzzleState::Direction::RIGHT, PuzzleState::Direction::DOWN});
   REQUIRE(start.asArray() != allZeroArr());

   size_t iters = 0;
   vector<PuzzleState> path = solveBFS(start, goal, &iters);

   REQUIRE(path.size() == 3);
   checkPathValid(path, start, goal);
   REQUIRE(iters >= path.size());
   REQUIRE(iters <= 500);
}

TEST_CASE("solveAstar matches BFS path length for small scrambles", "[weight=4][part=2]") {
   PuzzleState goal;
   // 6-move scramble that stays in-bounds
   vector<PuzzleState::Direction> scramble{
       PuzzleState::Direction::RIGHT, PuzzleState::Direction::DOWN, PuzzleState::Direction::RIGHT,
       PuzzleState::Direction::UP,    PuzzleState::Direction::DOWN, PuzzleState::Direction::LEFT};

   PuzzleState start = applyMoves(goal, scramble);
   REQUIRE(start.asArray() != allZeroArr());

   size_t bfsIters = 0;
   size_t astIters = 0;
   vector<PuzzleState> bfsPath = solveBFS(start, goal, &bfsIters);
   vector<PuzzleState> astPath = solveAstar(start, goal, &astIters);

   checkPathValid(bfsPath, start, goal);
   checkPathValid(astPath, start, goal);

   // Both should be optimal length
   REQUIRE(bfsPath.size() == astPath.size());

   // A* should generally expand no more than BFS on these tiny cases
   REQUIRE(astIters <= bfsIters);
}

TEST_CASE("Solutions do not mutate input states", "[weight=2][part=2]") {
   PuzzleState goal;
   PuzzleState start =
       applyMoves(goal, {PuzzleState::Direction::RIGHT, PuzzleState::Direction::DOWN});

   PuzzleState startCopy = start;
   PuzzleState goalCopy = goal;

   (void)solveBFS(start, goal, nullptr);
   REQUIRE(start == startCopy);
   REQUIRE(goal == goalCopy);

   (void)solveAstar(start, goal, nullptr);
   REQUIRE(start == startCopy);
   REQUIRE(goal == goalCopy);
}
