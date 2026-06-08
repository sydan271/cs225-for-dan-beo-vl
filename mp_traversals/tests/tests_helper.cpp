#include "tests_helper.h"

using namespace cs225;

/**
 * Returns a 4x4 image with a white center and black border:
 * 
 *     X X X X
 *     X - - X       X: black
 *     X - - X       -: white
 *     X X X X
 * 
 * The black/X (h=180, s=1, l=0) is as different from
 * white/- (h=0, s=0, l=1) as possible.
**/
PNG getTestPNG() {
  PNG png(4, 4);
  HSLAPixel blackPixel(180, 1, 0);
  
  for (unsigned i = 0; i < 4; i++) {
    png.getPixel(i, 0) = blackPixel;
    png.getPixel(0, i) = blackPixel;
    png.getPixel(i, 3) = blackPixel;
    png.getPixel(3, i) = blackPixel;
  }
    
  return png;
}

/**
 *     0 1 2 3 4 5 6 7 
 *   -----------------
 *  0: X X X X X X X X
 *  1: X - - - - - - X    X: black
 *  2: X - * - - X - X    -: white
 *  3: X X X X X X X X    *: Starting point (white)
 * 
 * The black/X (h=180, s=1, l=0) is as different from
 * white/- (h=0, s=0, l=1) as possible.
**/
PNG getTestPNG_8x4() {
  PNG png(8, 4);
  HSLAPixel blackPixel(180, 1, 0);
  
  for (unsigned i = 0; i < 8; i++) {    
    png.getPixel(i, 0) = blackPixel;
    png.getPixel(i, 3) = blackPixel;
  }

  png.getPixel(0, 1) = blackPixel;
  png.getPixel(0, 2) = blackPixel;
  png.getPixel(7, 1) = blackPixel;
  png.getPixel(7, 2) = blackPixel;
  png.getPixel(5, 2) = blackPixel;

  return png;
}

PNG getGradPNG() {
  PNG png(10,10);
  int hue = 225;
  for (unsigned i = 0; i < 10; i++) {
    for (unsigned j = 0; j < 10; j++) {
      png.getPixel(i,j) = HSLAPixel(hue, 1, 0.5);
    }
    hue += 5;
  }
  return png;
}