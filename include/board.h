#ifndef BOARD_H
#define BOARD_H
#define EMPTY_CELL 0b11111111 // empty cell
#define wKING 0b00000000	  // white king
#define wQUEEN 0b0010
#define wROOK 0b0100
#define wBISHOP 0b0110
#define wKNIGHT 0b1000
#define wPAWN 0b1010
#define bKING 0b00000001 // black king
#define bQUEEN 0b0011
#define bROOK 0b0101
#define bBISHOP 0b0111
#define bKNIGHT 0b1001
#define bPAWN 0b1011


//piece class

#define KING 0b000   // king piece
#define QUEEN 0b001  // queen piece
#define ROOK 0b010   // rook piece
#define BISHOP 0b011 // bishop piece
#define KNIGHT 0b100 // knight piece
#define PAWN 0b101   // pawn piece

#include "piece.h"
typedef struct Board
{
	Piece **grid; // 2D array of chars. grid is a pointer to a pointer. See boardExample for how the grid looks like.
} Board;

Board *makeEmptyBoard(); // creates an empty board and returns a pointer to it
Board *defaultBoard();	 // returns a default board with pieces in starting positions. returns a pointer to it

Piece *getPieceAt(Board *, int x, int y);

void freeBoard(Board *);
void printBoard(Board *);
void movePiece(Board *, Piece *, int[2]); // moves a piece from one position to another. sets the destination cell to the piece and the source cell to an empty cell.
void setBoard(Board *, Piece[8][8]);	  // sets the board based on the fen string


#define NOT_VALID 0
#define VALID 1
#define CAPTURE 2

int isValidMove(Board *, Piece *, int[2], int[2]); // checks if the move is valid (for all pieces types)


int isSpaceFree(Board *, int[2]); // checks if the space is empty
extern Piece initialBoardData[8][8];
extern int turn;
extern int enPassant;
extern int wCastle;
extern int bCastle;

#endif // !BOARD_H
