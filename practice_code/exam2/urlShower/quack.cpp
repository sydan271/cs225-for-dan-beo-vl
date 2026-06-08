#include "quack.h"
Quack::Quack() = default;
Quack::~Quack() = default;
void Quack::push(std::string item) { _data.insert(_data.begin(), item); }
std::string Quack::pop() {
   if (_data.size() == 0)
      return "";
   std::string s = _data[0];
   _data.erase(_data.begin());
   return s;
}
std::string Quack::pull() {
   if (_data.size() == 0)
      return "";
   std::string s = _data[_data.size() - 1];
   _data.pop_back(); // _data.erase(_data.end() - 1); also works
   return s;
}

size_t Quack::size() const { return _data.size(); }
bool Quack::empty() const { return _data.size() == 0; }
std::vector<std::string> Quack::toVector() const { return _data; }
