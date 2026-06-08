/* Your code here! */

#include "maze.h"
#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"
#include "dsets.h"
#include <algorithm>
#include <map>
#include <queue>
#include <random>
#include <utility>

void SquareMaze::makeMaze(int width, int height) {
   if (width <= 0 || height <= 0) return;
   rw_ = std::vector<std::vector<bool>>(height, std::vector<bool>(width, true));
   dw_ = std::vector<std::vector<bool>>(height, std::vector<bool>(width, true));
   this->width = width;
   this->height = height;

   DisjointSets sets(width * height); // since there are that many cells

   std::vector<std::pair<int, Direction>> walls;
   walls.reserve(2 * width * height - width - height);
   for (int col = 0; col < height; ++col) {
      for (int row = 0; row < width; ++row) {
         if (row != width - 1) walls.push_back({col * width + row, RIGHT});
         if (col != height - 1) walls.push_back({col * width + row, DOWN});
      }
   }
   std::shuffle(walls.begin(), walls.end(), std::mt19937(std::random_device{}()));

   int count_removed = 0;
   for (unsigned i = 0; i < walls.size(); ++i) {
      auto &[idx, dir] = walls[i];
      int x = idx % width;
      int y = idx / width;
      if (dir == DOWN) {
         if (sets.find(idx) != sets.find(idx + width)) {
            sets.setUnion(idx, idx + width);
            dw_[y][x] = false;
            ++count_removed;
         }
      } else {
         if (sets.find(idx) != sets.find(idx + 1)) {
            sets.setUnion(idx, idx + 1);
            rw_[y][x] = false;
            ++count_removed;
         }
      }

      if (count_removed == width * height - 1) break;
   }
}

bool SquareMaze::canTravel(int x, int y, Direction dir) const {
   if (x < 0 || y < 0 || x >= width || y >= height) return false;
   if (dir == DOWN) {
      return !dw_[y][x];

   } else if (dir == RIGHT) {
      return !rw_[y][x];

   } else if (dir == UP) {
      if (y >= 1) return !dw_[y - 1][x];

   } else {
      if (x >= 1) return !rw_[y][x - 1];
   }
   return false;
}

void SquareMaze::setWall(int x, int y, Direction dir, bool exists) {
   if (dir == UP) --y;
   else if (dir == LEFT) --x;

   if (x < 0 || y < 0 || x >= width || y >= height) return;

   if ((dir == UP || dir == DOWN) && y < height - 1) dw_[y][x] = exists;
   else if (x < width - 1) rw_[y][x] = exists;
}

std::vector<Direction> SquareMaze::solveMaze(int startX) {
   std::vector<Direction> ret;
   if (startX < 0 || startX >= width) return ret;
   if (height == 1) return ret;
   if (width == 1) {
      ret.assign(height - 1, DOWN);
      return ret;
   }

   std::queue<int> q;
   std::map<int, int> dis; // generate x and y from  first int (which is index)
   std::map<int, int> pre;

   q.push(getIdx(startX, 0));
   dis[getIdx(startX, 0)] = 0;
   dis[getIdx(startX, 0)] = getIdx(startX, 0);

   int maxIdx = getIdx(0, height - 1); // first cell on bottom row

   while (!q.empty()) {
      int curIdx = q.front();
      q.pop();

      int x = curIdx % width;
      int y = curIdx / width;

      // if curIdx is in bottom row
      if (y == height - 1) {
         // if distance of current cell is larger than current max
         if (dis[curIdx] > dis[maxIdx]) maxIdx = curIdx;
         // if ==, take the smaller x index
         else if (dis[curIdx] == dis[maxIdx]) maxIdx = (curIdx < maxIdx) ? curIdx : maxIdx;
      }

      if (canTravel(x, y, DOWN)) {
         int idx = getIdx(x, y + 1);
         if (dis[idx] == 0) {
            q.push(idx);
            dis[idx] = dis[curIdx] + 1;
            pre[idx] = curIdx;
         }
      }
      if (canTravel(x, y, RIGHT)) {
         int idx = getIdx(x + 1, y);
         if (dis[idx] == 0) {
            q.push(idx);
            dis[idx] = dis[curIdx] + 1;
            pre[idx] = curIdx;
         }
      }
      if (canTravel(x, y, UP)) {
         int idx = getIdx(x, y - 1);
         if (dis[idx] == 0) {
            q.push(idx);
            dis[idx] = dis[curIdx] + 1;
            pre[idx] = curIdx;
         }
      }
      if (canTravel(x, y, LEFT)) {
         int idx = getIdx(x - 1, y);
         if (dis[idx] == 0) {
            q.push(idx);
            dis[idx] = dis[curIdx] + 1;
            pre[idx] = curIdx;
         }
      }
   }

   // get direction vector
   int cur = maxIdx;
   while (cur != startX) {
      ret.push_back(getDir(cur, pre[cur]));
      cur = pre[cur];
   }

   std::reverse(ret.begin(), ret.end());

   return ret;
}

