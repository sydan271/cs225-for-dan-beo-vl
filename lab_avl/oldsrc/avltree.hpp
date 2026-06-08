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
   if (!t)
      return;
   Node *temp = t;
   t = t->right;

   // not necessary cuz rotateleft should only be called on right heavy tree which means
   // there are nodes on the right.
   temp->right = t->left;
   t->left = temp;

   temp->height = 1 + std::max(heightOrNeg1(temp->left), heightOrNeg1(temp->right));
   t->height = 1 + std::max(heightOrNeg1(t->left), heightOrNeg1(t->right));
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
   if (!t)
      return;

   Node *temp = t;
   t = t->left;
   temp->left = t->right;
   t->right = temp;

   temp->height = 1 + std::max(heightOrNeg1(temp->left), heightOrNeg1(temp->right));
   t->height = 1 + std::max(heightOrNeg1(t->left), heightOrNeg1(t->right));
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

   int l_height = heightOrNeg1(subtree->left);
   int r_height = heightOrNeg1(subtree->right);
   int hb = l_height - r_height;

   if (hb >= 2) {
      // subtree->left is guaranteed to exist here
      int sub_hb = heightOrNeg1(subtree->left->left) - heightOrNeg1(subtree->left->right);
      if (sub_hb >= 0)
         rotateRight(subtree);
      else
         rotateLeftRight(subtree);
   } else if (hb <= -2) {
      int sub_hb = heightOrNeg1(subtree->right->left) - heightOrNeg1(subtree->right->right);
      if (sub_hb <= 0)
         rotateLeft(subtree);
      else
         rotateRightLeft(subtree);
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
   if (!subtree) {
      subtree = new Node(key, value);
      return;
   }
   if (key == subtree->key) {
      subtree->value = value;
      return;
   } else if (key < subtree->key)
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

         if (IOP->left == NULL && IOP->right == NULL) {
            /* no-child remove */
            delete IOP;
            IOP = nullptr;
         } else {
            /* one-child remove */
            if (IOP->left) {
               Node *temp = IOP;
               IOP = IOP->left;
               delete temp;
               temp = nullptr;
            } else if (IOP->right) {
               Node *temp = IOP;
               IOP = IOP->right;
               delete temp;
               temp = nullptr;
            }
         }
      } else {
         /* one-child remove */
         // your code here
         if (subtree->left) {
            Node *temp = subtree;
            subtree = subtree->left;
            delete temp;
            temp = nullptr;
         } else if (subtree->right) {
            Node *temp = subtree;
            subtree = subtree->right;
            delete temp;
            temp = nullptr;
         }
      }
      // your code here
   }
   rebalance(subtree);
}

template <class K, class V>
typename AVLTree<K, V>::Node *&AVLTree<K, V>::findIOP(Node *&subRoot) {
   if (!subRoot)
      return subRoot;

   Node **cur = &(subRoot->left);
   while (*cur && (*cur)->right)
      cur = &((*cur)->right);
   return *cur;
}
