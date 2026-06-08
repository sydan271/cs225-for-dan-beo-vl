/* Your code here! */
#include "maze.h"
#include "cs225/PNG.h"
#include "dsets.h"
#include <algorithm>
#include <chrono>
#include <random>

unsigned SquareMaze::getIdx(unsigned x, unsigned y) const { return y * width + x; }

void SquareMaze::FillRandomWalls(std::vector<Wall> &walls, int w, int h) {
   walls.clear();
   // fill in right
   for (int x = 0; x < w - 1; ++x) {
      for (int y = 0; y < h; ++y) {
         walls.push_back({x, y, RIGHT});
      }
   }

   // fill in black
   for (int x = 0; x < w; ++x) {
      for (int y = 0; y < h - 1; ++y) {
         walls.push_back({x, y, DOWN});
      }
   }

   std::random_device rd;
   std::mt19937 gen(rd());
   std::shuffle(walls.begin(), walls.end(), gen); // shuffle the walls to randomize maze
}

void SquareMaze::makeMaze(int width, int height) {
   if (width <= 0 || height <= 0)
      return;

   this->width = width;
   this->height = height;

   int n = width * height;
   DisjointSets cells(n);

   rWalls_.assign(n, true);
   dWalls_.assign(n, true);

   std::vector<Wall> walls_;
   walls_.reserve(2 * n - width - height);
   FillRandomWalls(walls_, width, height);

   int deleted = 0;
   for (Wall &wall : walls_) {
      int x = wall.x;
      int y = wall.y;
      int idx = getIdx(x, y);

      int AdjCellIdx = (wall.dir == RIGHT) ? getIdx(x + 1, y) : getIdx(x, y + 1);
      // delete this wall
      if (cells.find(idx) != cells.find(AdjCellIdx)) {
         cells.setUnion(idx, AdjCellIdx);
         if (wall.dir == RIGHT)
            rWalls_[idx] = false;
         else
            dWalls_[idx] = false;
         ++deleted;
         if (deleted == n - 1) // delete n - 1 walls for spanning Tree
            break;
      }
   }
}

bool SquareMaze::canTravel(int x, int y, Direction dir) const {
   if (x < 0 || x >= width || y < 0 || y >= height)
      return false;

   switch (dir) {
   case UP:
      if (y >= 1) {
         return !dWalls_[getIdx(x, y - 1)];
      }
      break;
   case DOWN:
      if (y < height - 1) {
         return !dWalls_[getIdx(x, y)];
      }
      break;
   case LEFT:
      if (x >= 1) {
         return !rWalls_[getIdx(x - 1, y)];
      }
      break;
   case RIGHT:
      if (x < width - 1) {
         return !rWalls_[getIdx(x, y)];
      }
      break;
   }
   return false;
}

void SquareMaze::setWall(int x, int y, Direction dir, bool exists) {
   switch (dir) {
   case UP:
      --y;
      if (x >= 0 && x < width && y >= 0 && y < height) {
         dWalls_[getIdx(x, y)] = exists;
      }
      return;
   case DOWN:
      if (x >= 0 && x < width && y >= 0 && y < height) {
         dWalls_[getIdx(x, y)] = exists;
      }
      return;
   case LEFT:
      --x;
      if (x >= 0 && x < width && y >= 0 && y < height) {
         rWalls_[getIdx(x, y)] = exists;
      }
      return;
   case RIGHT:
      if (x >= 0 && x < width && y >= 0 && y < height) {
         rWalls_[getIdx(x, y)] = exists;
      }
   }
}

std::vector<Direction> SquareMaze::solveMaze(int startX) {
   std::vector<Direction> sol;
   if (startX < 0 || startX >= width || height == 1)
      return sol;

   std::vector<int> dis(width * height, 0);
   std::vector<int> predecessor(width * height, -1);

   int startIdx = getIdx(startX, 0);
   std::queue<int> q;
   q.push(startIdx);

   std::set<int> visited;
   visited.insert(startIdx);

   while (!q.empty()) {
      int curIdx = q.front();
      q.pop();

      addNeighbors(q, visited, curIdx, predecessor, dis);
   }

   int maxDisX = 0;
   // search last row for longest distance
   for (int x = 1; x < width; x++) {
      if (dis[getIdx(maxDisX, height - 1)] < dis[getIdx(x, height - 1)])
         maxDisX = x;
   }

   int cur = getIdx(maxDisX, height - 1);
   sol.assign(dis[cur], DOWN);

   // backtrack to get direction
   for (int index = dis[cur] - 1; index >= 0; index--) {
      sol[index] = getDir(predecessor[cur], cur);
      cur = predecessor[cur];
   }

   return sol;
}

