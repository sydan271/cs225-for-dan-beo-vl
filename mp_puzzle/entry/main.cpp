#include "PuzzleAnimation.h"
#include "puzzle.h"

int main(int argc, const char **argv) {

    auto sol = solveAstar(
        PuzzleState({5, 1, 2, 3, 9, 10, 6, 4, 13, 0, 7, 8, 14, 15, 11, 12}),
        PuzzleState({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0}));

    /*
    * NOTE: The gif and the svg builder use different paths to the image.
    * The gif builder makes copies of the image using the workspace path, for example: 
    * "/workspaces/fa24_cs225_NETID/mp_puzzle/data/15.png" 
    * While the SVG will reference the original file and will only work if given the path
    * from the perspective of your computer NOT the workspace.
    * For example something like this should work (If you have your cs225 repo on Desktop):
    * "/Users/USERNAME/Desktop/UIUC/fa24_cs225_NETID/mp_puzzle/data/15.png"
    */
    PuzzleAnimation ani("<path to image>", sol);
    ani.writeToFile("puzzle.svg");
    return 0;
}
