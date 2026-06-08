#include "PuzzleAnimation.h"
#include "cs225/PNG.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>

static std::array<int, 4> offsets = {1, 4, -1, -4};

bool exists(std::string const &path) {
    // Try stat-ing it
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    // Check for read permission
    if ((st.st_mode & S_IRUSR) == 0)
        return false;

    // Check for correct file/directory nature
    if (path[path.length() - 1] != '/')
        return S_ISREG(st.st_mode);

    // Otherwise we want a directory
    if ((st.st_mode & S_IXUSR) == 0)
        return false;
    return S_ISDIR(st.st_mode);
}

void PuzzleAnimation::writeToFile(std::string fname) {
    assert(not transitions_.empty());

    // 0: → 1: ↓ 2: ← 3: ↑
    // this is actually the direction moved by the empty space on the puzzle
    // unlike the mp implementation, it uses int instead of enums for simplicity.
    std::vector<int> directions;

    for (size_t i = 0; i < transitions_.size() - 1; i++) {
        auto s1 = transitions_[i];
        auto s2 = transitions_[i + 1];
        auto a1 = s1.asArray();
        auto a2 = s2.asArray();
        auto pos1 = std::find(a1.begin(), a1.end(), 0) - a1.begin();
        auto pos2 = std::find(a2.begin(), a2.end(), 0) - a2.begin();
        auto dir = std::find(offsets.begin(), offsets.end(), pos2 - pos1) -
                   offsets.begin();
        directions.push_back(dir);
    }

    if (fname.length() < 4)
        fname += ".svg";

    if (fname.substr(fname.length() - 4, 4) == ".svg") {
        writeToSvgFile(fname, directions);
    } else if (fname.substr(fname.length() - 4, 4) == ".gif") {
        writeToGifFile(fname, directions);
    } else {
        throw std::invalid_argument("Unsupported file type for file " + fname);
    }
}

void PuzzleAnimation::writeToSvgFile(std::string const &fname,
                                     std::vector<int> const &directions) {
    std::array<char, 16> initialState = transitions_[0].asArray();
    std::ofstream f(fname);

    f << "<svg width=\"" << width << "\" height=\"" << height
      << "\" xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;

    if (imgPath_.empty()) {
        auto textOffset = [=](int) { return tileSize / 2; };
        std::array<std::stringstream, 16> textAnimationTags =
            createAnimationTags(directions, textOffset, textOffset);
        std::array<std::stringstream, 16> rectAnimationTags =
            createAnimationTags(directions);

        f << "<rect x=\"0\" y=\"0\" width=\"" << width << "\" height=\""
          << height << "\" style =\"fill:" << bgc << ";\"/>" << std::endl;
        f << "<style>rect{fill:white;stroke:black;stroke-width:2;}text{"
             "text-anchor:middle;font-family:monospace;dominant-baseline:"
             "middle;}</style>"
          << std::endl;

        for (int i = 0; i < 16; i++) {
            int tile = initialState[i];
            if (tile == 0)
                continue;
            int pos = i;
            int x = pos % 4;
            int y = pos / 4;
            f << "<rect x=\"" << x * tileSize << "\" y=\"" << y * tileSize
              << "\" width=\"" << tileSize << "\" height=\"" << tileSize
              << "\">" << std::endl;
            f << rectAnimationTags[tile].str();
            f << "</rect>" << std::endl;

            f << "<text x=\"" << x * tileSize + tileSize / 2 << "\" y=\""
              << y * tileSize + tileSize / 2 << "\" font-size=\""
              << std::min(width, height) / 10 << "\">" << tile << std::endl;
            f << textAnimationTags[tile].str();
            f << "</text>" << std::endl;
        }
    } else {
        auto imgOffsetX = [=](int pos) {
            return -(((pos - 1) % 4) * tileSize);
        };
        auto imgOffsetY = [=](int pos) {
            return -((int)((pos - 1) / 4) * tileSize);
        };

        std::array<std::stringstream, 16> maskAnimationTags =
            createAnimationTags(directions);
        std::array<std::stringstream, 16> imgAnimationTags =
            createAnimationTags(directions, imgOffsetX, imgOffsetY);
        f << "<rect x=\"0\" y=\"0\" width=\"" << width << "\" height=\""
          << height << "\" style =\"fill:" << bgc << ";\"/>" << std::endl;
        for (int i = 0; i < 16; i++) {
            int tile = initialState[i];
            if (tile == 0)
                continue;

            int pos = i;
            int x = pos % 4;
            int y = pos / 4;
            f << "<mask id=\"mask-" << tile << "\"><rect x=\"" << x * tileSize
              << "\" y=\"" << y * tileSize << "\" width=\"" << tileSize
              << "\" height=\"" << tileSize << "\" fill=\"white\">"
              << std::endl;
            f << maskAnimationTags[tile].str();
            f << "</rect></mask>" << std::endl;

            f << "<image x=\"" << x * tileSize + imgOffsetX(tile) << "\" y=\""
              << y * tileSize + imgOffsetY(tile) << "\" width=\"" << width
              << "\" height=\"" << height << "\" href=\"" << imgPath_
              << "\" mask=\"url(#mask-" << tile << ")\">" << std::endl;
            f << imgAnimationTags[tile].str();
            f << "</image>" << std::endl;
        }
    }
    f << "</svg>";
}

double lerp(double a, double b, double t) { return a + t * (b - a); }