Direction SquareMaze::getDir(int pred, int cur) {
   int dif = pred - cur;
   if (dif == -width)
      return DOWN;
   if (dif == -1)
      return RIGHT;
   if (dif == 1)
      return LEFT;
   return UP;
}

void SquareMaze::addNeighbors(std::queue<int> &q,
                              std::set<int> &visited,
                              int index,
                              std::vector<int> &pre,
                              std::vector<int> &dis) {
   int x = index % width;
   int y = index / width;

   if (canTravel(x, y, UP)) {
      int upIdx = getIdx(x, y - 1);
      if (visited.find(upIdx) == visited.end()) {
         visited.insert(upIdx);
         q.push(upIdx);
         pre[upIdx] = index;
         dis[upIdx] = dis[index] + 1;
      }
   }
   if (canTravel(x, y, DOWN)) {
      int downIdx = getIdx(x, y + 1);
      if (visited.find(downIdx) == visited.end()) {
         visited.insert(downIdx);
         q.push(downIdx);
         pre[downIdx] = index;
         dis[downIdx] = dis[index] + 1;
      }
   }
   if (canTravel(x, y, LEFT)) {
      int leftIdx = getIdx(x - 1, y);
      if (visited.find(leftIdx) == visited.end()) {
         visited.insert(leftIdx);
         q.push(leftIdx);
         pre[leftIdx] = index;
         dis[leftIdx] = dis[index] + 1;
      }
   }
   if (canTravel(x, y, RIGHT)) {
      int rightIdx = getIdx(x + 1, y);
      if (visited.find(rightIdx) == visited.end()) {
         visited.insert(rightIdx);
         q.push(rightIdx);
         pre[rightIdx] = index;
         dis[rightIdx] = dis[index] + 1;
      }
   }
}

cs225::PNG *SquareMaze::drawMaze(int start) const {
   cs225::PNG *ret = new cs225::PNG(width * 10 + 1, height * 10 + 1);

   // blacken top row
   for (unsigned i = 0; i < ret->width(); i++)
      ret->getPixel(i, 0).l = 0;
   // blacken left column
   for (unsigned i = 0; i < ret->height(); i++)
      ret->getPixel(0, i).l = 0;
   // unblacken the entrance
   for (int x = start * 10 + 1; x <= (start + 1) * 10 - 1; ++x)
      ret->getPixel(x, 0).l = 1;

   for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
         // blacken right wall
         if (rWalls_[getIdx(x, y)]) {
            for (int k = 0; k <= 10; ++k) {
               ret->getPixel((x + 1) * 10, y * 10 + k).l = 0;
            }
         }

         // blacken below wall
         if (dWalls_[getIdx(x, y)]) {
            for (int k = 0; k <= 10; ++k) {
               ret->getPixel(x * 10 + k, (y + 1) * 10).l = 0;
            }
         }
      }
   }

   return ret;
}

cs225::PNG *SquareMaze::drawMazeWithSolution(int start) {
   cs225::PNG *ret = drawMaze(start);

   std::vector<Direction> sol = solveMaze(start);

   int x = 5 + start * 10;
   int y = 5;

   for (const Direction dir : sol) {
      drawDirectedLine(x, y, dir, ret);
      updatePenPosition(x, y, dir);
   }

   int destX = (x - 5) / 10;
   for (int k = 1; k < 10; k++) {
      ret->getPixel(10 * destX + k, height * 10).l = 1;
   }

   return ret;
}

void SquareMaze::drawDirectedLine(int x, int y, Direction dir, cs225::PNG *png) {
   switch (dir) {
   case LEFT:
      for (int i = 0; i <= 10; i++) {
         png->getPixel(x - i, y) = cs225::HSLAPixel(0, 1, 0.5);
      }
      return;
   case RIGHT:
      for (int i = 0; i <= 10; i++) {
         png->getPixel(x + i, y) = cs225::HSLAPixel(0, 1, 0.5);
      }
      return;
   case UP:
      for (int i = 0; i <= 10; i++) {
         png->getPixel(x, y - i) = cs225::HSLAPixel(0, 1, 0.5);
      }
      return;
   case DOWN:
      for (int i = 0; i <= 10; i++) {
         png->getPixel(x, y + i) = cs225::HSLAPixel(0, 1, 0.5);
      }
      return;
   }
}

void SquareMaze::updatePenPosition(int &x, int &y, Direction dir) {
   switch (dir) {
   case RIGHT:
      x += 10;
      return;
   case LEFT:
      x -= 10;
      return;
   case UP:
      y -= 10;
      return;
   case DOWN:
      y += 10;
   }
}
