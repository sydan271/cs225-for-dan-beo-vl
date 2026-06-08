/**
 * @file ImageTraversal.h
 */
#pragma once

#include "Point.h"
#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"
#include <algorithm>
#include <deque>
#include <iterator>
#include <unordered_set>
#include <vector>

using namespace cs225;

/**
 * @brief Namespace containing everything required for an image traversal.
 *
 */
namespace Traversals {
/**
 * The expected type for an add function for a traversal
 * The first parameter is the deque storing a list of points to be processed
 * The second parameter is the new Point that should be added to the deque
 */
typedef void (*add_function)(std::deque<Point> &, const Point &);

/**
 * The expected type for a pop function for a traversal
 * The first parameter is the deque storing a list of points to be processed
 * The function then removes the next Point from the deque
 */
typedef void (*pop_function)(std::deque<Point> &);

/**
 * The expected type for a peek function for a traversal
 * The first parameter is the deque storing a list of points to be processed
 * The function returns the next Point to be processed (without removing it from the deque)
 */
typedef Point (*peek_function)(std::deque<Point> &);

/**
 * The set of functions describing a traversal's operation.
 */
typedef struct TraversalFunctions {
   add_function add;
   pop_function pop;
   peek_function peek;
} TraversalFunctions;

void bfs_add(std::deque<Point> &work_list, const Point &point);
void dfs_add(std::deque<Point> &work_list, const Point &point);

void bfs_pop(std::deque<Point> &work_list);
void dfs_pop(std::deque<Point> &work_list);

Point bfs_peek(std::deque<Point> &work_list);
Point dfs_peek(std::deque<Point> &work_list);

double calculateDelta(const HSLAPixel &p1, const HSLAPixel &p2);

/**
 * A base class for traversal algorithms on images.
 * This class provides a traversal of the image by returning instances of ImageTraversal::Iterator.
 */
class ImageTraversal {
public:
   /**
    * A forward iterator through an ImageTraversal.
    */
   class Iterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Point;
      using difference_type = std::ptrdiff_t;
      using pointer = Point *;
      using reference = Point &;

      Iterator();
      Iterator(const ImageTraversal *parent);

      Iterator &operator++();
      Point operator*();
      bool operator!=(const Iterator &other);

      size_t size() const;
      bool empty() const;

      /** @todo [Part 1] */
      /** add member functions if neccesary*/
      void add(const Point &point);

   private:
      /** @todo [Part 1] */
      /** add private members here if neccesary*/
      std::deque<Point> work_list_;

      const ImageTraversal *parent;
      std::vector<std::vector<bool>> visited;

      void addNeighbors(const Point &point);
   };

   /**
    * Constructor for the given traversal
    */
   ImageTraversal(const PNG &png, const Point &start, double tolerance, TraversalFunctions fns);

   /**
    * Returns an iterator for the traversal starting at the first point (specified in the
    * constructor).
    */
   Iterator begin();

   /**
    * Returns an iterator for the traversal one past the end of the traversal.
    */
   Iterator end();

private:
   /** @todo [Part 1] */
   /** add private members here if neccesary*/
   const PNG &png;
   const Point startingPoint;
   double tolerance;
   TraversalFunctions travFns;
};
} // namespace Traversals
