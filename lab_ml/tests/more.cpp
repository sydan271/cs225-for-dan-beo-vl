
// nim_learner_more_tests.cpp
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "nim_graph/edge.h"
#include "nim_graph/graph.h"
#include "nim_learner.h"

using std::string;
using std::vector;

// ---------- Helpers ----------
static int playerOf(const string &v) {
   // Expected format: "p1-10" or "p2-0"
   if (v.size() < 4)
      return -1;
   if (v[0] != 'p')
      return -1;
   if (!std::isdigit(static_cast<unsigned char>(v[1])))
      return -1;
   return v[1] - '0';
}

static int tokensOf(const string &v) {
   auto dash = v.find('-');
   if (dash == string::npos || dash + 1 >= v.size())
      return -1;
   // stoi handles "0", "10", etc.
   return std::stoi(v.substr(dash + 1));
}

static bool isState(const string &v) {
   int p = playerOf(v);
   int t = tokensOf(v);
   if (!(p == 1 || p == 2))
      return false;
   if (t < 0)
      return false;
   return true;
}

static string makeState(int player, int tokens) {
   return "p" + std::to_string(player) + "-" + std::to_string(tokens);
}

static size_t expectedEdgeCount(unsigned n) {
   // Directed legal moves: take 1 or 2, toggling player.
   // For i=1..n: 2 edges (p1-i->p2-(i-1), p2-i->p1-(i-1)) => 2n
   // For i=2..n: 2 edges (p1-i->p2-(i-2), p2-i->p1-(i-2)) => 2(n-1)
   // Total = 4n - 2 for n>=1, else 0
   return (n == 0) ? 0u : static_cast<size_t>(4 * n - 2);
}

// ---------- Part 1: Constructor graph structure ----------
TEST_CASE("Constructor creates all vertices p1-i and p2-i for i=0..n", "[nim][part=1]") {
   NimLearner nim(10);
   const Graph &g = nim.getGraph();

   for (int i = 0; i <= 10; i++) {
      REQUIRE(g.vertexExists(makeState(1, i)));
      REQUIRE(g.vertexExists(makeState(2, i)));
   }
}

TEST_CASE("Constructor creates required starting vertex p1-n", "[nim][part=1]") {
   NimLearner nim(10);
   const Graph &g = nim.getGraph();
   REQUIRE(g.vertexExists("p1-10"));
}

TEST_CASE("Constructor graph is directed and edge weights start at 0", "[nim][part=1]") {
   NimLearner nim(5);
   const Graph &g = nim.getGraph();

   REQUIRE(g.isDirected() == true);

   // Check a known edge exists and has weight 0
   REQUIRE(g.edgeExists("p1-1", "p2-0"));
   REQUIRE(g.getEdgeWeight("p1-1", "p2-0") == 0);
}

TEST_CASE("Constructor creates exactly the legal edges and initializes all weights to 0",
          "[nim][part=1]") {
   const unsigned n = 8;
   NimLearner nim(n);
   const Graph &g = nim.getGraph();

   // Edge count check:
   auto edges = g.getEdges();
   REQUIRE(edges.size() == expectedEdgeCount(n));

   // Every edge must be legal:
   for (const auto &e : edges) {
      REQUIRE(isState(e.source));
      REQUIRE(isState(e.dest));

      int ps = playerOf(e.source);
      int pd = playerOf(e.dest);
      int ts = tokensOf(e.source);
      int td = tokensOf(e.dest);

      int expectedPd = (ps == 1) ? 2 : 1;
      REQUIRE(pd == expectedPd);

      REQUIRE(ts >= 0);
      REQUIRE(td >= 0);
      REQUIRE(ts > td);

      int delta = ts - td;
      bool legalDelta = (delta == 1) || (delta == 2);
      REQUIRE(legalDelta);

      REQUIRE(g.getEdgeWeight(e.source, e.dest) == 0);
   }

   // No outgoing edges from terminal states:
   REQUIRE(g.getAdjacent("p1-0").empty());
   REQUIRE(g.getAdjacent("p2-0").empty());
}

TEST_CASE("Constructor creates correct edges for n=3", "[nim][part=1]") {
   NimLearner nim(3);
   const Graph &g = nim.getGraph();

   // From p1-3: to p2-2 (take 1) and p2-1 (take 2)
   REQUIRE(g.edgeExists("p1-3", "p2-2"));
   REQUIRE(g.edgeExists("p1-3", "p2-1"));

   // From p2-3: to p1-2 and p1-1
   REQUIRE(g.edgeExists("p2-3", "p1-2"));
   REQUIRE(g.edgeExists("p2-3", "p1-1"));

   // From p1-2: to p2-1 and p2-0
   REQUIRE(g.edgeExists("p1-2", "p2-1"));
   REQUIRE(g.edgeExists("p1-2", "p2-0"));

   // From p2-2: to p1-1 and p1-0
   REQUIRE(g.edgeExists("p2-2", "p1-1"));
   REQUIRE(g.edgeExists("p2-2", "p1-0"));

   // From p1-1: to p2-0 only
   REQUIRE(g.edgeExists("p1-1", "p2-0"));
   REQUIRE(g.getAdjacent("p1-1").size() == 1);

   // From p2-1: to p1-0 only
   REQUIRE(g.edgeExists("p2-1", "p1-0"));
   REQUIRE(g.getAdjacent("p2-1").size() == 1);
}

// ---------- Part 2: playRandomGame correctness properties ----------
TEST_CASE("playRandomGame starts at p1-n", "[nim][part=2]") {
   const unsigned n = 10;
   NimLearner nim(n);

   std::srand(0);
   vector<Edge> path = nim.playRandomGame();

   REQUIRE(!path.empty());
   REQUIRE(path.front().source == makeState(1, static_cast<int>(n)));
}

