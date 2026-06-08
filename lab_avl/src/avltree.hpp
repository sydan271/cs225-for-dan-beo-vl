/**
 * @file avltree.cpp
 * Definitions of the binary tree functions you'll be writing for this lab.
 * You'll need to modify this file.
 */
#pragma once
#include "avltree.h"

template <class K, class V>
V AVLTree<K, V>::find(const K &key) const {
   return find(root, key);
}

template <class K, class V>
V AVLTree<K, V>::find(Node *subtree, const K &key) const {
   if (subtree == NULL)
      return V();
   else if (key == subtree->key)
      return subtree->value;
   else {
      if (key < subtree->key)
         return find(subtree->left, key);
      else
         return find(subtree->right, key);
   }
}

template <class K, class V>
void AVLTree<K, V>::rotateLeft(Node *&t) {
   functionCalls.push_back("rotateLeft"); // Stores the rotation name (don't remove this)
                                          // your code here
   Node *temp = t;
   Node *child = t->right;
   Node *child_left = t->right->left;

   t = child;
   temp->right = child_left;
   t->left = temp;

   temp->height = std::max(heightOrNeg1(temp->left), heightOrNeg1(temp->right)) + 1;
   t->height = std::max(heightOrNeg1(t->left), heightOrNeg1(t->right)) + 1;
}

template <class K, class V>
void AVLTree<K, V>::rotateLeftRight(Node *&t) {
   functionCalls.push_back("rotateLeftRight"); // Stores the rotation name (don't remove this)
   // Implemented for you:
   rotateLeft(t->left);
   rotateRight(t);
}

template <class K, class V>
void AVLTree<K, V>::rotateRight(Node *&t) {
   functionCalls.push_back("rotateRight"); // Stores the rotation name (don't remove this)
                                           // your code here
   Node *temp = t;
   Node *child = t->left;
   Node *child_right = child->right;

   t = child;
   temp->left = child_right;
   t->right = temp;

   temp->height = std::max(heightOrNeg1(temp->left), heightOrNeg1(temp->right)) + 1;
   t->height = std::max(heightOrNeg1(t->left), heightOrNeg1(t->right)) + 1;
}

template <class K, class V>
void AVLTree<K, V>::rotateRightLeft(Node *&t) {
   functionCalls.push_back("rotateRightLeft"); // Stores the rotation name (don't remove this)
                                               // your code here
   rotateRight(t->right);
   rotateLeft(t);
}

template <class K, class V>
void AVLTree<K, V>::rebalance(Node *&subtree) {
   // your code here
   if (!subtree)
      return;
   int hb = heightOrNeg1(subtree->right) - heightOrNeg1(subtree->left);
   if (hb == 2) {
      int hbRight = heightOrNeg1(subtree->right->right) - heightOrNeg1(subtree->right->left);
      if (hbRight >= 0)
         rotateLeft(subtree);
      else
         rotateRightLeft(subtree);
   } else if (hb == -2) {
      int hbLeft = heightOrNeg1(subtree->left->right) - heightOrNeg1(subtree->left->left);
      if (hbLeft <= 0)
         rotateRight(subtree);
      else
         rotateLeftRight(subtree);
   }
   subtree->height = 1 + std::max(heightOrNeg1(subtree->left), heightOrNeg1(subtree->right));
}

template <class K, class V>
void AVLTree<K, V>::insert(const K &key, const V &value) {
   insert(root, key, value);
}

template <class K, class V>
void AVLTree<K, V>::insert(Node *&subtree, const K &key, const V &value) {
   // your code here
   if (subtree == nullptr) {
      subtree = new Node(key, value);
      return;
   }
   if (subtree->key == key) {
      subtree->value = value;
      return;
   }
   if (subtree->key > key)
      insert(subtree->left, key, value);
   else
      insert(subtree->right, key, value);

   rebalance(subtree);
}

template <class K, class V>
void AVLTree<K, V>::remove(const K &key) {
   remove(root, key);
}

template <class K, class V>
void AVLTree<K, V>::remove(Node *&subtree, const K &key) {
   if (subtree == NULL)
      return;

   if (key < subtree->key) {
      // your code here
      remove(subtree->left, key);
   } else if (key > subtree->key) {
      // your code here
      remove(subtree->right, key);
   } else {
      if (subtree->left == NULL && subtree->right == NULL) {
         /* no-child remove */
         // your code here
         delete subtree;
         subtree = nullptr;
      } else if (subtree->left != NULL && subtree->right != NULL) {
         /* two-child remove */
         // your code here
         Node *&IOP = findIOP(subtree);
         swap(IOP, subtree);
         remove(IOP, key);
      } else {
         /* one-child remove */
         // your code here
         Node *temp = subtree;
         if (subtree->left)
            subtree = subtree->left;
         else
            subtree = subtree->right;
         delete temp;
      }
      // your code here
   }
   rebalance(subtree);
}

template <class K, class V>
typename AVLTree<K, V>::Node *&AVLTree<K, V>::findIOP(Node *&subtree) {
   if (subtree == nullptr)
      return subtree;
   Node **cur = &(subtree->left);
   while (*cur && (*cur)->right)
      cur = &((*cur)->right);
   return *cur;
}
