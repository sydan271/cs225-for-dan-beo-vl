#include <catch2/catch_test_macros.hpp>

#include "cs225/textfile.h"
#include "dhhashtable.h"
#include "hashes.h"
#include "lphashtable.h"
#include "schashtable.h"

#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::set;
using std::string;
using std::unordered_map;
using std::vector;

// ---------------------- Helpers ----------------------

static size_t min_size_for_load(size_t elems, double max_load) {
   return static_cast<size_t>(std::ceil(static_cast<double>(elems) / max_load));
}

template <typename HT>
static set<string> keys_via_iterator(const HT &ht) {
   set<string> keys;
   for (auto it = ht.begin(); it != ht.end(); ++it) {
      keys.insert(it->first);
   }
   return keys;
}

// Find N distinct strings that collide under hashes::hash(str, tableSize)
static vector<string> find_colliding_strings(size_t tableSize, size_t needed) {
   unordered_map<size_t, vector<string>> buckets;
   for (size_t i = 0; i < 500000; i++) {
      string s = "col_" + std::to_string(i);
      size_t h = hashes::hash(s, static_cast<int>(tableSize));
      auto &vec = buckets[h];
      vec.push_back(s);
      if (vec.size() >= needed)
         return vec;
   }
   // If this ever happens, something is very wrong with the hash or tableSize.
   return {};
}

template <typename HT>
static void basic_insert_find_checks() {
   TextFile infile("../data/textEasy.txt");
   HT ht(32);

   int val = 0;
   while (infile.good()) {
      string word = infile.getNextWord();
      if (word.empty())
         continue;
      ht.insert(word, ++val);
   }

   REQUIRE(ht.keyExists("all"));
   REQUIRE(ht.keyExists("base"));
   REQUIRE(ht.find("are") == 4);
}

template <typename HT>
static void remove_easy_checks() {
   TextFile infile("../data/textEasy.txt");
   HT ht(32);

   while (infile.good()) {
      string word = infile.getNextWord();
      if (word.empty())
         continue;
      ht.insert(word, 0);
   }

   ht.insert("__SENTINEL__", 0);
   REQUIRE(ht.keyExists("__SENTINEL__"));

   ht.remove("all");
   REQUIRE_FALSE(ht.keyExists("all"));

   ht.remove("base");
   REQUIRE_FALSE(ht.keyExists("base"));

   // removing missing key should be safe
   ht.remove("__NOT_A_KEY__");
   REQUIRE(ht.keyExists("__SENTINEL__"));
}

template <typename HT>
static void remove_hard_update_checks() {
   TextFile infile("../data/textHard.txt");
   HT ht(32);

   // update pattern required by spec: remove then insert
   while (infile.good()) {
      string word = infile.getNextWord();
      if (word.empty())
         continue;

      if (ht.keyExists(word)) {
         ht.remove(word);
         REQUIRE_FALSE(ht.keyExists(word));
         ht.insert(word, 1);
      } else {
         ht.insert(word, 0);
      }
   }

   ht.insert("__SENTINEL__", 0);
   REQUIRE(ht.keyExists("__SENTINEL__"));

   ht.remove("to");
   REQUIRE_FALSE(ht.keyExists("to"));

   ht.remove("decided");
   REQUIRE_FALSE(ht.keyExists("decided"));
}

template <typename HT>
static void resize_once_textLong_checks() {
   TextFile infile("../data/textLong.txt");
   HT ht(16);

   int val = 0;
   while (infile.good()) {
      string word = infile.getNextWord();
      if (word.empty())
         continue;

      if (!ht.keyExists(word)) {
         ht.insert(word, ++val);
      }
   }

   REQUIRE(ht.tableSize() >= min_size_for_load(static_cast<size_t>(val), 0.7));
   REQUIRE(ht.find("got") == 2);
   REQUIRE(ht.find("to") == 3);
}

template <typename HT>
static void resize_all_alpha_checks() {
   vector<string> strings;
   for (int c = 'a'; c <= 'z'; c++) {
      string s;
      s.push_back(static_cast<char>(c));
      strings.push_back(s);
   }

   HT ht(16);
   for (size_t i = 0; i < strings.size(); i++) {
      ht.insert(strings[i], static_cast<int>(i + 1));
   }

   REQUIRE(ht.tableSize() >= min_size_for_load(strings.size(), 0.7));

   for (auto &s : strings) {
      REQUIRE(ht.keyExists(s));
   }
}

template <typename HT>
static void bracket_operator_checks() {
   HT ht(17);

   int &r = ht["missing"];
   REQUIRE(r == 0);

   r = 42;
   REQUIRE(ht.keyExists("missing"));
   REQUIRE(ht.find("missing") == 42);

   ht.insert("x", 7);
   int &r2 = ht["x"];
   REQUIRE(r2 == 7);
   r2 = 9;
   REQUIRE(ht.find("x") == 9);
}

template <typename HT>
static void iterator_checks() {
   HT ht(7);
   set<string> expected;

   for (int i = 0; i < 120; i++) {
      string k = "it" + std::to_string(i);
      expected.insert(k);
      ht.insert(k, i);
   }

   REQUIRE(keys_via_iterator(ht) == expected);

   // trigger growth and ensure iterator still correct
   for (int i = 120; i < 260; i++) {
      string k = "it" + std::to_string(i);
      expected.insert(k);
      ht.insert(k, i);
   }

   REQUIRE(keys_via_iterator(ht) == expected);
}

