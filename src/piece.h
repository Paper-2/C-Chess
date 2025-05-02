

#ifndef PIECE_H
#define PIECE_H

// Piece is an integer. It is used to represent a piece on the board.
// The value of the piece is the index of the piece in the pieces array.
// The pieces array is a 2D array of chars. The first dimension is the number of pieces.
// The second dimension is the size of each piece. The size of each piece is the number of squares in the piece.
// The number of squares in the piece is the number of squares in the grid. The grid is a pointer to a pointer.
// See boardExample for how the grid looks like.
#include <stdint.h>
typedef uint8_t Piece;

void setPieceBitMask(Piece *, uint8_t); // changes the bitmask of a piece to a given value
void setColor(Piece *, uint8_t);        // sets the color
void setClass(Piece *, uint8_t);        // sets the class

void killPiece(Piece *); // kills a piece by deleting it and deallocating the memory

uint8_t getColor(Piece *); // gets the color of a piece
uint8_t getClass(Piece *); // gets the type of a piece

#endif // !Piece