#include "Image.h"
#include <cmath>

void Image::lighten() {
   lighten(0.1);
}

void Image::lighten(double amount) {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &l = getPixel(row, col).l;
         l += amount;
         l = (l > 1) ? 1 : l;
      }
   }
}

void Image::darken() {
   darken(0.1);
}

void Image::darken(double amount) {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &l = getPixel(row, col).l;
         l -= amount;
         l = (l < 0) ? 0 : l;
      }
   }
}

void Image::saturate() {
   saturate(0.1);
}

void Image::saturate(double amount) {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &s = getPixel(row, col).s;
         s += amount;
         s = (s > 1) ? 1 : s;
      }
   }
}

void Image::desaturate() {
   desaturate(0.1);
}

void Image::desaturate(double amount) {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &s = getPixel(row, col).s;
         s -= amount;
         s = (s < 0) ? 0 : s;
      }
   }
}

void Image::grayscale() {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &s = getPixel(row, col).s;
         s = 0;
      }
   }
}

void Image::rotateColor(double degrees) {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &h = getPixel(row, col).h;
         h = std::fmod(h + degrees, 360);
         while (h < 0)
            h += 360;
      }
   }
}

void Image::invert() {
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         auto &pixel = getPixel(row, col);
         pixel.s = 1 - pixel.s;
         pixel.l = 1 - pixel.l;
         pixel.h = std::fmod(pixel.h + 180, 360);
      }
   }
}

void Image::scale(double factor) {
   if (factor < 0) return;
   Image temp(*this);

   resize(width() * factor, height() * factor);
   for (unsigned row = 0; row < width(); ++row) {
      for (unsigned col = 0; col < height(); ++col) {
         getPixel(row, col) = temp.getPixel(row / factor, col / factor);
      }
   }
}

void Image::scale(unsigned w, unsigned h) {
   double scaleFactor = std::min((double)w / width(), (double)h / height());
   scale(scaleFactor);
}
