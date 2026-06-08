/**
 * @file more.cpp
 * Comprehensive BLACK-BOX tests for CS225 lab_huffman.
 *
 * Important constraints (based on your headers):
 *  - HuffmanTree::TreeNode is private -> tests MUST NOT reference it
 *  - HuffmanTree::getBitsForChar is private -> tests MUST NOT call it
 *  - No std::set or std::map in this test file
 *
 * What we *can* test comprehensively using only public surface:
 *  1) removeSmallest behavior via HuffmanTree::testRemoveSmallest (public)
 *     - empty-single / empty-merge
 *     - smaller head selection
 *     - tie-breaking rule (prefer singleQueue on equal freq)
 *     - multiple mixed cases
 *
 *  2) Encoder/Decoder correctness end-to-end (public)
 *     - encodeFile -> decodeFile round-trip equals original
 *     - many edge distributions: single-char, two-char (balanced/unbalanced),
 *       skewed powers-of-two, many distinct bytes, includes '\n' and spaces
 *     - stability under varying input order (same multiset of chars but shuffled)
 *
 *  3) Tree serialization round-trip (public)
 *     - encoder writes tree.huff
 *     - HuffmanTree(BinaryFileReader&) reads that tree
 *     - decoding test.bin using that HuffmanTree matches original
 *
 *  4) Optional determinism checks vs provided solution binaries (if present)
 *     - compare produced test.bin/tree.huff with ../data/soln_*
 *
 * Notes:
 *  - This file purposely avoids any direct inspection of the Huffman codes,
 *    since code accessors are private in your version.
 */

#include <catch2/catch_test_macros.hpp>

#include "binary_file_reader.h"
#include "decoder.h"
#include "encoder.h"
#include "frequency.h"
#include "huffman_tree.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/*******************************************************
 * Helpers: file I/O
 *******************************************************/

static void writeTextFile(const string &path, const string &content) {
   ofstream out(path, ios::binary);
   REQUIRE(out.good());
   out << content;
}

static string readTextFile(const string &path) {
   ifstream in(path, ios::binary);
   REQUIRE(in.good());
   stringstream ss;
   ss << in.rdbuf();
   return ss.str();
}

static string readBinaryFileBytes(const string &path) {
   ifstream in(path, ios::binary);
   REQUIRE(in.good());
   stringstream ss;
   ss << in.rdbuf();
   return ss.str();
}

static void compareBinaryFilesOrFail(const string &aPath, const string &bPath) {
   REQUIRE(readBinaryFileBytes(aPath) == readBinaryFileBytes(bPath));
}

/*******************************************************
 * Helpers: generate text datasets
 *******************************************************/

static void duplicateChar(char c, int freq, string &s) {
   for (int i = 0; i < freq; i++)
      s.push_back(c);
}

static string makeDataset_basic1() {
   // a20 b30 c60 d40
   string s;
   duplicateChar('a', 20, s);
   duplicateChar('b', 30, s);
   duplicateChar('c', 60, s);
   duplicateChar('d', 40, s);
   return s;
}

static string makeDataset_basic2() {
   // a5 b6 c12 d13 e7 f8
   string s;
   duplicateChar('a', 5, s);
   duplicateChar('b', 6, s);
   duplicateChar('c', 12, s);
   duplicateChar('d', 13, s);
   duplicateChar('e', 7, s);
   duplicateChar('f', 8, s);
   return s;
}

static string makeDataset_powersOfTwo(int start) {
   // a..e doubling counts + newline
   string s;
   for (char c = 'a'; c <= 'e'; ++c) {
      duplicateChar(c, start, s);
      start *= 2;
   }
   s.push_back('\n');
   return s;
}

static string makeDataset_singleChar(int n) { return string(n, 'x'); }

static string makeDataset_twoChars(int aCount, int bCount) {
   string s;
   duplicateChar('A', aCount, s);
   duplicateChar('B', bCount, s);
   return s;
}

static string makeDataset_manyAscii() {
   // deterministic: ASCII 32..126 with varying counts + newlines/spaces
   string s;
   for (int ch = 32; ch <= 126; ch++) {
      int reps = (ch % 5) + 1;
      for (int k = 0; k < reps; k++)
         s.push_back(static_cast<char>(ch));
   }
   s += "\nline two\nline three\n";
   return s;
}

static string makeDataset_allBytesSmall() {
   // include bytes 1..127 exactly once, plus a few repeats, plus '\n'
   string s;
   for (int ch = 1; ch < 128; ch++)
      s.push_back(static_cast<char>(ch));
   s.push_back('\n');
   duplicateChar('Z', 20, s);
   duplicateChar('\0', 3, s); // include NUL in the content if pipeline supports it
   return s;
}

