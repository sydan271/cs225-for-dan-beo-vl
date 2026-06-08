/**
 * @file maptiles.cpp
 * Code for the maptiles function.
 */

#include <iostream>
#include <map>

#include "cs225/LUVAPixel.h"
#include "maptiles.h"
#include "mosaiccanvas.h"
#include "tileimage.h"

using namespace std;

Point<3> convertToXYZ(LUVAPixel pixel) {
   return Point<3>(pixel.l, pixel.u, pixel.v);
}

MosaicCanvas *mapTiles(SourceImage const &theSource, vector<TileImage> &theTiles) {
   /**
    * @todo Implement this function!
    */
   if (theSource.getColumns() == 0 || theSource.getRows() == 0 || theTiles.empty()) return nullptr;

   MosaicCanvas *canva = new MosaicCanvas(theSource.getRows(), theSource.getColumns());

   std::map<Point<3>, TileImage *> map_tiles;
   vector<Point<3>> tiles;
   tiles.reserve(theTiles.size());

   for (TileImage &tile : theTiles) {
      Point<3> average = convertToXYZ(tile.getAverageColor());
      tiles.push_back(average);
      map_tiles.insert({average, &tile});
   }
   KDTree<3> averageTiles(tiles);

   for (int row = 0; row < canva->getRows(); ++row) {
      for (int col = 0; col < canva->getColumns(); ++col) {
         LUVAPixel regionAverageColor = theSource.getRegionColor(row, col);

         Point<3> bestMatch = averageTiles.findNearestNeighbor(convertToXYZ(regionAverageColor));
         canva->setTile(row, col, map_tiles.at(bestMatch));
      }
   }

   return canva;
}
