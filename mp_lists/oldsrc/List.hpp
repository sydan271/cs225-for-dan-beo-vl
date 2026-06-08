/**
 * @file list.cpp
 * Doubly Linked List (MP 3).
 */
#pragma once
#include "List.h"
#include <cstddef>

template <class T>
List<T>::List() {
   // @TODO: graded in mp_lists part 1
   head_ = NULL;
   tail_ = NULL;
   length_ = 0;
}

/**
 * Returns a ListIterator with a position at the beginning of
 * the List.
 */
template <typename T>
typename List<T>::ListIterator List<T>::begin() const {
   // @TODO: graded in mp_lists part 1
   return ListIterator(head_);
}

/**
 * Returns a ListIterator one past the end of the List.
 */
template <typename T>
typename List<T>::ListIterator List<T>::end() const {
   // @TODO: graded in mp_lists part 1
   return ListIterator(NULL);
}

/**
 * Destroys all dynamically allocated memory associated with the current
 * List class.
 */
template <typename T>
void List<T>::_destroy() {
   /// @todo Graded in mp_lists part 1
   while (head_ != NULL) {
      ListNode *temp = head_->next;
      delete head_;
      head_ = temp;
   }
}

/**
 * Inserts a new node at the front of the List.
 * This function **SHOULD** create a new ListNode.
 *
 * @param ndata The data to be inserted.
 */
template <typename T>
void List<T>::insertFront(T const &ndata) {
   /// @todo Graded in mp_lists part 1
   ListNode *newNode = new ListNode(ndata);
   newNode->next = head_;
   // newNode->prev = NULL; dont need cuz the ListNode ctor already set NULL

   if (head_ != NULL) {
      head_->prev = newNode;
   }
   if (tail_ == NULL) {
      tail_ = newNode;
   }

   head_ = newNode;
   length_++;
}

/**
 * Inserts a new node at the back of the List.
 * This function **SHOULD** create a new ListNode.
 *
 * @param ndata The data to be inserted.
 */
template <typename T>
void List<T>::insertBack(const T &ndata) {
   /// @todo Graded in mp_lists part 1
   ListNode *newN = new ListNode(ndata);
   newN->prev = tail_;

   if (tail_ != NULL) {
      tail_->next = newN;
   }
   if (head_ == NULL) head_ = newN;

   tail_ = newN;
   ++length_;
}

/**
 * Helper function to split a sequence of linked memory at the node
 * splitPoint steps **after** start. In other words, it should disconnect
 * the sequence of linked memory after the given number of nodes, and
 * return a pointer to the starting node of the new sequence of linked
 * memory.
 *
 * This function **SHOULD NOT** create **ANY** new List or ListNode objects!
 *
 * This function is also called by the public split() function located in
 * List-given.hpp
 *
 * @param start The node to start from.
 * @param splitPoint The number of steps to walk before splitting.
 * @return The starting node of the sequence that was split off.
 */
template <typename T>
typename List<T>::ListNode *List<T>::split(ListNode *start, int splitPoint) {
   /// @todo Graded in mp_lists part 1
   ListNode *curr = start;

   for (int i = 0; i < splitPoint && curr != NULL; i++) {
      curr = curr->next;
   }

   if (curr != NULL) {
      if (curr->prev != NULL) curr->prev->next = NULL;
      curr->prev = NULL;
   }

   return curr;
}

/**
 * Modifies the List using the waterfall algorithm.
 * Every other node (starting from the second one) is removed from the
 * List, but appended at the back, becoming the new tail. This continues
 * until the next thing to be removed is either the tail (**not necessarily
 * the original tail!**) or NULL.  You may **NOT** allocate new ListNodes.
 * Note that since the tail should be continuously updated, some nodes will
 * be moved more than once.
 */
template <typename T>
void List<T>::waterfall() {
   /// @todo Graded in part 1
   ListNode *curr = head_;
   while (curr != NULL && curr->next != NULL && curr->next != tail_) {
      ListNode *toMove = curr->next;
      curr->next = toMove->next;
      curr->next->prev = curr;
      toMove->next = NULL;
      toMove->prev = tail_;
      tail_->next = toMove;
      tail_ = toMove;

      curr = curr->next;
   }
}

/**
 * Reverses the current List.
 */
template <typename T>
void List<T>::reverse() {
   reverse(head_, tail_);
}

