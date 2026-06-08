#include <cmath>
#include <iostream>
#include <iterator>

#include "Point.h"
#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"

#include "ImageTraversal.h"

namespace Traversals {
/**
 * Calculates a metric for the difference between two pixels, used to
 * calculate if a pixel is within a tolerance.
 *
 * @param p1 First pixel
 * @param p2 Second pixel
 * @return the difference between two HSLAPixels
 */
double calculateDelta(const HSLAPixel &p1, const HSLAPixel &p2) {
   double h = fabs(p1.h - p2.h);
   double s = p1.s - p2.s;
   double l = p1.l - p2.l;

   // Handle the case where we found the bigger angle between two hues:
   if (h > 180) {
      h = 360 - h;
   }
   h /= 360;

   return sqrt((h * h) + (s * s) + (l * l));
}

/**
 * Adds a Point for the bfs traversal to visit at some point in the future.
 * @param work_list the deque storing a list of points to be processed
 * @param point the point to be added
 */
void bfs_add(std::deque<Point> &work_list, const Point &point) {
   work_list.push_back(point);
}

/**
 * Adds a Point for the dfs traversal to visit at some point in the future.
 * @param work_list the deque storing a list of points to be processed
 * @param point the point to be added
 */
void dfs_add(std::deque<Point> &work_list, const Point &point) {
   work_list.push_back(point);
}

/**
 * Removes the current Point in the bfs traversal
 * @param work_list the deque storing a list of points to be processed
 */
void bfs_pop(std::deque<Point> &work_list) {
   work_list.pop_front();
}

/**
 * Removes the current Point in the dfs traversal
 * @param work_list the deque storing a list of points to be processed
 */
void dfs_pop(std::deque<Point> &work_list) {
   work_list.pop_back();
}

/**
 * Returns the current Point in the bfs traversal
 * @param work_list the deque storing a list of points to be processed
 * @return the current Point
 */
Point bfs_peek(std::deque<Point> &work_list) {
   /** @todo [Part 1] */
   return work_list.front();
}

/**
 * Returns the current Point in the dfs traversal
 * @param work_list the deque storing a list of points to be processed
 * @return the current Point
 */
Point dfs_peek(std::deque<Point> &work_list) {
   /** @todo [Part 1] */
   return work_list.back();
}

/**
 * Initializes a ImageTraversal on a given `png` image,
 * starting at `start`, and with a given `tolerance`.
 * @param png The image this traversal is going to traverse
 * @param start The start point of this traversal
 * @param tolerance If the current point is too different (difference larger than tolerance) with
 * the start point, it will not be included in this traversal
 * @param fns the set of functions describing a traversal's operation
 */
ImageTraversal::ImageTraversal(const PNG &png, const Point &start, double tolerance, TraversalFunctions fns)
    : png(png)
    , startingPoint(start)
    , tolerance(tolerance)
    , travFns(fns) {
   /** @todo [Part 1] */
   if (startingPoint.x < 0 || startingPoint.y < 0 || startingPoint.x >= png.width() ||
       startingPoint.y >= png.height() || png.height() == 0 || png.width() == 0) {
      std::cerr << "Passed in starting Point out of range\n or png invalid\n";
   }
}

/**
 * Returns an iterator for the traversal starting at the first point.
 */
ImageTraversal::Iterator ImageTraversal::begin() {
   /** @todo [Part 1] */
   ImageTraversal::Iterator it(this);
   it.add(startingPoint);

   return it;
}

/**
 * Returns an iterator for the traversal one past the end of the traversal.
 */
ImageTraversal::Iterator ImageTraversal::end() {
   /** @todo [Part 1] */
   return ImageTraversal::Iterator();
}

/**
 * Default iterator constructor.
 */
ImageTraversal::Iterator::Iterator()
    : work_list_(0)
    , parent(nullptr) {
}

ImageTraversal::Iterator::Iterator(const ImageTraversal *parent)
    : parent(parent)
    , visited(parent->png.width(), std::vector<bool>(parent->png.height(), false)) {
}

void ImageTraversal::Iterator::add(const Point &point) {
   parent->travFns.add(work_list_, point);
}

/**
 * Iterator increment operator.
 *
 * Advances the traversal of the image.
 */
ImageTraversal::Iterator &ImageTraversal::Iterator::operator++() {
   /** @todo [Part 1] */
   if (work_list_.empty()) return *this;

   Point point = parent->travFns.peek(work_list_);
   parent->travFns.pop(work_list_);

   addNeighbors(point);
   visited[point.x][point.y] = true;

   while (!work_list_.empty()) {
      Point next = parent->travFns.peek(work_list_);
      if (!visited[next.x][next.y]) break;
      parent->travFns.pop(work_list_);
   }

   return *this;
}

void ImageTraversal::Iterator::addNeighbors(const Point &point) {
   unsigned x = point.x;
   unsigned y = point.y;

   unsigned width = parent->png.width();
   unsigned height = parent->png.height();

   auto &[startX, startY] = parent->startingPoint;
   auto &start = parent->png.getPixel(startX, startY);

   // Right
   if (x < width - 1 && visited[x + 1][y] == false &&
       calculateDelta(start, parent->png.getPixel(x + 1, y)) < parent->tolerance) {
      parent->travFns.add(work_list_, Point(x + 1, y));
   }

   // Below
   if (y < height - 1 && visited[x][y + 1] == false &&
       calculateDelta(start, parent->png.getPixel(x, y + 1)) < parent->tolerance) {
      parent->travFns.add(work_list_, Point(x, y + 1));
   }

   // Left
   if (x >= 1 && visited[x - 1][y] == false &&
       calculateDelta(start, parent->png.getPixel(x - 1, y)) < parent->tolerance) {
      parent->travFns.add(work_list_, Point(x - 1, y));
   }

   // Above
   if (y >= 1 && visited[x][y - 1] == false &&
       calculateDelta(start, parent->png.getPixel(x, y - 1)) < parent->tolerance) {
      parent->travFns.add(work_list_, Point(x, y - 1));
   }
}

/**
 * Iterator accessor operator.
 *
 * Accesses the current Point in the ImageTraversal.
 */
Point ImageTraversal::Iterator::operator*() {
   /** @todo [Part 1] */
   if (work_list_.empty()) {
      std::cerr << "trying to dereference end()\n";
      return Point();
   }

   return parent->travFns.peek(work_list_);
}

/**
 * Iterator inequality operator.
 *
 * Determines if two iterators are not equal.
 */
bool ImageTraversal::Iterator::operator!=(const ImageTraversal::Iterator &other) {
   /** @todo [Part 1] */
   return work_list_ != other.work_list_;
}

/**
 * Iterator size function.
 *
 * @return size_t the size of the iterator work queue.
 */
size_t ImageTraversal::Iterator::size() const {
   return work_list_.size();
}

/**
 * Iterator empty function.
 *
 * @return bool whether the iterator work queue is empty.
 */
bool ImageTraversal::Iterator::empty() const {
   return work_list_.empty();
}

} // namespace Traversals