void PuzzleAnimation::writeToGifFile(std::string const &fname,
                                     std::vector<int> const &directions) {
    std::array<char, 16> initialState = transitions_[0].asArray();
    std::array<char, 16> board = initialState;
    std::array<int, 16> positions;
    std::array<std::stringstream, 16> out;

    for (int i = 0; i < 16; i++) {
        positions[initialState[i]] = i;
    }

    auto it0 = std::find(initialState.begin(), initialState.end(), 0);
    // must be a zero on board
    assert(it0 != initialState.end());
    int pos0 = it0 - initialState.begin();
    if (!exists("frames"))
        mkdir("frames", 0700);

    system("rm frames/*.png");

    auto imgPath = imgPath_.empty() ? "../data/15.png" : imgPath_;

    system(("convert " + imgPath + " -crop 4x4@ +repage +adjoin frames/tile_%d.png").c_str());
    system(("convert frames/tile_15.png -fill \"" + bgc + "\" -draw 'color 0,0 reset'  PNG32:frames/empty.png").c_str());

    double tw, th;
    {
        cs225::PNG img;
        img.readFromFile(imgPath);
        tw = (double)img.width() / 4.;
        th = (double)img.height() / 4.;
    }

    std::vector<std::string> tilePngs;
    for (auto tile : initialState) {
        if (tile == 0)
            tile = 16;
        tilePngs.push_back("frames/tile_" + std::to_string(tile - 1) + ".png");
    }
    auto tilePngEmpty = "frames/empty.png";
    double frameInterval = 1 / fps;
    int nFrames = directions.size() * ceil(stepDuration / frameInterval);
    int padWidth = std::to_string(nFrames).length();

    int frameCnt = 0;
    double duration = 0;

    for (int dir : directions) {
        // moves must be valid
        assert(isValid(pos0, dir));

        int tileToMove = board[pos0 + offsets[dir]];
        int originalPos = positions[tileToMove];
        int originalX = originalPos % 4;
        int originalY = originalPos / 4;
        int newPos = originalPos + offsets[(dir + 2) % 4];
        int newX = newPos % 4;
        int newY = newPos / 4;

        std::stringstream cmd;
        cmd << "montage ";
        for (int tile : board) {
            if (tile == tileToMove || tile == 0)
                cmd << tilePngEmpty << " ";
            else
                cmd << "frames/tile_" << (tile - 1) << ".png ";
        }
        cmd << "-tile 4x4 -geometry +0+0 frames/canvas.png";
        system(cmd.str().c_str());

        while (duration <= stepDuration) {
            int x = round(lerp(originalX * tw, newX * tw, duration / stepDuration));
            int y = round(lerp(originalY * th, newY * th, duration / stepDuration));
            std::stringstream cmd;
            cmd << "composite -geometry +" << x << "+" << y << " frames/tile_"
                << (tileToMove - 1) << ".png frames/canvas.png frames/frame_";
            cmd.fill('0');
            cmd.width(padWidth);
            cmd << frameCnt++ << ".png";
            system(cmd.str().c_str());
            duration += frameInterval;
        }
        duration -= stepDuration;

        std::swap(board[pos0], board[positions[tileToMove]]);
        std::swap(tilePngs[pos0], tilePngs[positions[tileToMove]]);
        positions[tileToMove] = newPos;
        pos0 += offsets[dir];
    }

    system(("convert -delay " + std::to_string(round(frameInterval * 100)) +
            " frames/frame_*.png " + fname)
               .c_str());
}

std::array<std::stringstream, 16>
PuzzleAnimation::createAnimationTags(std::vector<int> const &directions,
                                     std::function<double(int)> offsetX,
                                     std::function<double(int)> offsetY) {
    std::array<char, 16> initialState = transitions_[0].asArray();

    std::array<char, 16> board = initialState;
    std::array<int, 16> positions;
    std::array<std::stringstream, 16> out;

    for (int i = 0; i < 16; i++) {
        positions[initialState[i]] = i;
    }

    auto it0 = std::find(initialState.begin(), initialState.end(), 0);
    // must be a zero on board
    assert(it0 != initialState.end());
    int pos0 = it0 - initialState.begin();

    int i = 0;
    for (int dir : directions) {
        // moves must be valid
        assert(isValid(pos0, dir));

        int tileToMove = board[pos0 + offsets[dir]];
        int originalPos = positions[tileToMove];
        int originalX = originalPos % 4;
        int originalY = originalPos / 4;
        int newPos = originalPos + offsets[(dir + 2) % 4];
        int newX = newPos % 4;
        int newY = newPos / 4;
        double offX = offsetX(tileToMove);
        double offY = offsetY(tileToMove);

        std::string axis = dir % 2 ? "y" : "x";
        if ((dir % 2)) {
            std::swap(originalX, originalY);
            std::swap(newX, newY);
            std::swap(offX, offY);
        }

        out[tileToMove] << "<animate attributeName=\"" << axis << "\" from=\""
                        << originalX * tileSize + offX << "\" to=\""
                        << newX * tileSize + offX << "\" begin=\""
                        << i * stepDuration << "s\" dur=\"" << stepDuration
                        << "s\" fill=\"freeze\" />" << std::endl;

        std::swap(board[pos0], board[positions[tileToMove]]);
        positions[tileToMove] = newPos;
        pos0 += offsets[dir];
        i++;
    }

    return out;
}

bool PuzzleAnimation::isValid(int pos, int dir) {
    return (dir == 0 && pos % 4 != 3) || (dir == 1 && pos < 12) ||
           (dir == 2 && pos % 4 != 0) || (dir == 3 && pos > 3);
}
