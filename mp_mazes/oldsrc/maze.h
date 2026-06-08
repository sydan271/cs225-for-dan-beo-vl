/**
 * @file maze.h
 * Square Maze implementation.
 * @date 12/07/2009
 * @date 12/08/2009
 */
#pragma once

#include "cs225/HSLAPixel.h"
#include "cs225/PNG.h"
#include "dsets.h"
#include <algorithm>
#include <queue>
#include <set>
#include <vector>

/**
 * An enum is a special type representing a collection of constants.
 * See https://www.w3schools.com/cpp/cpp_enum.asp for details
 *
 * This enum is used to specify the direction of movement within the maze.
 * Each coordinate direction is associated with a fixed integer value.
 * The value is arbitrarily chosen but will be used for grading so don't modify this.
 *
 * The directions are defined as follows:
 * - RIGHT: Move to the right (Equivalent: +1 to the x coordinate) (Has value 0)
 * - DOWN: Move downward (Equivalent: +1 to the y coordinate) (Has value 1)
 * - LEFT: Move to the left (Equivalent: -1 to the x coordinate) (Has value 2)
 * - UP: Move upward (Equivalent: -1 to the y coordinate) (Has value 3)
 */
enum Direction { RIGHT = 0, DOWN = 1, LEFT = 2, UP = 3 };

/**
 * Each SquareMaze object represents a randomly-generated square maze and its
 *  solution.
 *
 * (Note that by "square maze" we mean a maze in which each cell is a square;
 *  the maze itself need not be a square.)
 */
class SquareMaze {
private:
   struct Wall {
      int x;
      int y;
      Direction dir = RIGHT;
   };

   std::vector<bool> rWalls_;
   std::vector<bool> dWalls_;

   int width;
   int height;

   unsigned getIdx(unsigned x, unsigned y) const;
   void FillRandomWalls(std::vector<Wall> &walls, int w, int h);
   void addNeighbors(std::queue<int> &q,
                     std::set<int> &visited,
                     int index,
                     std::vector<int> &pre,
                     std::vector<int> &dis);
   Direction getDir(int pred, int cur);
   void drawDirectedLine(int x, int y, Direction dir, cs225::PNG *png);
   void updatePenPosition(int &x, int &y, Direction dir);

public:
   /**
    * No-parameter constructor.
    * Creates an empty maze.
    */
   SquareMaze() = default;

   /**
    * Makes a new SquareMaze of the given height and width.
    *
    * If this object already represents a maze it will clear all the
    * existing data before doing so. You will start with a square grid (like
    * graph paper) with the specified height and width. You will select
    * random walls to delete without creating a cycle, until there are no
    * more walls that could be deleted without creating a cycle. Do not
    * delete walls on the perimeter of the grid.
    *
    * Hints: You only need to store 2 bits per square: the "down" and
    * "right" walls. The finished maze is always a tree of corridors.)
    *
    * @param width The width of the SquareMaze (number of cells)
    * @param height The height of the SquareMaze (number of cells)
    */
   void makeMaze(int width, int height);

   /**
    * This uses your representation of the maze to determine whether it is
    * possible to travel in the given direction from the square at
    * coordinates (x,y).
    *
    * For example, after makeMaze(2,2), the possible input coordinates will
    * be (0,0), (0,1), (1,0), and (1,1).
    *
    * You can not step off of the maze or through a wall.
    *
    * This function will be very helpful in solving the maze. It will also
    * be used by the grading program to verify that your maze is a tree that
    * occupies the whole grid, and to verify your maze solution. So make
    * sure that this function works!
    *
    * @param x The x coordinate of the current cell
    * @param y The y coordinate of the current cell
    * @param dir The desired direction to move from the current cell
    * @return whether you can travel in the specified direction
    */
   bool canTravel(int x, int y, Direction dir) const;

   /**
    * Sets whether or not the specified wall exists.
    *
    * This function should be fast (constant time). You can assume that in
    * grading we will not make your maze a non-tree and then call one of the
    * other member functions. setWall should not prevent cycles from
    * occurring, but should simply set a wall to be present or not present.
    * Our tests will call setWall to copy a specific maze into your
    * implementation.
    *
    * NOTE: You will only need to support setting the bottom or right walls
    * of every squares in the grid
    *
    * @param x The x coordinate of the current cell
    * @param y The y coordinate of the current cell
    * @param dir The desired direction to set the wall. This will always be RIGHT or DOWN
    * @param exists true if setting the wall to exist, false otherwise
    */
   void setWall(int x, int y, Direction dir, bool exists);

   /**
    * Solves this SquareMaze.
    *
    * For each square on the bottom row (maximum y coordinate), there is a
    * distance from the starting point (startX, 0) (i.e. any position on the
    * top row), which is defined as the length (measured as a number of steps)
    * of the only path through the maze from the starting point to that square.
    *
    * Select the square in the bottom row with the largest distance from the
    * starting point as the destination of the maze. `solveMaze()` returns
    * the winning path from the origin to the destination as a vector of
    * directions.
    *
    * If multiple paths of maximum length exist, use the one with the
    * destination cell that has the smallest x value.
    *
    * HINT: this function should run in time linear in the number of cells
    * in the maze.
    *
    * @return a vector of directions taken to solve the maze
    */
   std::vector<Direction> solveMaze(int startX);

   /**
    * Draws the maze without the solution.
    *
    * First, create a new PNG. Set the dimensions of the PNG to
    * (width*10+1,height*10+1). where height and width were the arguments to
    * makeMaze. Blacken the entire topmost row and leftmost column of
    * pixels, except for the entrance. Instead make a gap above the specified
    * start point defined as the inclusive region from ((start*10)+1, 0) to ((start+1)*10-1, 0).
    * [The gap is the pixels larger than start*10 and smaller than (start+1)*10 ]
    * For each square in the maze, call its maze coordinates (x,y). If the right wall exists,
    * then blacken the pixels with coordinates ((x+1)*10,y*10+k) for k from
    * 0 to 10. If the bottom wall exists, then blacken the pixels with
    * coordinates (x*10+k, (y+1)*10) for k from 0 to 10.
    *
    * The resulting PNG will look like the sample image, except there will
    * be no exit from the maze and the red line will be missing.
    *
    * @return a PNG of the unsolved SquareMaze
    */
   cs225::PNG *drawMaze(int start) const;

   /**
    * This function calls drawMaze, then solveMaze; it modifies the PNG
    * from drawMaze to show the solution vector and the exit.
    *
    * Start at the middle of the square specified by (start, 0). For example if start was 0
    * the start position of the solution is (5,5). Each direction in the solution vector corresponds
    * to a trail of 11 red pixels in the given direction. If the first step is downward,
    * color pixels (5,5) through (5,15) red. (Red is 0,1,0.5,1 in HSLA).
    * Then if the second step is right, color pixels (5,15) through (15,15) red.
    * Then if the third step is up, color pixels (15,15) through (15,5) red.
    * Continue in this manner until you get to the end of the solution vector.
    *
    * Make the exit by undoing the bottom wall of the destination square:
    * call the destination maze coordinates (x,y), and whiten the pixels
    * with coordinates (x*10+k, (y+1)*10) for k from 1 to 9.
    *
    * @return a PNG of the solved SquareMaze
    */
   cs225::PNG *drawMazeWithSolution(int start);
};
