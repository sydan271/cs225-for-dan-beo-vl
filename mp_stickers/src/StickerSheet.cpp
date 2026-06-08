#include "StickerSheet.h"

StickerSheet::StickerSheet(const Image &picture)
    : base_(picture) {
}

int StickerSheet::addSticker(Image &sticker, int x, int y) {
   for (unsigned i = 0; i < stickers_.size(); ++i) {
      if (stickers_[i] == nullptr) {
         stickers_[i] = &sticker;
         pos_[i] = {x, y};
         return i;
      }
   }
   stickers_.push_back(&sticker);
   pos_.push_back({x, y});
   return stickers_.size() - 1;
}

Image *StickerSheet::getSticker(unsigned index) {
   if (index < 0 || index >= stickers_.size()) return NULL;
   return stickers_[index];
}

int StickerSheet::setStickerAtLayer(Image &sticker, unsigned layer, int x, int y) {
   if (layer < 0) return -1;
   if (layer >= stickers_.size()) {
      stickers_.push_back(&sticker);
      pos_.push_back({x, y});
      return stickers_.size() - 1;
   }
   stickers_[layer] = &sticker;
   pos_[layer] = {x, y};
   return layer;
}

int StickerSheet::layers() const {
   return stickers_.size();
}

void StickerSheet::removeSticker(unsigned index) {
   if (index < 0 || index >= stickers_.size()) return;
   stickers_[index] = nullptr;
}

bool StickerSheet::translate(unsigned index, int x, int y) {
   if (index < 0 || index >= stickers_.size() || stickers_[index] == nullptr) return false;
   pos_[index] = {x, y};

   return true;
}

Image StickerSheet::render() const {
   // calculate size of image
   int left_wall = 0;
   int up_wall = 0;
   int right_wall = base_.width();
   int down_wall = base_.height();

   for (unsigned layer = 0; layer < stickers_.size(); ++layer) {
      Image *sticker = stickers_[layer];
      if (sticker == nullptr) continue;
      auto &pos = pos_[layer];

      left_wall = std::min(left_wall, pos.first);
      right_wall = std::max(right_wall, pos.first + static_cast<int>(sticker->width()));
      up_wall = std::min(up_wall, pos.second);
      down_wall = std::max(down_wall, pos.second + static_cast<int>(sticker->height()));
   }

   int offsetX = -left_wall;
   int offsetY = -up_wall;

   Image canva(right_wall - left_wall, down_wall - up_wall);

   for (unsigned row = 0; row < base_.width(); row++) {
      for (unsigned col = 0; col < base_.height(); ++col) {
         canva.getPixel(row + offsetX, col + offsetY) = base_.getPixel(row, col);
      }
   }

   for (unsigned layer = 0; layer < stickers_.size(); ++layer) {
      if (stickers_[layer] == nullptr) continue;
      Image &sticker = *stickers_[layer];
      auto &pos = pos_[layer];
      for (unsigned row = 0; row < sticker.width(); ++row) {
         for (unsigned col = 0; col < sticker.height(); ++col) {
            auto &canvaPixel =
                canva.getPixel(pos.first + row + offsetX, pos.second + col + offsetY);
            auto &srcPixel = sticker.getPixel(row, col);
            if (srcPixel.a == 0) continue;
            canvaPixel = srcPixel;
         }
      }
   }

   return canva;
}
