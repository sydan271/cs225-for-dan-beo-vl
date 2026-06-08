/**
 * @file PuzzleAnimation.h
 */

#pragma once

#include "puzzle.h"

#include <array>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

/**
 * Puzzle animation utility class
 */
class PuzzleAnimation {
  public:
    /**
     * Constructs an animated puzzle with numeric board.
     * @param transitions same as the results from solveAstar/solveBFS
     */
    PuzzleAnimation(std::vector<PuzzleState> const &transitions)
        : transitions_(transitions) {}

    /**
     * Constructs an animated puzzle with an image board.
     * @param img_path path the image used for animation
     * @param transitions same as the results from solveAstar/solveBFS
     */
    PuzzleAnimation(std::string const &img_path,
                    std::vector<PuzzleState> const &transitions)
        : transitions_(transitions), imgPath_(img_path) {}

    /**
     * Writes the animation to an SVG or GIF file based on the file
     * extension (.svg/.gif). You can use any modern browser, or the
     * SVG VSCode extension to view the SVG. If you want to build a GIF 
     * of a puzzle where the base image itself is an animated GIF, you 
     * must use the SVG construction. (writeToGifFile() cannot handle GIFs as input)
     * @param fname name of the file
     */
    void writeToFile(std::string fname);

    /** width of the svg */
    double width = 200;
    /** height of the svg */
    double height = 200;
    /** size of each tile */
    double tileSize = std::min(width, height) / 4;
    /** duration of each animation step in seconds */
    double stepDuration = 0.3;
    /** frames per second */
    double fps = 12;
    /** background color */
    std::string bgc = "#808080";

  private:
    void writeToSvgFile(std::string const&, std::vector<int> const&);
    void writeToGifFile(std::string const&, std::vector<int> const&);

    std::array<std::stringstream, 16> createAnimationTags(
        std::vector<int> const &directions,
        std::function<double(int)> offsetX = [](int) { return 0; },
        std::function<double(int)> offsetY = [](int) { return 0; });

    bool isValid(int pos, int dir);
    std::vector<PuzzleState> transitions_;
    std::string imgPath_;
};
