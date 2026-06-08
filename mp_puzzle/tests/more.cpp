
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "PuzzleAnimation.h"
#include "puzzle.h"

// ---------- Helpers ----------

static void animateSolution(const std::vector<PuzzleState> &solution, const std::string &filename) {
   PuzzleAnimation animation(solution);
   animation.writeToFile(filename);
}

// Check if b is a valid one-move neighbor of a (in ANY direction)
static bool isOneMoveNeighbor(const PuzzleState &a, const PuzzleState &b) {
   auto neigh = a.getNeighbors();
   return std::find(neigh.begin(), neigh.end(), b) != neigh.end();
}

// Validate a returned solution path:
// - empty is allowed only when "expectSolvable == false"
// - otherwise must start at start, end at goal, and every consecutive step must be a legal move
static void validatePath(const std::vector<PuzzleState> &path,
                         const PuzzleState &start,
                         const PuzzleState &goal,
                         bool expectSolvable) {
   if (!expectSolvable) {
      REQUIRE(path.empty());
      return;
   }

   REQUIRE_FALSE(path.empty());
   REQUIRE(path.front() == start);
   REQUIRE(path.back() == goal);

   for (size_t i = 1; i < path.size(); ++i) {
      REQUIRE(isOneMoveNeighbor(path[i - 1], path[i]));
   }
}

// Optional: “exact path” check (use only for tests where you KNOW the path is unique / you want
// strict)
static void validateExactPath(const std::vector<PuzzleState> &path,
                              const std::vector<PuzzleState> &expected) {
   REQUIRE(path.size() == expected.size());
   for (size_t i = 0; i < expected.size(); ++i) {
      REQUIRE(path[i] == expected[i]);
   }
}

// Solve wrapper
static size_t runSolve(const PuzzleState &start,
                       const PuzzleState &goal,
                       bool bfs,
                       std::vector<PuzzleState> &out,
                       size_t *iters = nullptr) {
   if (bfs) out = solveBFS(start, goal, iters);
   else out = solveAstar(start, goal, iters);
   return iters ? *iters : 0;
}

// For 4x4 15-puzzle solvability:
// Using standard rule: with blank row counting from bottom (1..4).
// solvable iff (inversions + blankRowFromBottom) is odd for even width=4.
static bool isSolvable15(const PuzzleState &s) {
   auto a = s.asArray();
   int inv = 0;
   int blankIdx = -1;

   for (int i = 0; i < 16; ++i) {
      if (a[i] == 0) blankIdx = i;
   }

   for (int i = 0; i < 16; ++i) {
      if (a[i] == 0) continue;
      for (int j = i + 1; j < 16; ++j) {
         if (a[j] == 0) continue;
         if (a[i] > a[j]) ++inv;
      }
   }

   int blankRowFromTop = blankIdx / 4;           // 0..3
   int blankRowFromBottom = 4 - blankRowFromTop; // 1..4

   return ((inv + blankRowFromBottom) % 2) == 1;
}

// ---------- Tests: Part 2 BFS / A* ----------

TEST_CASE("BFS: solved puzzle returns size 1 and iterations==1", "[part=2][BFS][weight=1]") {
   size_t it = 999;
   std::vector<PuzzleState> sol;
   runSolve(PuzzleState(), PuzzleState(), true, sol, &it);

   validatePath(sol, PuzzleState(), PuzzleState(), true);
   REQUIRE(sol.size() == 1);
   REQUIRE(it == 1);
}

TEST_CASE("A*: solved puzzle returns size 1 and iterations==1", "[part=2][A*][weight=1]") {
   size_t it = 999;
   std::vector<PuzzleState> sol;
   runSolve(PuzzleState(), PuzzleState(), false, sol, &it);

   validatePath(sol, PuzzleState(), PuzzleState(), true);
   REQUIRE(sol.size() == 1);
   REQUIRE(it == 1);
}

TEST_CASE("BFS/A*: iterations pointer may be NULL", "[part=2][weight=1]") {
   PuzzleState start({1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12});
   PuzzleState goal;

   // should not crash:
   auto a = solveBFS(start, goal, nullptr);
   auto b = solveAstar(start, goal, nullptr);

   validatePath(a, start, goal, true);
   validatePath(b, start, goal, true);
}

