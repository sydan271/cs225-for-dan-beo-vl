#pragma once
#include "List.h"

template <typename T>
class List<T>::ListIterator {
private:
   // @TODO: graded in mp_lists part 1
   ListNode *position_;

public:
   using difference_type = std::ptrdiff_t;
   using value_type = T;
   using pointer = T *;
   using reference = T &;
   using iterator_category = std::bidirectional_iterator_tag;

   ListIterator()
       : position_(NULL) {
   }

   ListIterator(ListNode *x)
       : position_(x) {
   }

   // Pre-Increment, ++iter
   ListIterator &operator++() {
      // @TODO: graded in mp_lists part 1
      if (position_) position_ = position_->next;
      return *this;
   }

   // Post-Increment, iter++
   ListIterator operator++(int) {
      // @TODO: graded in mp_lists part 1
      ListNode *temp = position_;
      if (position_) position_ = position_->next;
      return ListIterator(temp);
   }

   // Pre-Decrement, --iter
   ListIterator &operator--() {
      // @TODO: graded in mp_lists part 1
      if (position_) position_ = position_->prev;
      return *this;
   }

   // Post-Decrement, iter--
   ListIterator operator--(int) {
      // @TODO: graded in mp_lists part 1
      ListNode *temp = position_;
      if (position_) position_ = position_->prev;
      return ListIterator(temp);
   }

   bool operator!=(const ListIterator &rhs) const {
      // @TODO: graded in mp_lists part 1
      return position_ != rhs.position_;
   }

   bool operator==(const ListIterator &rhs) const {
      return !(*this != rhs);
   }

   const T &operator*() {
      return position_->data;
   }

   const T *operator->() {
      return &(position_->data);
   }
};
