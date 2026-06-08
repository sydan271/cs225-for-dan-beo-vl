#pragma once
#include "bst.h"
#include <algorithm>

/**
 * @file bst.cpp
 * Definitions of the binary tree functions you'll be writing for this lab.
 * You'll need to modify this file.
 */

template <class K, class V>
V BST<K, V>::find(const K &key) {
   // your code here
   Node *&ideal = find(root, key);
   if (!ideal) return V();
   return ideal->value;
}

template <class K, class V>
struct BST<K, V>::Node *&BST<K, V>::find(Node *&subtree, const K &key) {
   // Your code here
   if (subtree == nullptr || subtree->key == key) return subtree;
   if (subtree->key < key) return find(subtree->right, key);
   return find(subtree->left, key);
}

template <class K, class V>
void BST<K, V>::insert(const K &key, const V &value) {
   // your code here
   insert(root, key, value);
}

template <class K, class V>
void BST<K, V>::insert(Node *&subtree, const K &key, const V &value) {
   // your code here
   Node *&ideal = find(subtree, key);
   if (ideal == nullptr) ideal = new Node(key, value);
}

template <class K, class V>
void BST<K, V>::swap(Node *&first, Node *&second) {
   // your code here
   if (first == second) return;
   if (first == nullptr || second == nullptr) return;
   K key = first->key;
   V value = first->value;
   first->key = second->key;
   first->value = second->value;
   second->key = key;
   second->value = value;
}

template <class K, class V>
typename BST<K, V>::Node *&BST<K, V>::findIOP(Node *&subtree) {
   if (subtree == nullptr) return subtree;
   Node **curr = &(subtree->left);
   while (*curr != nullptr && (*curr)->right != nullptr)
      curr = &((*curr)->right);
   return *curr;
}

template <class K, class V>
void BST<K, V>::remove(const K &key) {
   remove(root, key);
}

template <class K, class V>
void BST<K, V>::remove(Node *&subtree, const K &key) {
   // your code here
   Node *&ideal = find(subtree, key);
   if (ideal == nullptr) return;
   if (ideal->left == nullptr && ideal->right == nullptr) {
      delete ideal;
      ideal = nullptr;
      return;
   }
   if (ideal->left == nullptr) {
      Node *temp = ideal;
      ideal = ideal->right;
      delete temp;
      return;
   }
   if (ideal->right == nullptr) {
      Node *temp = ideal;
      ideal = ideal->left;
      delete temp;
      return;
   }

   // IOP is guaranteed to not be nullptr because ideal must have 2 children here
   Node *&IOP = findIOP(ideal);
   swap(ideal, IOP);

   if (IOP->left != nullptr) {
      Node *temp = IOP;
      IOP = IOP->left;
      delete temp;
      return;
   }

   delete IOP;
   IOP = nullptr;
}

template <class K, class V>
BST<K, V> listBuild(std::vector<std::pair<K, V>> inList) {
   // your code here
   BST<K, V> bst;
   for (const auto &[key, value] : inList) {
      bst.insert(key, value);
   }
   return bst;
}

template <class K, class V>
std::vector<int> allBuild(std::vector<std::pair<K, V>> inList) {
   // your code here
   std::vector<int> count(inList.size(), 0);
   std::sort(inList.begin(), inList.end());

   do {
      ++count[listBuild(inList).height()];
   } while (std::next_permutation(inList.begin(), inList.end()));

   return count;
}
