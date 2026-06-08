/* Your code here! */
#include "dsets.h"

DisjointSets::DisjointSets(int num)
    : data_(num, -1) {
}

void DisjointSets::addElements(int num) {
   if (num <= 0) return;
   data_.insert(data_.end(), num, -1);
}

int DisjointSets::find(int elem) {
   if (elem < 0 || elem >= static_cast<int>(data_.size())) return -1;
   if (data_[elem] < 0) return elem;

   data_[elem] = find(data_[elem]);

   return data_[elem];
}

void DisjointSets::setUnion(int a, int b) {
   if (a < 0 || b < 0 || a >= static_cast<int>(data_.size()) || b >= static_cast<int>(data_.size())) return;
   int root_a = find(a);
   int root_b = find(b);
   if (root_b == root_a) return;

   if (data_[root_a] <= data_[root_b]) {
      data_[root_a] += data_[root_b];
      data_[root_b] = root_a;
   } else {
      data_[root_b] += data_[root_a];
      data_[root_a] = root_b;
   }
}

int DisjointSets::size(int elem) {
   if (elem < 0 || elem >= static_cast<int>(data_.size())) return 0;
   return -data_[find(elem)];
}

int DisjointSets::getValue(int elem) const {
   if (elem < 0 || elem >= static_cast<int>(data_.size())) return -2;
   return data_[elem];
}