template <typename HT>
static void copy_and_assignment_checks() {
   HT ht(7);
   for (int i = 0; i < 150; i++) {
      ht.insert("c" + std::to_string(i), i);
   }

   // copy ctor
   HT copy(ht);

   ht.remove("c5");
   ht.insert("new", 999);

   REQUIRE(copy.keyExists("c5"));
   REQUIRE_FALSE(copy.keyExists("new"));
   REQUIRE(copy.find("c5") == 5);

   // assignment
   HT other(17);
   other = ht;

   ht.remove("c8");
   ht.insert("zzz", 777);

   REQUIRE(other.keyExists("c8"));
   REQUIRE_FALSE(other.keyExists("zzz"));
}

// Open addressing tombstone behavior using colliding STRING keys (no int hashes needed)
static void tombstone_checks_LP_strings() {
   const size_t S = 17;
   auto keys = find_colliding_strings(S, 3);
   REQUIRE(keys.size() >= 3);

   LPHashTable<string, int> ht(S);

   const string &k0 = keys[0];
   const string &k1 = keys[1];
   const string &k2 = keys[2];

   ht.insert(k0, 100);
   ht.insert(k1, 200);
   ht.insert(k2, 300);

   REQUIRE(ht.keyExists(k0));
   REQUIRE(ht.keyExists(k1));
   REQUIRE(ht.keyExists(k2));

   ht.remove(k0);
   REQUIRE_FALSE(ht.keyExists(k0));

   // Must still find keys that were inserted "after" the deleted slot in probe chain
   REQUIRE(ht.keyExists(k2));
   REQUIRE(ht.find(k2) == 300);

   // Also test inserting a new colliding key after deletion (reuse hole)
   auto more = find_colliding_strings(S, 4);
   REQUIRE(more.size() >= 4);
   const string &k3 = more[3];
   ht.insert(k3, 400);
   REQUIRE(ht.keyExists(k3));
   REQUIRE(ht.find(k3) == 400);
}

static void tombstone_checks_DH_strings() {
   const size_t S = 17;
   auto keys = find_colliding_strings(S, 3);
   REQUIRE(keys.size() >= 3);

   DHHashTable<string, int> ht(S);

   const string &k0 = keys[0];
   const string &k1 = keys[1];
   const string &k2 = keys[2];

   ht.insert(k0, 100);
   ht.insert(k1, 200);
   ht.insert(k2, 300);

   REQUIRE(ht.keyExists(k0));
   REQUIRE(ht.keyExists(k1));
   REQUIRE(ht.keyExists(k2));

   ht.remove(k0);
   REQUIRE_FALSE(ht.keyExists(k0));

   // For DH, search must not stop at a deleted-but-probed slot
   REQUIRE(ht.keyExists(k2));
   REQUIRE(ht.find(k2) == 300);

   // Inserting a new colliding key should still work
   auto more = find_colliding_strings(S, 4);
   REQUIRE(more.size() >= 4);
   const string &k3 = more[3];
   ht.insert(k3, 400);
   REQUIRE(ht.keyExists(k3));
   REQUIRE(ht.find(k3) == 400);
}

// ---------------------- SC Tests ----------------------

TEST_CASE("SC: basic insert/find/keyExists (textEasy)", "[SC][basic]") {
   basic_insert_find_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: remove easy + remove-missing safe", "[SC][remove]") {
   remove_easy_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: remove hard with update pattern", "[SC][remove][update]") {
   remove_hard_update_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: resize once (textLong) preserves values", "[SC][resize]") {
   resize_once_textLong_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: resize all (a..z) keeps all keys", "[SC][resize]") {
   resize_all_alpha_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: operator[] default insert and reference mutation", "[SC][brackets]") {
   bracket_operator_checks<SCHashTable<string, int>>();
}

TEST_CASE("SC: copy ctor and assignment are deep copies", "[SC][copy]") {
   copy_and_assignment_checks<SCHashTable<string, int>>();
}

// ---------------------- LP Tests ----------------------

TEST_CASE("LP: basic insert/find/keyExists (textEasy)", "[LP][basic]") {
   basic_insert_find_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: remove easy + remove-missing safe", "[LP][remove]") {
   remove_easy_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: remove hard with update pattern", "[LP][remove][update]") {
   remove_hard_update_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: resize once (textLong) preserves values", "[LP][resize]") {
   resize_once_textLong_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: resize all (a..z) keeps all keys", "[LP][resize]") {
   resize_all_alpha_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: operator[] default insert and reference mutation", "[LP][brackets]") {
   bracket_operator_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: iterator visits all keys (also after growth)", "[LP][iter]") {
   iterator_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: copy ctor and assignment are deep copies", "[LP][copy]") {
   copy_and_assignment_checks<LPHashTable<string, int>>();
}

TEST_CASE("LP: tombstones preserve search past deletions (colliding strings)", "[LP][tombstone]") {
   tombstone_checks_LP_strings();
}

// ---------------------- DH Tests ----------------------

TEST_CASE("DH: basic insert/find/keyExists (textEasy)", "[DH][basic]") {
   basic_insert_find_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: remove easy + remove-missing safe", "[DH][remove]") {
   remove_easy_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: remove hard with update pattern", "[DH][remove][update]") {
   remove_hard_update_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: resize once (textLong) preserves values", "[DH][resize]") {
   resize_once_textLong_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: resize all (a..z) keeps all keys", "[DH][resize]") {
   resize_all_alpha_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: operator[] default insert and reference mutation", "[DH][brackets]") {
   bracket_operator_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: iterator visits all keys (also after growth)", "[DH][iter]") {
   iterator_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: copy ctor and assignment are deep copies", "[DH][copy]") {
   copy_and_assignment_checks<DHHashTable<string, int>>();
}

TEST_CASE("DH: tombstones preserve search past deletions (colliding strings)", "[DH][tombstone]") {
   tombstone_checks_DH_strings();
}
