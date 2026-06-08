#pragma once

#include <list>
#include <string>
#include <vector>

class Quack {
 public:
   Quack();
   ~Quack();
   void push(std::string item);
   std::string pop();
   std::string pull();
   size_t size() const;
   bool empty() const;
   std::vector<std::string> toVector() const;

 private:
   std::vector<std::string> _data;
};
