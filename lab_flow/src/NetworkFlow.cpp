#include <algorithm>
#include <set>
#include <vector>

#include "cs225_graph/edge.h"
#include "cs225_graph/graph.h"

#include "NetworkFlow.h"

int min(int a, int b) {
   if (a < b)
      return a;
   else
      return b;
}

NetworkFlow::NetworkFlow(Graph &startingGraph, Vertex source, Vertex sink)
    : g_(startingGraph)
    , residual_(Graph(true, true))
    , flow_(Graph(true, true))
    , source_(source)
    , sink_(sink) {

   for (const Vertex &vertex : g_.getVertices()) {
      residual_.insertVertex(vertex);
      flow_.insertVertex(vertex);
   }

   for (const Edge &edge : g_.getEdges()) {
      Vertex src = edge.source;
      Vertex dest = edge.dest;

      flow_.insertEdge(src, dest);
      flow_.setEdgeWeight(src, dest, 0);

      residual_.insertEdge(src, dest);
      residual_.setEdgeWeight(src, dest, g_.getEdgeWeight(src, dest));

      residual_.insertEdge(dest, src);
      residual_.setEdgeWeight(dest, src, 0);
   }
}

/**
 * findAugmentingPath - use DFS to find a path in the residual graph with leftover capacity.
 *  This version is the helper function.
 *
 * @param source  The starting (current) vertex
 * @param sink    The destination vertex
 * @param path    The vertices in the path
 * @param visited A set of vertices we have visited
 */

bool NetworkFlow::findAugmentingPath(Vertex source,
                                     Vertex sink,
                                     std::vector<Vertex> &path,
                                     std::set<Vertex> &visited) {

   if (visited.count(source) != 0)
      return false;
   visited.insert(source);

   if (source == sink) {
      return true;
   }

   vector<Vertex> adjs = residual_.getAdjacent(source);
   for (auto it = adjs.begin(); it != adjs.end(); it++) {
      if (visited.count(*it) == 0 && residual_.getEdgeWeight(source, *it) > 0) {
         path.push_back(*it);
         if (findAugmentingPath(*it, sink, path, visited))
            return true;
         else {
            path.pop_back();
         }
      }
   }

   return false;
}

/**
 * findAugmentingPath - use DFS to find a path in the residual graph with leftover capacity.
 *  This version is the main function.  It initializes a set to keep track of visited vertices.
 *
 * @param source The starting (current) vertex
 * @param sink   The destination vertex
 * @param path   The vertices in the path
 */

bool NetworkFlow::findAugmentingPath(Vertex source, Vertex sink, std::vector<Vertex> &path) {
   std::set<Vertex> visited;
   path.clear();
   path.push_back(source);
   return findAugmentingPath(source, sink, path, visited);
}

/**
 * pathCapacity - Determine the capacity of a path in the residual graph.
 *
 * @param path   The vertices in the path
 */

int NetworkFlow::pathCapacity(const std::vector<Vertex> &path) const {
   // YOUR CODE HERE
   //
   if (path.size() < 2)
      return 0;
   int minCap = residual_.getEdgeWeight(path[0], path[1]);
   for (unsigned i = 1; i < path.size() - 1; ++i) {
      minCap = min(minCap, residual_.getEdgeWeight(path[i], path[i + 1]));
   }
   return minCap;
}

/**
 * calculateFlow - Determine the maximum flow of the entire graph.
 * Sets the member variable `maxFlow_` to the computed maximum flow, and updates the
 * residual graph and flow graph according to the algorithm.
 *
 * @return The network flow graph.
 */

const Graph &NetworkFlow::calculateFlow() {
   std::vector<Vertex> path;

   while (findAugmentingPath(source_, sink_, path)) {
      int capacity = pathCapacity(path);
      for (unsigned i = 0; i < path.size() - 1; ++i) {
         Vertex src = path[i];
         Vertex dest = path[i + 1];
         if (flow_.edgeExists(src, dest))
            flow_.setEdgeWeight(src, dest, capacity + flow_.getEdgeWeight(src, dest));
         else {
            flow_.setEdgeWeight(dest, src, -capacity + flow_.getEdgeWeight(dest, src));
         }

         residual_.setEdgeWeight(src, dest, -capacity + residual_.getEdgeWeight(src, dest));
         residual_.setEdgeWeight(dest, src, capacity + residual_.getEdgeWeight(dest, src));
      }

      path.clear();
   }

   maxFlow_ = 0;
   for (Vertex &v : flow_.getAdjacent(source_)) {
      maxFlow_ += flow_.getEdgeWeight(source_, v);
   }

   return flow_;
}

int NetworkFlow::getMaxFlow() const { return maxFlow_; }

const Graph &NetworkFlow::getGraph() const { return g_; }

const Graph &NetworkFlow::getFlowGraph() const { return flow_; }

const Graph &NetworkFlow::getResidualGraph() const { return residual_; }