/*******************************************************
 * Helpers: round-trip expectations
 *******************************************************/

static void roundTripEncodeDecodeExpectEqual(const string &content) {
   writeTextFile("text.txt", content);
   encoder::encodeFile("text.txt", "test.bin", "tree.huff");
   decoder::decodeFile("test.bin", "tree.huff", "out.txt");
   REQUIRE(readTextFile("out.txt") == content);
}

/**
 * Stronger: decode test.bin using HuffmanTree built from the serialized tree file,
 * without calling decoder::decodeFile (so you exercise HuffmanTree(BinaryFileReader) + decodeFile).
 */
static void roundTripViaTreeReaderExpectEqual(const string &content) {
   writeTextFile("text.txt", content);
   encoder::encodeFile("text.txt", "test.bin", "tree.huff");

   BinaryFileReader treeReader("tree.huff");
   HuffmanTree fromTree(treeReader);

   BinaryFileReader bitsReader("test.bin");
   string decoded = fromTree.decodeFile(bitsReader);

   REQUIRE(decoded == content);
}

/*******************************************************
 * Helpers: build Frequency vectors WITHOUT map/set
 *******************************************************/

static vector<Frequency> freqsFromString_noMap(const string &s) {
   array<int, 256> cnt{};
   cnt.fill(0);
   for (unsigned char ch : s)
      cnt[ch]++;

   vector<Frequency> freqs;
   freqs.reserve(256);
   for (int i = 0; i < 256; i++) {
      if (cnt[i] > 0)
         freqs.push_back(Frequency(static_cast<char>(i), cnt[i]));
   }

   // NOTE: HuffmanTree constructor will stable_sort internally in your code,
   // but removeSmallest tests treat inputs as already-sorted (like the lab intends).
   sort(freqs.begin(), freqs.end());
   return freqs;
}

/*******************************************************
 * 1) removeSmallest: comprehensive via public testRemoveSmallest
 *******************************************************/

TEST_CASE("removeSmallest: merge empty takes from single", "[huffman][removeSmallest]") {
   vector<Frequency> single = {Frequency('a', 1), Frequency('b', 2), Frequency('c', 3)};
   vector<Frequency> merge;
   Frequency got = HuffmanTree::testRemoveSmallest(single, merge);
   REQUIRE(got == single[0]);
}

TEST_CASE("removeSmallest: single empty takes from merge", "[huffman][removeSmallest]") {
   vector<Frequency> single;
   vector<Frequency> merge = {Frequency('a', 1), Frequency('b', 2), Frequency('c', 3)};
   Frequency got = HuffmanTree::testRemoveSmallest(single, merge);
   REQUIRE(got == merge[0]);
}

TEST_CASE("removeSmallest: smaller head wins (single smaller)", "[huffman][removeSmallest]") {
   vector<Frequency> single = {Frequency('s', 3)};
   vector<Frequency> merge = {Frequency('m', 5)};
   Frequency got = HuffmanTree::testRemoveSmallest(single, merge);
   REQUIRE(got == single[0]);
}

TEST_CASE("removeSmallest: smaller head wins (merge smaller)", "[huffman][removeSmallest]") {
   vector<Frequency> single = {Frequency('s', 7)};
   vector<Frequency> merge = {Frequency('m', 2)};
   Frequency got = HuffmanTree::testRemoveSmallest(single, merge);
   REQUIRE(got == merge[0]);
}

TEST_CASE("removeSmallest: tie-breaking prefers singleQueue head", "[huffman][removeSmallest]") {
   vector<Frequency> single = {Frequency('x', 10)};
   vector<Frequency> merge = {Frequency('y', 10)};
   Frequency got = HuffmanTree::testRemoveSmallest(single, merge);
   REQUIRE(got == single[0]);
}

TEST_CASE("removeSmallest: multiple mixed cases", "[huffman][removeSmallest]") {
   // single: 2,4,6,8,10 ; merge: 1,3,5,7,9
   vector<Frequency> single, merge;
   for (int i = 1; i <= 5; i++) {
      single.push_back(Frequency('a' + i, 2 * i));
      merge.push_back(Frequency('k' + i, 2 * i - 1));
   }

   // head is merge (1)
   REQUIRE(HuffmanTree::testRemoveSmallest(single, merge) == merge[0]);

   // make merge head bigger than single head
   merge[0] = Frequency('z', 100);
   sort(merge.begin(), merge.end());
   REQUIRE(HuffmanTree::testRemoveSmallest(single, merge) == single[0]);

   // force tie at head
   vector<Frequency> single2 = {Frequency('p', 5)};
   vector<Frequency> merge2 = {Frequency('q', 5)};
   REQUIRE(HuffmanTree::testRemoveSmallest(single2, merge2) == single2[0]);
}

