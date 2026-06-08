#include "cs225/PNG.h"
#include "dsets.h"
#include "maze.h"
#include "mazereader.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>

using namespace cs225;
using namespace std;

TEST_CASE("addElements1", "[weight=10][part1]") {
   DisjointSets disjSets(5);
   REQUIRE(3 == disjSets.find(3));
}

TEST_CASE("addElements2", "[weight=10][part1]") {
   DisjointSets disjSets;
   disjSets.addElements(5);
   REQUIRE(0 == disjSets.find(0));
   disjSets.addElements(5);
   REQUIRE(9 == disjSets.find(9));
}

TEST_CASE("testFindAndSetUnion1", "[weight=10][part1]") {
   DisjointSets disjSets(4);
   disjSets.setUnion(1, 2);
   REQUIRE(disjSets.find(2) == disjSets.find(1));
}

TEST_CASE("testFindAndSetUnion2", "[weight=10][part1]") {
   DisjointSets disjSets(10);

   disjSets.setUnion(4, 8);
   disjSets.setUnion(1, 3);
   disjSets.setUnion(1, 5);
   disjSets.setUnion(8, 3);
   disjSets.setUnion(6, 7);
   disjSets.setUnion(2, 8);
   disjSets.setUnion(7, 1);

   int root = disjSets.find(1);
   for (int i = 2; i <= 8; i++)
      REQUIRE(root == disjSets.find(i));
   REQUIRE(disjSets.find(0) != disjSets.find(9));
}

TEST_CASE("testFindAndSetUnion3", "[weight=10][part1]") {
   DisjointSets disjSets(8);

   disjSets.setUnion(1, 2);
   disjSets.setUnion(1, 3);
   disjSets.setUnion(4, 5);
   disjSets.setUnion(4, 6);
   disjSets.setUnion(3, 6);

   REQUIRE(disjSets.find(4) == disjSets.find(1));
   REQUIRE(disjSets.find(0) != disjSets.find(4));
}

TEST_CASE("size1", "[weight=5][part1]") {
   DisjointSets disjSets(4);
   REQUIRE(1 == disjSets.size(2));
}

TEST_CASE("size2", "[weight=10][part1]") {
   DisjointSets disjSets(8);

   disjSets.setUnion(1, 7);
   disjSets.setUnion(2, 6);
   disjSets.setUnion(3, 5);
   disjSets.setUnion(2, 3);
   REQUIRE(4 == disjSets.size(3));
   disjSets.setUnion(1, 3);
   REQUIRE(6 == disjSets.size(6));
}
