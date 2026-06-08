/**
 * @file no_ordered.h
 * Masks std set and map with error messages to prevent students from
 * using the STL's ordered data structures.
 * @date Summer 2025
 */

#pragma once

#include <map>
#include <set>

namespace std {
template <typename K, typename V>
class map<K, V> {
};
template <typename V>
class set<V> {
};
} // namespace std
