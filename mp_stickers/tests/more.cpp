
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Image.h"
#include "cs225/HSLAPixel.h"

#include <algorithm>
#include <cmath>

using cs225::HSLAPixel;

static Image makePattern(unsigned w, unsigned h) {
   Image img;
   img.resize(w, h);

   // Unique, easy-to-track values per coordinate.
   // Hue in [0, 360), saturation/luminance in [0,1].
   for (unsigned y = 0; y < h; y++) {
      for (unsigned x = 0; x < w; x++) {
         HSLAPixel &p = img.getPixel(x, y);
         p.h = (x * 40 + y * 25) % 360;                // unique-ish hue
         p.s = std::min(1.0, 0.05 + 0.10 * double(x)); // depends on x
         p.l = std::min(1.0, 0.05 + 0.10 * double(y)); // depends on y
         p.a = 0.3 + 0.01 * double(x + y);             // alpha varies too
      }
   }
   return img;
}

static void
setPixel(Image &img, unsigned x, unsigned y, double h, double s, double l, double a = 1.0) {
   HSLAPixel &p = img.getPixel(x, y);
   p.h = h;
   p.s = s;
   p.l = l;
   p.a = a;
}

static void requireSameExceptL(const HSLAPixel &before, const HSLAPixel &after) {
   REQUIRE(after.h == Catch::Approx(before.h));
   REQUIRE(after.s == Catch::Approx(before.s));
   REQUIRE(after.a == Catch::Approx(before.a));
}

static void requireSameExceptS(const HSLAPixel &before, const HSLAPixel &after) {
   REQUIRE(after.h == Catch::Approx(before.h));
   REQUIRE(after.l == Catch::Approx(before.l));
   REQUIRE(after.a == Catch::Approx(before.a));
}

static void requireSameExceptHue(const HSLAPixel &before, const HSLAPixel &after) {
   REQUIRE(after.s == Catch::Approx(before.s));
   REQUIRE(after.l == Catch::Approx(before.l));
   REQUIRE(after.a == Catch::Approx(before.a));
}

static double wrapHue(double h) {
   while (h < 0)
      h += 360;
   while (h >= 360)
      h -= 360;
   return h;
}

/**************************************
 * Lighten / Darken
 **************************************/

TEST_CASE("lighten() increases luminance by 0.1 (clamped) for all pixels",
          "[part=1][image][lighten]") {
   Image img = makePattern(4, 4);
   Image before = img;

   img.lighten(); // +0.1

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptL(b, a);

         double expected = std::min(1.0, b.l + 0.1);
         REQUIRE(a.l == Catch::Approx(expected));
      }
   }
}

TEST_CASE("lighten(amount) increases luminance by amount (clamped)", "[part=1][image][lighten]") {
   Image img = makePattern(3, 3);
   Image before = img;

   img.lighten(0.35);

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptL(b, a);

         double expected = std::min(1.0, b.l + 0.35);
         REQUIRE(a.l == Catch::Approx(expected));
      }
   }
}

TEST_CASE("darken() decreases luminance by 0.1 (clamped) for all pixels",
          "[part=1][image][darken]") {
   Image img = makePattern(4, 4);
   Image before = img;

   img.darken(); // -0.1

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptL(b, a);

         double expected = std::max(0.0, b.l - 0.1);
         REQUIRE(a.l == Catch::Approx(expected));
      }
   }
}

TEST_CASE("darken(amount) decreases luminance by amount (clamped)", "[part=1][image][darken]") {
   Image img = makePattern(3, 3);
   Image before = img;

   img.darken(0.35);

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptL(b, a);

         double expected = std::max(0.0, b.l - 0.35);
         REQUIRE(a.l == Catch::Approx(expected));
      }
   }
}

/**************************************
 * Saturate / Desaturate
 **************************************/

TEST_CASE("saturate() increases saturation by 0.1 (clamped) for all pixels",
          "[part=1][image][saturate]") {
   Image img = makePattern(4, 4);
   Image before = img;

   img.saturate(); // +0.1

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptS(b, a);

         double expected = std::min(1.0, b.s + 0.1);
         REQUIRE(a.s == Catch::Approx(expected));
      }
   }
}

TEST_CASE("saturate(amount) increases saturation by amount (clamped)",
          "[part=1][image][saturate]") {
   Image img = makePattern(3, 3);
   Image before = img;

   img.saturate(0.6);

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptS(b, a);

         double expected = std::min(1.0, b.s + 0.6);
         REQUIRE(a.s == Catch::Approx(expected));
      }
   }
}

