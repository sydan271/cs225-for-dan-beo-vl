/* Your code here! */
#include "dsets.h"

void DisjointSets::addElements(int num) {
   container_.insert(container_.end(), num, -1);
}

int DisjointSets::find(int elem) {
   if (container_.size() == 0 || elem < 0 || elem >= (int)container_.size()) return -1;
   if (container_[elem] < 0) return elem;
   int root = find(container_[elem]);
   container_[elem] = root;
   return root;
}

void DisjointSets::setUnion(int a, int b) {
   if (a < 0 || b < 0 || a >= (int)container_.size() || b >= (int)container_.size()) return;
   int root1 = find(a);
   int root2 = find(b);
   if (root1 == root2) return;

   if (container_[root1] <= container_[root2]) {
      container_[root1] += container_[root2];
      container_[root2] = root1;
   }

   else {
      container_[root2] += container_[root1];
      container_[root1] = root2;
   }
}

int DisjointSets::size(int elem) {
   if (elem >= 0 && elem < (int)container_.size()) return -container_[find(elem)];
   return 0;
}

int DisjointSets::getValue(int elem) const {
   if (elem >= 0 && elem < (int)container_.size()) return container_[elem];
   return -2;
}