/**
 * Helper function to reverse a sequence of linked memory inside a List,
 * starting at startPoint and ending at endPoint. You are responsible for
 * updating startPoint and endPoint to point to the new starting and ending
 * points of the rearranged sequence of linked memory in question.
 *
 * @param startPoint A pointer reference to the first node in the sequence
 *  to be reversed.
 * @param endPoint A pointer reference to the last node in the sequence to
 *  be reversed.
 */
template <typename T>
void List<T>::reverse(ListNode *&startPoint, ListNode *&endPoint) {
   /// @todo Graded in mp_lists part 2
   if (startPoint == NULL || endPoint == NULL) return;
   if (startPoint == endPoint) return;

   ListNode *before = startPoint->prev;
   ListNode *after = endPoint->next;

   ListNode *curr = startPoint;
   while (curr != after) {
      ListNode *temp = curr->next;
      curr->next = curr->prev;
      curr->prev = temp;
      curr = temp;
   }

   curr = startPoint; // curr acts as the *p to original startPoint now (im not
                      // making new var for efficiency)
   startPoint = endPoint;
   endPoint = curr;

   if (before) before->next = startPoint;
   else head_ = startPoint;
   startPoint->prev = before;

   if (after) after->prev = endPoint;
   else tail_ = endPoint;
   endPoint->next = after;
}

/**
 * Reverses blocks of size n in the current List. You should use your
 * reverse( ListNode * &, ListNode * & ) helper function in this method!
 *
 * @param n The size of the blocks in the List to be reversed.
 */
template <typename T>
void List<T>::reverseNth(int n) {
   /// @todo Graded in mp_lists part 2
   int iter = (length_ + n - 1) / n;
   ListNode *start = head_;
   ListNode *end = head_;
   for (int i = 0; i < iter - 1; i++) {
      for (int j = 0; j < n - 1; j++) {
         if (end) end = end->next;
      }
      reverse(start, end);
      if (end) end = end->next;
      start = end;
   }

   reverse(start, tail_);
}

/**
 * Merges the given sorted list into the current sorted list.
 *
 * @param otherList List to be merged into the current list.
 */
template <typename T>
void List<T>::mergeWith(List<T> &otherList) {
   // set up the current list
   head_ = merge(head_, otherList.head_);
   tail_ = head_;

   // make sure there is a node in the new list
   if (tail_ != NULL) {
      while (tail_->next != NULL)
         tail_ = tail_->next;
   }
   length_ = length_ + otherList.length_;

   // empty out the parameter list
   otherList.head_ = NULL;
   otherList.tail_ = NULL;
   otherList.length_ = 0;
}

/**
 * Helper function to merge two **sorted** and **independent** sequences of
 * linked memory. The result should be a single sequence that is itself
 * sorted.
 *
 * This function **SHOULD NOT** create **ANY** new List objects.
 *
 * @param first The starting node of the first sequence.
 * @param second The starting node of the second sequence.
 * @return The starting node of the resulting, sorted sequence.
 */
template <typename T>
typename List<T>::ListNode *List<T>::merge(ListNode *first, ListNode *second) {
   /// @todo Graded in mp_lists part 2
   if (first == nullptr) return second;
   if (second == nullptr) return first;
   if (first == second) return first;

   ListNode *curr = (first->data < second->data) ? first : second;
   ListNode *head = curr;

   if (curr == first) first = first->next;
   else second = second->next;

   while (first != nullptr || second != nullptr) {
      if (second == nullptr) {
         curr->next = first;
         first->prev = curr;
         first = first->next;
      } else if (first == nullptr) {
         curr->next = second;
         second->prev = curr;
         second = second->next;
      } else if (first->data < second->data) {
         curr->next = first;
         first->prev = curr;
         first = first->next;
      } else if (!(first->data < second->data)) {
         curr->next = second;
         second->prev = curr;
         second = second->next;
      }
      curr = curr->next;
   }

   curr->next = nullptr;
   head->prev = nullptr;

   return head;
}

/**
 * Sorts a chain of linked memory given a start node and a size.
 * This is the recursive helper for the Mergesort algorithm (i.e., this is
 * the divide-and-conquer step).
 *
 * Called by the public sort function in List-given.hpp
 *
 * @param start Starting point of the chain.
 * @param chainLength Size of the chain to be sorted.
 * @return A pointer to the beginning of the now sorted chain.
 */
template <typename T>
typename List<T>::ListNode *List<T>::mergesort(ListNode *start, int chainLength) {
   if (chainLength == 1) return start;

   int first_len = (chainLength + 1) / 2;
   ListNode *second_half = split(start, first_len);
   /// @todo Graded in mp_lists part 2
   return merge(mergesort(start, first_len), mergesort(second_half, chainLength - first_len));
}