Direction SquareMaze::getDir(int curIdx, int preIdx) const {
   int xcur = curIdx % width;
   int ycur = curIdx / width;
   int xpre = preIdx % width;
   int ypre = preIdx / width;
   if (xcur == xpre + 1) return RIGHT;
   if (xcur == xpre - 1) return LEFT;
   if (ycur == ypre + 1) return DOWN;
   return UP;
}

int SquareMaze::getIdx(int x, int y) const {
   return y * width + x;
}

cs225::PNG *SquareMaze::drawMaze(int start) const {
   cs225::PNG *png = new cs225::PNG(width * 10 + 1, height * 10 + 1);
   for (unsigned i = 0; i < png->width(); ++i)
      png->getPixel(i, 0).l = 0;
   for (unsigned i = 0; i < png->height(); ++i)
      png->getPixel(0, i).l = 0;
   for (int i = start * 10 + 1; i <= (start + 1) * 10 - 1; ++i)
      png->getPixel(i, 0).l = 1;

   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
         if (rw_[y][x] == true) {
            for (int k = 0; k <= 10; ++k)
               png->getPixel((x + 1) * 10, y * 10 + k).l = 0;
         }
      }
   }

   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
         if (dw_[y][x] == true) {
            for (int k = 0; k <= 10; ++k)
               png->getPixel(x * 10 + k, (y + 1) * 10).l = 0;
         }
      }
   }

   return png;
}

cs225::PNG *SquareMaze::drawMazeWithSolution(int start) {
   cs225::PNG *png = drawMaze(start);
   std::vector<Direction> sol = solveMaze(start);

   std::pair<int, int> cell{start, 0};
   cs225::HSLAPixel red(0, 1, 0.5, 1);
   for (Direction dir : sol) {
      if (dir == DOWN) {
         for (int k = 0; k < 11; k++) {
            png->getPixel(5 + cell.first * 10, 5 + cell.second * 10 + k) = red;
         }
         ++cell.second;
      } else if (dir == RIGHT) {
         for (int k = 0; k < 11; k++) {
            png->getPixel(5 + cell.first * 10 + k, 5 + cell.second * 10) = red;
         }
         ++cell.first;
      } else if (dir == UP) {
         for (int k = 0; k < 11; k++) {
            png->getPixel(5 + cell.first * 10, 5 + cell.second * 10 - k) = red;
         }
         --cell.second;
      } else {
         for (int k = 0; k < 11; k++) {
            png->getPixel(5 + cell.first * 10 - k, 5 + cell.second * 10) = red;
         }
         --cell.first;
      }
   }

   // now cell == destination
   //
   for (int k = 1; k < 10; ++k)
      png->getPixel(cell.first * 10 + k, (cell.second + 1) * 10).l = 1;

   return png;
}
