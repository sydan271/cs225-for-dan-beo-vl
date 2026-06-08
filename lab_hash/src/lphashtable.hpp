/**
 * @file lphashtable.cpp
 * Implementation of the LPHashTable class.
 */
#pragma once
#include "hashes.h"
#include "lphashtable.h"

template <class K, class V>
LPHashTable<K, V>::LPHashTable(size_t tsize) {
   if (tsize <= 0)
      tsize = 17;
   size = findPrime(tsize);
   table = new std::pair<K, V> *[size];
   should_probe = new bool[size];
   for (size_t i = 0; i < size; i++) {
      table[i] = NULL;
      should_probe[i] = false;
   }
   elems = 0;
}

template <class K, class V>
LPHashTable<K, V>::~LPHashTable() {
   for (size_t i = 0; i < size; i++)
      delete table[i];
   delete[] table;
   delete[] should_probe;
}

template <class K, class V>
LPHashTable<K, V> const &LPHashTable<K, V>::operator=(LPHashTable const &rhs) {
   if (this != &rhs) {
      for (size_t i = 0; i < size; i++)
         delete table[i];
      delete[] table;
      delete[] should_probe;

      table = new std::pair<K, V> *[rhs.size];
      should_probe = new bool[rhs.size];
      for (size_t i = 0; i < rhs.size; i++) {
         should_probe[i] = rhs.should_probe[i];
         if (rhs.table[i] == NULL)
            table[i] = NULL;
         else
            table[i] = new std::pair<K, V>(*(rhs.table[i]));
      }
      size = rhs.size;
      elems = rhs.elems;
   }
   return *this;
}

template <class K, class V>
LPHashTable<K, V>::LPHashTable(LPHashTable<K, V> const &other) {
   table = new std::pair<K, V> *[other.size];
   should_probe = new bool[other.size];
   for (size_t i = 0; i < other.size; i++) {
      should_probe[i] = other.should_probe[i];
      if (other.table[i] == NULL)
         table[i] = NULL;
      else
         table[i] = new std::pair<K, V>(*(other.table[i]));
   }
   size = other.size;
   elems = other.elems;
}

template <class K, class V>
void LPHashTable<K, V>::insert(K const &key, V const &value) {

   /**
    * @todo Implement this function.
    *
    * @note Remember to resize the table when necessary (load factor >= 0.7).
    * **Do this check *after* increasing elems (but before inserting)!!**
    * Also, don't forget to mark the cell for probing with should_probe!
    */

   size_t idx = hashes::hash(key, size);
   size_t offset = 0;

   while (table[(idx + offset) % size])
      ++offset;

   table[(idx + offset) % size] = new std::pair<K, V>(key, value);
   should_probe[(idx + offset) % size] = true;
   ++elems;

   if (shouldResize())
      resizeTable();
}

template <class K, class V>
void LPHashTable<K, V>::remove(K const &key) {
   /**
    * @todo: implement this function
    */
   int idx = findIndex(key);
   if (idx == -1)
      return;

   delete table[idx];
   table[idx] = nullptr;
   --elems;
}

template <class K, class V>
int LPHashTable<K, V>::findIndex(const K &key) const {

   /**
    * @todo Implement this function
    *
    * Be careful in determining when the key is not in the table!
    */
   size_t idx = hashes::hash(key, size);
   size_t offset = 0;

   size_t curIdx = (idx + offset) % size;

   while (should_probe[curIdx]) {
      if (table[curIdx] && table[curIdx]->first == key)
         return curIdx;

      ++offset;
      curIdx = (idx + offset) % size;
   }

   return -1;
}

template <class K, class V>
V LPHashTable<K, V>::find(K const &key) const {
   int idx = findIndex(key);
   if (idx != -1)
      return table[idx]->second;
   return V();
}

template <class K, class V>
V &LPHashTable<K, V>::operator[](K const &key) {
   // First, attempt to find the key and return its value by reference
   int idx = findIndex(key);
   if (idx == -1) {
      // otherwise, insert the default value and return it
      insert(key, V());
      idx = findIndex(key);
   }
   return table[idx]->second;
}

template <class K, class V>
bool LPHashTable<K, V>::keyExists(K const &key) const {
   return findIndex(key) != -1;
}

template <class K, class V>
void LPHashTable<K, V>::clear() {
   for (size_t i = 0; i < size; i++)
      delete table[i];
   delete[] table;
   delete[] should_probe;
   table = new std::pair<K, V> *[17];
   should_probe = new bool[17];
   for (size_t i = 0; i < 17; i++)
      should_probe[i] = false;
   size = 17;
   elems = 0;
}

template <class K, class V>
void LPHashTable<K, V>::resizeTable() {

   /**
    * @todo Implement this function
    *
    * The size of the table should be the closest prime to size * 2.
    *
    * @hint Use findPrime()!
    */
   size_t newSize = findPrime(size * 2);
   std::pair<K, V> **tmp = new std::pair<K, V> *[newSize]();
   bool *tmp_probe = new bool[newSize]();

   size_t idx = 0;
   size_t offset = 0;
   size_t curIdx = 0;

   for (size_t i = 0; i < size; ++i) {
      std::pair<K, V> *pair = table[i];
      if (pair) {
         offset = 0;
         idx = hashes::hash(pair->first, newSize);
         curIdx = (idx + offset) % newSize;
         while (tmp[curIdx]) {
            ++offset;
            curIdx = (idx + offset) % newSize;
         }
         tmp[curIdx] = pair;
         tmp_probe[curIdx] = true;
      }
   }

   delete[] should_probe;
   delete[] table;

   table = tmp;
   should_probe = tmp_probe;
   size = newSize;
}
