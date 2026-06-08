#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <vector>

#include "cs225/point.h"
#include "kdtree.h"

// ------------------------------
// Helpers
// ------------------------------

template <int Dim>
static double sqDist(const Point<Dim> &a, const Point<Dim> &b) {
   double s = 0;
   for (int i = 0; i < Dim; i++) {
      double d = a[i] - b[i];
      s += d * d;
   }
   return s;
}

template <int Dim>
static Point<Dim> bruteForceNN(const std::vector<Point<Dim>> &pts, const Point<Dim> &q) {
   REQUIRE(!pts.empty());
   Point<Dim> best = pts[0];
   double bestD = sqDist(best, q);
   for (size_t i = 1; i < pts.size(); i++) {
      double d = sqDist(pts[i], q);
      if (d < bestD || (d == bestD && pts[i] < best)) {
         best = pts[i];
         bestD = d;
      }
   }
   return best;
}

// Strict comparator for ints (important: must be strict-weak-ordering)
static auto intLess = [](int a, int b) { return a < b; };

// ------------------------------
// Mine Action
// ------------------------------
template <int Dim>
class MineActionFAIL : public Point<Dim>::MineAction {
public:
   void onMine(const Point<Dim> &point) const override {
      if (trigger) {
         FAIL("Mine visited: " << point);
      }
   }
   bool trigger = false;
};

// ------------------------------
// smallerDimVal / shouldReplace
// ------------------------------

TEST_CASE("smallerDimVal handles ties using operator<", "[part=1][weight=1]") {
   Point<3> a(1, 2, 3);
   Point<3> b(1, 2, 4);

   REQUIRE(smallerDimVal(a, b, 0) == true);
   REQUIRE(smallerDimVal(a, b, 1) == true);
   REQUIRE(smallerDimVal(a, b, 2) == true);
   REQUIRE(smallerDimVal(b, a, 2) == false);
}

TEST_CASE("shouldReplace uses distance then operator< tie-break", "[part=1][weight=1]") {
   Point<3> target(0, 0, 0);
   Point<3> cur(1, 0, 0); // dist^2 = 1
   Point<3> pot(0, 1, 0); // dist^2 = 1, tie

   REQUIRE(shouldReplace(target, cur, pot) == (pot < cur));
}

TEST_CASE("shouldReplace prefers strictly closer even if operator< says otherwise",
          "[part=1][weight=1]") {
   Point<2> target(0, 0);
   Point<2> cur(100, 100);
   Point<2> pot(1, 0);
   REQUIRE(shouldReplace(target, cur, pot) == true);
}

// ------------------------------
// select / partition correctness
// ------------------------------

TEST_CASE("select places kth element equal to std::nth_element result (ints)",
          "[part=1][weight=1]") {
   std::vector<int> v = {5, 42, 6, 21, 1, 99, 57, 87, 27};

   for (size_t k = 0; k < v.size(); k++) {
      auto a = v;
      auto b = v;

      select(a.begin(), a.end(), a.begin() + k, intLess);
      std::nth_element(b.begin(), b.begin() + k, b.end(), intLess);

      REQUIRE(a[k] == b[k]);

      for (size_t i = 0; i < k; i++)
         REQUIRE(!(a[k] < a[i])); // a[i] <= a[k]
      for (size_t i = k + 1; i < a.size(); i++)
         REQUIRE(!(a[i] < a[k])); // a[k] <= a[i]
   }
}

TEST_CASE("select randomized stress test vs nth_element (ints)", "[part=1][weight=2]") {
   std::mt19937 rng(1337);
   std::uniform_int_distribution<int> dist(-1000, 1000);

   for (int trial = 0; trial < 200; trial++) {
      int n = 1 + (rng() % 80);
      std::vector<int> v(n);
      for (int i = 0; i < n; i++)
         v[i] = dist(rng);

      int k = rng() % n;

      auto a = v;
      auto b = v;

      select(a.begin(), a.end(), a.begin() + k, intLess);
      std::nth_element(b.begin(), b.begin() + k, b.end(), intLess);

      REQUIRE(a[k] == b[k]);
   }
}

// ------------------------------
// KDTree NN correctness vs brute force
// ------------------------------

TEST_CASE("KDTree::findNearestNeighbor matches brute force on random 2D points",
          "[part=1][weight=3]") {
   std::mt19937 rng(2025);
   std::uniform_real_distribution<double> dist(-50.0, 50.0);

   std::vector<Point<2>> pts;
   pts.reserve(200);
   for (int i = 0; i < 200; i++) {
      double c[2] = {dist(rng), dist(rng)};
      pts.emplace_back(c);
   }

   KDTree<2> tree(pts);

   for (int q = 0; q < 200; q++) {
      double qc[2] = {dist(rng), dist(rng)};
      Point<2> query(qc);

      Point<2> expected = bruteForceNN(pts, query);
      Point<2> actual = tree.findNearestNeighbor(query);

      REQUIRE(actual == expected);
   }
}

