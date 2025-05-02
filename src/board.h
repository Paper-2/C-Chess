#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
typedef struct Board
{
	Piece **grid; // 2D array of chars. grid is a pointer to a pointer. See boardExample for how the grid looks like.
} Board;

Board *makeEmptyBoard(); 							// creates an empty board and returns a pointer to it
Board *defaultBoard();	 							// returns a default board with pieces in starting positions. returns a pointer to it

void freeBoard(Board *);
void printBoard(Board *);
void movePiece(Board *, Piece *, int[2]); 			// moves a piece from one position to another. sets the destination cell to the piece and the source cell to an empty cell.
void setBoard(Board *, Piece[8][8]);		  		// sets the board based on the fen string

int isvalidMove(Board *, Piece *, int[2],  int[2]); 	// checks if the move is valid (for all pieces types)

int isvalidMoveKing(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a king
int isvalidMoveQueen(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a queen
int isvalidMoveRook(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a rook
int isvalidMoveBishop(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a bishop
int isvalidMoveKnight(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a knight
int isvalidMovePawn(Board *, Piece *, int[2],  int[2]);	// checks if the move is valid for a pawn

int isSpaceFree(Board *, int[2]);					// checks if the space is empty

#endif // !BOARD_H