TEST_CASE("playRandomGame ends at p1-0 or p2-0", "[nim][part=2]") {
   NimLearner nim(10);

   std::srand(1);
   vector<Edge> path = nim.playRandomGame();

   REQUIRE(!path.empty());
   REQUIRE(tokensOf(path.back().dest) == 0);

   bool endIsP1 = (path.back().dest == "p1-0");
   bool endIsP2 = (path.back().dest == "p2-0");
   bool okEnd = endIsP1 || endIsP2;
   REQUIRE(okEnd);
}

TEST_CASE("playRandomGame edges exist in graph and follow legality + continuity rules",
          "[nim][part=2]") {
   const unsigned n = 10;
   NimLearner nim(n);
   const Graph &g = nim.getGraph();

   std::srand(2);
   vector<Edge> path = nim.playRandomGame();
   REQUIRE(!path.empty());

   for (size_t i = 0; i < path.size(); i++) {
      const auto &e = path[i];

      REQUIRE(g.edgeExists(e.source, e.dest));
      REQUIRE(isState(e.source));
      REQUIRE(isState(e.dest));

      int ps = playerOf(e.source);
      int pd = playerOf(e.dest);
      int ts = tokensOf(e.source);
      int td = tokensOf(e.dest);

      int expectedPd = (ps == 1) ? 2 : 1;
      REQUIRE(pd == expectedPd);

      REQUIRE(ts > 0);
      REQUIRE(td >= 0);

      int delta = ts - td;
      bool legalDelta = (delta == 1) || (delta == 2);
      REQUIRE(legalDelta);

      if (i + 1 < path.size()) {
         REQUIRE(path[i].dest == path[i + 1].source);
      }
   }

   // Move count bounds: ceil(n/2) <= moves <= n
   REQUIRE(path.size() >= (n + 1) / 2);
   REQUIRE(path.size() <= n);
}

TEST_CASE("NimLearner(1) playRandomGame returns exactly one move p1-1 -> p2-0", "[nim][part=2]") {
   NimLearner nim(1);

   std::srand(0);
   vector<Edge> path = nim.playRandomGame();

   REQUIRE(path.size() == 1);
   REQUIRE(path[0].source == "p1-1");
   REQUIRE(path[0].dest == "p2-0");
}

// ---------- Part 3: updateEdgeWeights correctness ----------
TEST_CASE("updateEdgeWeights for n=1 rewards Player 1 by +1 on the only edge", "[nim][part=3]") {
   NimLearner nim(1);
   const Graph &g = nim.getGraph();

   std::srand(0);
   vector<Edge> path = nim.playRandomGame();
   REQUIRE(path.size() == 1);

   Edge e = path[0];
   REQUIRE(g.getEdgeWeight(e.source, e.dest) == 0);

   nim.updateEdgeWeights(path);
   REQUIRE(g.getEdgeWeight(e.source, e.dest) == 1);
}

TEST_CASE("updateEdgeWeights rewards winner edges and punishes loser edges on a crafted path",
          "[nim][part=3]") {
   NimLearner nim(2);
   const Graph &g = nim.getGraph();

   // Player 2 wins:
   // p1-2 -> p2-1
   // p2-1 -> p1-0
   vector<Edge> path;
   path.push_back(g.getEdge("p1-2", "p2-1"));
   path.push_back(g.getEdge("p2-1", "p1-0"));

   REQUIRE(g.getEdgeWeight("p1-2", "p2-1") == 0);
   REQUIRE(g.getEdgeWeight("p2-1", "p1-0") == 0);

   nim.updateEdgeWeights(path);

   // Winner is p2 => p2-edge +1, p1-edge -1
   REQUIRE(g.getEdgeWeight("p1-2", "p2-1") == -1);
   REQUIRE(g.getEdgeWeight("p2-1", "p1-0") == 1);
}

TEST_CASE("After many games (n=4), terminal-edge weights sum to games played", "[nim][part=3]") {
   NimLearner nim(4);
   const Graph &g = nim.getGraph();

   std::srand(0);
   const int GAMES = 10000;

   for (int i = 0; i < GAMES; i++) {
      vector<Edge> path = nim.playRandomGame();
      nim.updateEdgeWeights(path);
   }

   // Terminal edges are those that go to token 0:
   int sumTerminal = g.getEdgeWeight("p1-1", "p2-0") + g.getEdgeWeight("p1-2", "p2-0") +
                     g.getEdgeWeight("p2-1", "p1-0") + g.getEdgeWeight("p2-2", "p1-0");

   REQUIRE(sumTerminal == GAMES);
}

// ---------- Part 4: labelEdgesFromThreshold correctness ----------
TEST_CASE("labelEdgesFromThreshold sets WIN for weight>t and LOSE for weight<-t", "[nim][part=4]") {
   NimLearner nim(3);
   const Graph &g = nim.getGraph();

   // Train to create non-zero weights:
   std::srand(0);
   for (int i = 0; i < 5000; i++) {
      auto path = nim.playRandomGame();
      nim.updateEdgeWeights(path);
   }

   // Label with threshold 0:
   nim.labelEdgesFromThreshold(0);

   // Validate labels match sign (ignore exactly 0):
   for (const auto &v : g.getVertices()) {
      for (const auto &w : g.getAdjacent(v)) {
         int wt = g.getEdgeWeight(v, w);
         string lab = g.getEdgeLabel(v, w);

         if (wt > 0)
            REQUIRE(lab == "WIN");
         if (wt < 0)
            REQUIRE(lab == "LOSE");
      }
   }
}