TEST_CASE("desaturate() decreases saturation by 0.1 (clamped) for all pixels",
          "[part=1][image][desaturate]") {
   Image img = makePattern(4, 4);
   Image before = img;

   img.desaturate(); // -0.1

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptS(b, a);

         double expected = std::max(0.0, b.s - 0.1);
         REQUIRE(a.s == Catch::Approx(expected));
      }
   }
}

TEST_CASE("desaturate(amount) decreases saturation by amount (clamped)",
          "[part=1][image][desaturate]") {
   Image img = makePattern(3, 3);
   Image before = img;

   img.desaturate(0.6);

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptS(b, a);

         double expected = std::max(0.0, b.s - 0.6);
         REQUIRE(a.s == Catch::Approx(expected));
      }
   }
}

/**************************************
 * Grayscale
 **************************************/

TEST_CASE("grayscale() sets saturation to 0 and preserves hue/luminance/alpha",
          "[part=1][image][grayscale]") {
   Image img = makePattern(4, 3);
   Image before = img;

   img.grayscale();

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         REQUIRE(a.s == Catch::Approx(0.0));
         REQUIRE(a.h == Catch::Approx(b.h));
         REQUIRE(a.l == Catch::Approx(b.l));
         REQUIRE(a.a == Catch::Approx(b.a));
      }
   }
}

/**************************************
 * rotateColor
 **************************************/

TEST_CASE("rotateColor(deg) rotates hue and wraps into [0,360)", "[part=1][image][rotateColor]") {
   Image img;
   img.resize(1, 1);
   setPixel(img, 0, 0, 350, 0.4, 0.6, 0.7);

   img.rotateColor(20); // 350 -> 10
   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(10));

   img.rotateColor(-30); // 10 -> 340
   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(340));

   img.rotateColor(450); // +450 == +90, 340 -> 70
   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(70));
}

TEST_CASE("rotateColor preserves s/l/a for all pixels", "[part=1][image][rotateColor]") {
   Image img = makePattern(5, 4);
   Image before = img;

   img.rotateColor(-123);

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         requireSameExceptHue(b, a);

         double expected = wrapHue(b.h - 123);
         REQUIRE(a.h == Catch::Approx(expected));
      }
   }
}

/**************************************
 * invert
 **************************************/

TEST_CASE("invert() inverts h/s/l and preserves alpha (single pixel cases)",
          "[part=1][image][invert]") {
   Image img;
   img.resize(1, 1);

   setPixel(img, 0, 0, 45, 1.0, 0.55, 0.42);
   img.invert();

   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(225));
   REQUIRE(img.getPixel(0, 0).s == Catch::Approx(0.0));
   REQUIRE(img.getPixel(0, 0).l == Catch::Approx(0.45));
   REQUIRE(img.getPixel(0, 0).a == Catch::Approx(0.42));

   setPixel(img, 0, 0, 355, 0.3, 0.2, 0.9);
   img.invert();

   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(175));
   REQUIRE(img.getPixel(0, 0).s == Catch::Approx(0.7));
   REQUIRE(img.getPixel(0, 0).l == Catch::Approx(0.8));
   REQUIRE(img.getPixel(0, 0).a == Catch::Approx(0.9));
}

TEST_CASE("invert() applies to all pixels (pattern image)", "[part=1][image][invert]") {
   Image img = makePattern(4, 4);
   Image before = img;

   img.invert();

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);

         REQUIRE(a.h == Catch::Approx(wrapHue(b.h + 180)));
         REQUIRE(a.s == Catch::Approx(1.0 - b.s));
         REQUIRE(a.l == Catch::Approx(1.0 - b.l));
         REQUIRE(a.a == Catch::Approx(b.a));
      }
   }
}

/**************************************
 * scale(factor)
 * Assumes nearest-neighbor style mapping: new(x,y) = old(floor(x/f), floor(y/f)).
 **************************************/

