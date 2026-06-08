
#include "exam.h"
#include "quack.h"

vector<string> processUrls(vector<string> input, size_t k) {
   Quack q;
   for (const std::string &url : input) {
      if (url == "BACK") {
         q.pop();
      } else {
         q.push(url);
      }
   }
   while (q.size() > k)
      q.pull();
   return q.toVector();
}
