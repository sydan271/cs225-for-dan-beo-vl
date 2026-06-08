/**
 * @file puzzle.cpp
 * Implementation of puzzle class.
 */
#include "puzzle.h"
#include <cstdint>
#include <functional>
#include <queue>
#include <set>

uint64_t PuzzleState::getTile(unsigned idx) const {
   if (idx >= 16)
      std::cerr << "Get tile outside range: " << 4 * idx << " bits from start of data_\n";
   return (data_ >> (4 * idx)) & 15;
}

static int parity4x4(const PuzzleState &s) {
   auto a = s.asArray();

   int inv = 0;
   int zeroIdx = -1;

   for (int i = 0; i < 16; i++) {
      if (a[i] == 0) zeroIdx = i;
   }

   for (int i = 0; i < 16; i++) {
      if (a[i] == 0) continue;
      for (int j = i + 1; j < 16; j++) {
         if (a[j] == 0) continue;
         if (a[i] > a[j]) inv++;
      }
   }

   int rowFromBottom = 4 - (zeroIdx / 4); // 1..4
   return (inv + rowFromBottom) & 1;
}

static bool isSolvable(const PuzzleState &start, const PuzzleState &goal) {
   return parity4x4(start) == parity4x4(goal);
}

PuzzleState::PuzzleState()
    : data_(0) {
   for (uint64_t i = 1; i < 16; ++i) {
      data_ |= (i << (4 * (i - 1)));
   }
}

PuzzleState::PuzzleState(const std::array<char, 16> state)
    : data_(0) {
   for (unsigned i = 0; i < 16; ++i) {
      uint64_t tile = state[i];
      data_ |= (tile << (4 * i));
   }
}

bool PuzzleState::operator<(const PuzzleState &rhs) const {
   for (unsigned i = 0; i < 16; ++i) {
      uint64_t tile = getTile(i);
      uint64_t tile2 = rhs.getTile(i);
      if (tile < tile2) return true;
      else if (tile > tile2) return false;
   }
   return false;
}

std::array<char, 16> PuzzleState::asArray() const {
   std::array<char, 16> ret;
   for (unsigned i = 0; i < 16; ++i) {
      ret[i] = getTile(i);
   }
   return ret;
}

void PuzzleState::getMap(std::map<char, unsigned> &tileDict) const {
   for (unsigned i = 0; i < 16; ++i) {
      tileDict.insert({static_cast<char>(getTile(i)), i});
   }
   if (tileDict.size() != 16) std::cerr << "Error when creating dict of chars\n";
}

int PuzzleState::manhattanDistance(const PuzzleState desiredState) const {
   int distance = 0;

   std::map<char, unsigned> curTileDict;
   getMap(curTileDict);
   std::map<char, unsigned> wantedTileDict;
   desiredState.getMap(wantedTileDict);

   for (unsigned i = 1; i < 16; ++i) {
      unsigned curPos = curTileDict[static_cast<char>(i)];
      unsigned desiredPos = wantedTileDict[static_cast<char>(i)];

      int curX = curPos % 4;
      int curY = curPos / 4;

      int wantedX = desiredPos % 4;
      int wantedY = desiredPos / 4;

      distance += std::abs(curX - wantedX) + std::abs(curY - wantedY);
   }

   return distance;
}

bool PuzzleState::operator==(const PuzzleState &rhs) const {
   return data_ == rhs.data_;
}

bool PuzzleState::operator!=(const PuzzleState &rhs) const {
   return !(*this == rhs);
}

