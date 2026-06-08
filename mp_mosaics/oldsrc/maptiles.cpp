/**
 * @file maptiles.cpp
 * Code for the maptiles function.
 */

#include <iostream>
#include <map>

#include "cs225/LUVAPixel.h"
#include "cs225/point.h"
#include "kdtree.h"
#include "maptiles.h"
#include "mosaiccanvas.h"
#include "sourceimage.h"
#include "tileimage.h"

using namespace std;

Point<3> convertToXYZ(LUVAPixel pixel) { return Point<3>(pixel.l, pixel.u, pixel.v); }

MosaicCanvas *mapTiles(SourceImage const &theSource, vector<TileImage> &theTiles) {
   /**
    * @todo Implement this function!
    */
   if (theTiles.empty() || theSource.getColumns() == 0 || theSource.getRows() == 0)
      return NULL;

   std::map<Point<3>, unsigned> tile_index;
   vector<Point<3>> points(theTiles.size());
   for (unsigned i = 0; i < theTiles.size(); ++i) {
      points[i] = convertToXYZ(theTiles[i].getAverageColor());
      tile_index.insert({points[i], i});
   }

   KDTree<3> kdtree(points);

   MosaicCanvas *ret = new MosaicCanvas(theSource.getRows(), theSource.getColumns());

   for (int row = 0; row < theSource.getRows(); ++row) {
      for (int col = 0; col < theSource.getColumns(); ++col) {
         LUVAPixel regionAvgColor = theSource.getRegionColor(row, col);
         Point<3> closest = kdtree.findNearestNeighbor(convertToXYZ(regionAvgColor));

         ret->setTile(row, col, &theTiles.at(tile_index.at(closest)));
      }
   }

   return ret;
}