TEST_CASE("KDTree::findNearestNeighbor matches brute force on random 3D points",
          "[part=1][weight=3]") {
   std::mt19937 rng(777);
   std::uniform_real_distribution<double> dist(-30.0, 30.0);

   std::vector<Point<3>> pts;
   pts.reserve(150);
   for (int i = 0; i < 150; i++) {
      double c[3] = {dist(rng), dist(rng), dist(rng)};
      pts.emplace_back(c);
   }

   KDTree<3> tree(pts);

   for (int q = 0; q < 150; q++) {
      double qc[3] = {dist(rng), dist(rng), dist(rng)};
      Point<3> query(qc);

      Point<3> expected = bruteForceNN(pts, query);
      Point<3> actual = tree.findNearestNeighbor(query);

      REQUIRE(actual == expected);
   }
}

// ------------------------------
// KDTree tie-breaking tests
// ------------------------------

TEST_CASE("KDTree NN tie-breaking: equal distance returns operator< smallest (1D)",
          "[part=1][weight=2]") {
   double a0[1] = {-1};
   double a1[1] = {+1};
   double q[1] = {0};

   std::vector<Point<1>> pts = {Point<1>(a1), Point<1>(a0)};
   KDTree<1> tree(pts);

   Point<1> query(q);
   Point<1> ans = tree.findNearestNeighbor(query);

   REQUIRE(ans == Point<1>(a0));
}

TEST_CASE("KDTree NN tie-breaking: multiple equal-distance candidates (2D)", "[part=1][weight=2]") {
   double p1[2] = {1, 0};
   double p2[2] = {-1, 0};
   double p3[2] = {0, 1};
   double p4[2] = {0, -1};
   double q[2] = {0, 0};

   std::vector<Point<2>> pts = {Point<2>(p1), Point<2>(p2), Point<2>(p3), Point<2>(p4)};
   KDTree<2> tree(pts);

   Point<2> query(q);
   Point<2> expected = bruteForceNN(pts, query);
   Point<2> actual = tree.findNearestNeighbor(query);

   REQUIRE(actual == expected);
}

// ------------------------------
// KDTree pruning path tests with mines
// ------------------------------

TEST_CASE("KDTree NN path: wrong far-branch choice triggers mine (2D)", "[part=1][weight=4]") {
   MineActionFAIL<2> action;

   double safe[][2] = {{-8, 7}, {-6, 4}, {-5, 6}, {-3, 5}, {0, 7}};
   double mines[][2] = {{2, -7}, {3, 0}, {5, -4}, {6, -3}, {7, -6}};

   std::vector<Point<2>> pts;
   for (auto &c : safe)
      pts.emplace_back(Point<2>(c, false, &action));
   for (auto &c : mines)
      pts.emplace_back(Point<2>(c, true, &action));

   KDTree<2> tree(pts);

   action.trigger = true;

   double qcoords[2] = {-7, 5};
   double ecoords[2] = {-6, 4};
   Point<2> query(qcoords);
   Point<2> expected(ecoords);

   REQUIRE(tree.findNearestNeighbor(query) == expected);
}

TEST_CASE("KDTree NN path: mines pruned by root split (2D, deterministic)", "[part=1][weight=3]") {
   MineActionFAIL<2> action;

   double safe0[2] = {-1000, 0};
   double safe1[2] = {-900, 10};
   double safe2[2] = {-800, -10};
   double safeRoot[2] = {100, 5};

   double mine0[2] = {1000, 0};
   double mine1[2] = {1100, 10};
   double mine2[2] = {1200, -10};

   std::vector<Point<2>> pts;
   pts.emplace_back(Point<2>(safe0, false, &action));
   pts.emplace_back(Point<2>(safe1, false, &action));
   pts.emplace_back(Point<2>(safe2, false, &action));
   pts.emplace_back(Point<2>(safeRoot, false, &action));

   pts.emplace_back(Point<2>(mine0, true, &action));
   pts.emplace_back(Point<2>(mine1, true, &action));
   pts.emplace_back(Point<2>(mine2, true, &action));

   KDTree<2> tree(pts);

   // Turn on mine detection ONLY for the KDTree query
   action.trigger = true;

   Point<2> query(safe0);
   Point<2> expected(safe0); // exact match exists

   REQUIRE(tree.findNearestNeighbor(query) == expected);
}

TEST_CASE("KDTree NN path: mines pruned by root split (3D, deterministic)", "[part=1][weight=3]") {
   MineActionFAIL<3> action;

   double safe0[3] = {-1000, 0, 0};
   double safe1[3] = {-900, 10, 1};
   double safe2[3] = {-800, -10, 2};
   double safeRoot[3] = {100, 5, 3};

   double mine0[3] = {1000, 0, 0};
   double mine1[3] = {1100, 10, 1};
   double mine2[3] = {1200, -10, 2};

   std::vector<Point<3>> pts;
   pts.emplace_back(Point<3>(safe0, false, &action));
   pts.emplace_back(Point<3>(safe1, false, &action));
   pts.emplace_back(Point<3>(safe2, false, &action));
   pts.emplace_back(Point<3>(safeRoot, false, &action));

   pts.emplace_back(Point<3>(mine0, true, &action));
   pts.emplace_back(Point<3>(mine1, true, &action));
   pts.emplace_back(Point<3>(mine2, true, &action));

   KDTree<3> tree(pts);

   action.trigger = true;

   Point<3> query(safe0);
   Point<3> expected(safe0);

   REQUIRE(tree.findNearestNeighbor(query) == expected);
}