PuzzleState PuzzleState::getNeighbor(Direction direction) const {
   int idx0 = 0;
   for (unsigned i = 0; i < 16; ++i) {
      if (getTile(i) == 0) {
         idx0 = i;
         break;
      }
   }
   std::array<char, 16> arr = asArray();
   int idxNeighbor;

   switch (direction) {
      // move tile below the tile 0 UP
   case Direction::UP:
      idxNeighbor = idx0 + 4;
      if (idxNeighbor < 16) {
         std::swap(arr[idx0], arr[idxNeighbor]);
         return arr;
      }
      break;
   case Direction::DOWN:
      idxNeighbor = idx0 - 4;
      if (idxNeighbor >= 0) {
         std::swap(arr[idx0], arr[idxNeighbor]);
         return arr;
      }
      break;
   case Direction::LEFT:
      idxNeighbor = idx0 + 1;
      if (idxNeighbor < 16 && idxNeighbor / 4 == idx0 / 4) {
         std::swap(arr[idx0], arr[idxNeighbor]);
         return arr;
      }
      break;
   case Direction::RIGHT:
      idxNeighbor = idx0 - 1;
      if (idxNeighbor >= 0 && idx0 / 4 == idxNeighbor / 4) {
         std::swap(arr[idx0], arr[idxNeighbor]);
         return arr;
      }
      break;
   }

   return PuzzleState(std::array<char, 16>{});
}

std::vector<PuzzleState> PuzzleState::getNeighbors() const {
   std::vector<PuzzleState> neighbours;
   PuzzleState up, down, left, right;

   up = getNeighbor(Direction::UP);
   down = getNeighbor(Direction::DOWN);
   left = getNeighbor(Direction::LEFT);
   right = getNeighbor(Direction::RIGHT);
   // auto invalid = PuzzleState(std::array<char,16>{});

   if (up.data_ != 0) neighbours.push_back(up);
   if (down.data_ != 0) neighbours.push_back(down);
   if (right.data_ != 0) neighbours.push_back(right);
   if (left.data_ != 0) neighbours.push_back(left);

   return neighbours;
}

std::vector<PuzzleState>
solveBFS(const PuzzleState &startState, const PuzzleState &desiredState, size_t *iterations) {
   if (iterations) *iterations = 0;
   std::vector<PuzzleState> ret;

   std::queue<PuzzleState> q;
   std::map<PuzzleState, PuzzleState> predecessor;

   q.push(startState);
   predecessor.insert({startState, startState});

   while (!q.empty()) {
      PuzzleState cur = q.front();
      q.pop();
      if (iterations) *iterations += 1;
      if (cur == desiredState) break;

      std::vector<PuzzleState> neighbours = cur.getNeighbors();
      for (PuzzleState &state : neighbours) {
         if (predecessor.count(state) == 0) {
            predecessor.insert({state, cur});
            q.push(state);
         }
      }
   }

   // fill in ret vector
   PuzzleState cur = desiredState;
   while (cur != startState) {
      if (predecessor.count(cur) == 0) return ret;
      ret.push_back(cur);
      cur = predecessor[cur];
   }

   ret.push_back(startState);

   std::reverse(ret.begin(), ret.end());

   return ret;
}

std::vector<PuzzleState>
solveAstar(const PuzzleState &startState, const PuzzleState &desiredState, size_t *iterations) {
   if (iterations) *iterations = 0;
   if (!isSolvable(startState, desiredState)) return {};

   using Pair = std::pair<int, PuzzleState>;

   std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> q;
   std::map<PuzzleState, PuzzleState> predecessor;
   std::map<PuzzleState, unsigned> dis;

   q.push({0, startState});

   predecessor[startState] = startState;
   dis[startState] = 0;

   while (!q.empty()) {
      Pair curPair = q.top();
      q.pop();
      PuzzleState cur = curPair.second;

      if (iterations) *iterations += 1;
      if (cur == desiredState) break;

      std::vector<PuzzleState> neighbours = cur.getNeighbors();
      for (PuzzleState &state : neighbours) {
         unsigned newCost = dis.at(cur) + 1;
         if (predecessor.count(state) == 0 || dis.at(state) > newCost) {
            predecessor[state] = cur;
            dis[state] = newCost;
            q.push({newCost + state.manhattanDistance(desiredState), state});
         }
      }
   }

   // fill in ret vector
   std::vector<PuzzleState> ret;
   ret.reserve(dis[desiredState]);
   PuzzleState cur = desiredState;
   while (cur != startState) {
      if (predecessor.count(cur) == 0) return ret;
      ret.push_back(cur);
      cur = predecessor[cur];
   }

   ret.push_back(startState);

   std::reverse(ret.begin(), ret.end());

   return ret;
}