TEST_CASE("BFS: easy cases (validate legality + optimal length)", "[part=2][BFS][weight=2]") {
   PuzzleState goal;

   PuzzleState start1({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15}); // 1 move away

   std::vector<PuzzleState> sol;
   size_t it = 0;
   runSolve(start1, goal, true, sol, &it);

   validatePath(sol, start1, goal, true);
   REQUIRE(sol.size() == 2); // start + goal = 2 states, 1 move

   PuzzleState start2({1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15,
                       12}); // known 3 moves away in your examples

   runSolve(start2, goal, true, sol, &it);
   validatePath(sol, start2, goal, true);
   REQUIRE(sol.size() == 4); // 3 moves => 4 states
}

TEST_CASE("A*: easy cases (validate legality + optimal length)", "[part=2][A*][weight=2]") {
   PuzzleState goal;

   PuzzleState start1({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15}); // 1 move away

   std::vector<PuzzleState> sol;
   size_t it = 0;
   runSolve(start1, goal, false, sol, &it);

   validatePath(sol, start1, goal, true);
   REQUIRE(sol.size() == 2);

   PuzzleState start2({1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}); // 3 moves away

   runSolve(start2, goal, false, sol, &it);
   validatePath(sol, start2, goal, true);
   REQUIRE(sol.size() == 4);
}

TEST_CASE("Spiral case: BFS returns correct exact path", "[part=2][BFS][weight=2][timeout=60000]") {
   std::vector<PuzzleState> expected = {
       PuzzleState({5, 1, 2, 3, 9, 10, 6, 4, 13, 0, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 0, 6, 4, 13, 10, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 0, 4, 13, 10, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 0, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 14, 15, 0, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 14, 0, 15, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 0, 14, 15, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 0, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({5, 1, 2, 3, 0, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({0, 1, 2, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 0, 2, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 0, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0})};

   size_t it = 0;
   auto got = solveBFS(expected.front(), expected.back(), &it);

   validateExactPath(got, expected);
   validatePath(got, expected.front(), expected.back(), true);
}

TEST_CASE("Spiral case: A* returns correct exact path", "[part=2][A*][weight=2][timeout=15000]") {
   std::vector<PuzzleState> expected = {
       PuzzleState({5, 1, 2, 3, 9, 10, 6, 4, 13, 0, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 0, 6, 4, 13, 10, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 0, 4, 13, 10, 7, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 0, 8, 14, 15, 11, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 14, 15, 0, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 14, 0, 15, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 13, 10, 11, 8, 0, 14, 15, 12}),
       PuzzleState({5, 1, 2, 3, 9, 6, 7, 4, 0, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({5, 1, 2, 3, 0, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({0, 1, 2, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 0, 2, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 0, 3, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 8, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 13, 14, 15, 12}),
       PuzzleState({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0})};

   size_t it = 0;
   auto got = solveAstar(expected.front(), expected.back(), &it);

   validateExactPath(got, expected);
   validatePath(got, expected.front(), expected.back(), true);
}

TEST_CASE("Unsolvable puzzles: BFS and A* return empty", "[part=2][weight=2]") {
   // Classic unsolvable: swap 14 and 15 in solved state
   PuzzleState unsolvable({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 14, 0});
   PuzzleState goal;

   REQUIRE_FALSE(isSolvable15(unsolvable));
   REQUIRE(isSolvable15(goal));

   size_t it = 0;
   auto b = solveBFS(unsolvable, goal, &it);
   validatePath(b, unsolvable, goal, false);

   it = 0;
   auto a = solveAstar(unsolvable, goal, &it);
   validatePath(a, unsolvable, goal, false);
}

TEST_CASE("A* should expand far fewer states than BFS on a hard case",
          "[part=2][weight=2][timeout=20000]") {
   PuzzleState start({5, 1, 7, 4, 9, 3, 11, 8, 0, 2, 12, 15, 13, 6, 10, 14});
   PuzzleState goal;

   size_t itB = 0, itA = 0;
   std::vector<PuzzleState> sb, sa;

   runSolve(start, goal, true, sb, &itB);
   runSolve(start, goal, false, sa, &itA);

   validatePath(sb, start, goal, true);
   validatePath(sa, start, goal, true);

   // both should be optimal length; thus same number of moves => same path length
   REQUIRE(sb.size() == sa.size());

   // A* should be meaningfully smaller; keep this loose to avoid overfitting
   REQUIRE(itA < itB);
   REQUIRE(itB > 100000);
}