/*******************************************************
 * 2) Encoder/Decoder end-to-end round-trip tests
 *******************************************************/

TEST_CASE("round-trip: basic1 dataset", "[huffman][roundtrip]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_basic1());
}

TEST_CASE("round-trip: basic2 dataset", "[huffman][roundtrip]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_basic2());
}

TEST_CASE("round-trip: powers-of-two skew + newline", "[huffman][roundtrip]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_powersOfTwo(2));
}

TEST_CASE("round-trip: single unique character", "[huffman][corner]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_singleChar(300));
}

TEST_CASE("round-trip: two characters balanced", "[huffman][corner]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_twoChars(200, 200));
}

TEST_CASE("round-trip: two characters very unbalanced", "[huffman][corner]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_twoChars(1, 500));
}

TEST_CASE("round-trip: many ASCII characters + whitespace", "[huffman][roundtrip]") {
   roundTripEncodeDecodeExpectEqual(makeDataset_manyAscii());
}

/*******************************************************
 * 3) Serialization round-trip (tree.huff -> HuffmanTree -> decodeFile)
 *******************************************************/

TEST_CASE("serialize: HuffmanTree(tree.huff) decodes test.bin correctly (basic2)",
          "[huffman][serialize]") {
   roundTripViaTreeReaderExpectEqual(makeDataset_basic2());
}

TEST_CASE("serialize: HuffmanTree(tree.huff) decodes test.bin correctly (skew + newline)",
          "[huffman][serialize]") {
   roundTripViaTreeReaderExpectEqual(makeDataset_powersOfTwo(2));
}

TEST_CASE("serialize: HuffmanTree(tree.huff) decodes test.bin correctly (single-char)",
          "[huffman][serialize]") {
   roundTripViaTreeReaderExpectEqual(makeDataset_singleChar(120));
}

/*******************************************************
 * 4) Invariance under permutation: same multiset of chars -> decode must still match original
 *
 * This does NOT assert deterministic binaries, only correctness.
 *******************************************************/

TEST_CASE("invariance: shuffled content still round-trips", "[huffman][roundtrip]") {
   string data = makeDataset_basic2();
   string shuffled = data;
   // deterministic shuffle: reverse then rotate
   reverse(shuffled.begin(), shuffled.end());
   rotate(shuffled.begin(), shuffled.begin() + (shuffled.size() / 3), shuffled.end());
   roundTripEncodeDecodeExpectEqual(shuffled);
}

/*******************************************************
 * 5) Optional determinism checks vs provided solution binaries
 * Keep only if you have ../data/soln_* files. Remove otherwise.
 *******************************************************/

TEST_CASE("determinism: basic1 matches solution binaries", "[huffman][soln][weight=1]") {
   string data = makeDataset_basic1();
   writeTextFile("text.txt", data);
   encoder::encodeFile("text.txt", "test.bin", "tree.huff");
   compareBinaryFilesOrFail("test.bin", "../data/soln_test.bin");
   compareBinaryFilesOrFail("tree.huff", "../data/soln_tree.huff");
}

TEST_CASE("determinism: basic2 matches solution binaries", "[huffman][soln][weight=1]") {
   string data = makeDataset_basic2();
   writeTextFile("text.txt", data);
   encoder::encodeFile("text.txt", "test.bin", "tree.huff");
   compareBinaryFilesOrFail("test.bin", "../data/soln_test2.bin");
   compareBinaryFilesOrFail("tree.huff", "../data/soln_tree2.huff");
}

TEST_CASE("determinism: powers-of-two dataset matches solution binaries",
          "[huffman][soln][weight=5][valgrind]") {
   string data = makeDataset_powersOfTwo(2);
   writeTextFile("text.txt", data);
   encoder::encodeFile("text.txt", "test.bin", "tree.huff");
   decoder::decodeFile("test.bin", "tree.huff", "out.txt");
   REQUIRE(readTextFile("out.txt") == data);
   compareBinaryFilesOrFail("tree.huff", "../data/soln_tree_2.huff");
   compareBinaryFilesOrFail("test.bin", "../data/soln_test_2.bin");
}
