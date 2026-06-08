#include "Image.h"
#include "StickerSheet.h"

int main() {
  //
  // Reminder:
  //   Before exiting main, save your creation to disk as myImage.png
  //
  std::cout << "Program running...\n";

  Image base;
  if (!base.readFromFile("../data/alma.png")) std::cerr << "Unable to read alma.png\n";

  Image layer1;
  if (!layer1.readFromFile("../data/i.png")) std::cerr << "Unable to read i.png\n";
  Image layer2;
  if (!layer2.readFromFile("../data/penguin.png")) std::cerr << "Unable to read penguin.png\n";
  Image layer3(layer1);

  StickerSheet sheet(base);
  sheet.addSticker(layer1, 100, 100);
  sheet.addSticker(layer2, -1000, 100);
  sheet.addSticker(layer3, 100, -900);

  std::cout << "Layer: " << (sheet.layers()) << std::endl;

  sheet.render().writeToFile("test.png");


  return 0;
}
