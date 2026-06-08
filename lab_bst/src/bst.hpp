/**
 * @file bst.cpp
 * Definitions of the binary tree functions you'll be writing for this lab.
 * You'll need to modify this file.
 *
 */
#pragma once
#include "bst.h"
#include <algorithm>

template <class K, class V>
V BST<K, V>::find(const K &key) {
   // your code here
   Node *&node = find(root, key);
   if (!node) return V();
   return node->value;
}

template <class K, class V>
struct BST<K, V>::Node *&BST<K, V>::find(Node *&subtree, const K &key) {
   // Your code here
   if (subtree == nullptr) return subtree;
   if (subtree->key == key) return subtree;

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
   auto &node = find(subtree, key);
   if (node == nullptr) node = new Node(key, value);
}

template <class K, class V>
void BST<K, V>::swap(Node *&first, Node *&second) {
   // your code here
   if (first == second || !first || !second) return;
   K tmp1 = first->key;
   V tmp2 = first->value;

   first->key = second->key;
   first->value = second->value;

   second->key = tmp1;
   second->value = tmp2;
}

template <class K, class V>
void BST<K, V>::remove(const K &key) {
   // your code here
   remove(root, key);
}

template <class K, class V>
void BST<K, V>::remove(Node *&subtree, const K &key) {
   // your code here
   Node *&node = find(subtree, key);
   if (!node) return;
   if (!node->right && !node->left) {
      delete node;
      node = nullptr;
      return;
   }
   if (!node->right) {
      Node *tmp = node;
      node = node->left;
      delete tmp;
      return;
   }
   if (!node->left) {
      Node *tmp = node;
      node = node->right;
      delete tmp;
      return;
   }

   Node *&IOP = findIOP(node);
   swap(node, IOP);
   remove(IOP, key);
}

template <class K, class V>
struct BST<K, V>::Node *&BST<K, V>::findIOP(Node *&subroot) {
   if (!subroot) return subroot;
   Node **cur = &(subroot->left);
   while (*cur && (*cur)->right) {
      cur = &((*cur)->right);
   }
   return *cur;
}

template <class K, class V>
BST<K, V> listBuild(std::vector<std::pair<K, V>> inList) {
   // your code here
   BST<K, V> bst;
   if (inList.empty()) return bst;

   for (auto &[k, v] : inList) {
      bst.insert(k, v);
   }
   return bst;
}

template <class K, class V>
std::vector<int> allBuild(std::vector<std::pair<K, V>> inList) {
   // your code here
   std::vector<int> ret(inList.size(), 0);
   std::sort(inList.begin(), inList.end());
   do {
      BST<K, V> bst = listBuild(inList);
      ret[bst.height()]++;
   } while (std::next_permutation(inList.begin(), inList.end()));

   return ret;
}