TEST_CASE("scale(1.0) keeps dimensions and pixel values", "[part=1][image][scale]") {
   Image img = makePattern(5, 3);
   Image before = img;

   img.scale(1.0);

   REQUIRE(img.width() == before.width());
   REQUIRE(img.height() == before.height());

   for (unsigned y = 0; y < img.height(); y++) {
      for (unsigned x = 0; x < img.width(); x++) {
         const HSLAPixel &b = before.getPixel(x, y);
         const HSLAPixel &a = img.getPixel(x, y);
         REQUIRE(a.h == Catch::Approx(b.h));
         REQUIRE(a.s == Catch::Approx(b.s));
         REQUIRE(a.l == Catch::Approx(b.l));
         REQUIRE(a.a == Catch::Approx(b.a));
      }
   }
}

TEST_CASE("scale(2.0) doubles dimensions and replicates nearest-neighbor blocks",
          "[part=1][image][scale]") {
   Image img;
   img.resize(2, 2);

   setPixel(img, 0, 0, 10, 0.2, 0.3, 0.4);
   setPixel(img, 1, 0, 20, 0.2, 0.3, 0.4);
   setPixel(img, 0, 1, 30, 0.2, 0.3, 0.4);
   setPixel(img, 1, 1, 40, 0.2, 0.3, 0.4);

   img.scale(2.0);

   REQUIRE(img.width() == 4);
   REQUIRE(img.height() == 4);

   // Top-left original pixel should fill a 2x2 block
   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(10));
   REQUIRE(img.getPixel(1, 0).h == Catch::Approx(10));
   REQUIRE(img.getPixel(0, 1).h == Catch::Approx(10));
   REQUIRE(img.getPixel(1, 1).h == Catch::Approx(10));

   // Top-right original pixel fills block at x in [2,3], y in [0,1]
   REQUIRE(img.getPixel(2, 0).h == Catch::Approx(20));
   REQUIRE(img.getPixel(3, 1).h == Catch::Approx(20));

   // Bottom-left
   REQUIRE(img.getPixel(0, 2).h == Catch::Approx(30));
   REQUIRE(img.getPixel(1, 3).h == Catch::Approx(30));

   // Bottom-right
   REQUIRE(img.getPixel(2, 2).h == Catch::Approx(40));
   REQUIRE(img.getPixel(3, 3).h == Catch::Approx(40));
}

TEST_CASE("scale(0.5) halves dimensions and uses nearest-neighbor source pixels",
          "[part=1][image][scale]") {
   Image img;
   img.resize(4, 4);

   // Hue = x + 10*y makes it obvious which source pixel was picked.
   for (unsigned y = 0; y < 4; y++) {
      for (unsigned x = 0; x < 4; x++) {
         setPixel(img, x, y, double(x + 10 * y), 0.5, 0.5, 1.0);
      }
   }

   img.scale(0.5);

   REQUIRE(img.width() == 2);
   REQUIRE(img.height() == 2);

   // If mapping is floor(x/0.5)=floor(2x), the chosen originals are (0,0), (2,0), (0,2), (2,2)
   REQUIRE(img.getPixel(0, 0).h == Catch::Approx(0));  // (0,0)
   REQUIRE(img.getPixel(1, 0).h == Catch::Approx(2));  // (2,0)
   REQUIRE(img.getPixel(0, 1).h == Catch::Approx(20)); // (0,2)
   REQUIRE(img.getPixel(1, 1).h == Catch::Approx(22)); // (2,2)
}

/**************************************
 * scale(w,h) : fit within bounding box while preserving aspect ratio
 **************************************/

TEST_CASE("scale(w,h) preserves aspect ratio and fits within bounds (wide image)",
          "[part=1][image][scale]") {
   Image img = makePattern(200, 100); // aspect 2:1
   img.scale(100, 100);

   // Expected: width hits 100, height becomes 50
   REQUIRE(img.width() == 100);
   REQUIRE(img.height() == 50);
}

TEST_CASE("scale(w,h) preserves aspect ratio and fits within bounds (tall image)",
          "[part=1][image][scale]") {
   Image img = makePattern(100, 200); // aspect 1:2
   img.scale(100, 100);

   // Expected: height hits 100, width becomes 50
   REQUIRE(img.width() == 50);
   REQUIRE(img.height() == 100);
}

TEST_CASE("scale(w,h) does not exceed either bound", "[part=1][image][scale]") {
   Image img = makePattern(80, 60);
   img.scale(100, 70);

   REQUIRE(img.width() <= 100);
   REQUIRE(img.height() <= 70);

   // At least one dimension should match a bound (fits tightly)
   REQUIRE((img.width() == 100 || img.height() == 70));
}
