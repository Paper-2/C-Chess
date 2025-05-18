#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "utility.h"
#include "board.h"
#include "piece.h"

Piece initialBoardData[8][8] = {
	{0b0101, 0b1001, 0b0111, 0b0011, 0b0001, 0b0111, 0b1001, 0b0101},
	{0b1011, 0b1011, 0b1011, 0b1011, 0b1011, 0b1011, 0b1011, 0b1011},
	{EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL},
	{EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL},
	{EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL},
	{EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL},
	{0b1010, 0b1010, 0b1010, 0b1010, 0b1010, 0b1010, 0b1010, 0b1010},
	{0b0100, 0b1000, 0b0110, 0b0010, 0b0000, 0b0110, 0b1000, 0b0100}};

int turn;
int enPassant;
int wCastle;
int bCastle;

// Private function prototypes
int isValidMoveKing(Board *, Piece *, int[2], int[2]);	 // checks if the move is valid for a king
int isValidMoveQueen(Board *, Piece *, int[2], int[2]);	 // checks if the move is valid for a queen
int isValidMoveRook(Board *, Piece *, int[2], int[2]);	 // checks if the move is valid for a rook
int isValidMoveBishop(Board *, Piece *, int[2], int[2]); // checks if the move is valid for a bishop
int isValidMoveKnight(Board *, Piece *, int[2], int[2]); // checks if the move is valid for a knight
int isValidMovePawn(Board *, Piece *, int[2], int[2]);	 // checks if the move is valid for a pawn

Board *makeEmptyBoard()
{
	Board *board = malloc(sizeof(Board));
	board->grid = malloc(8 * sizeof(Piece *));

	for (int row = 0; row < 8; row++)
	{
		board->grid[row] = malloc(8 * sizeof(Piece)); // initialize each row of the grid

		for (int col = 0; col < 8; col++)
		{
			board->grid[row][col] = EMPTY_CELL; // set each cell to 0b11111111, which doesn't represent any piece.
		}
	}

	return board;
}

// Board *defaultBoard()
// { #TODO
//     return nullptr;
// }

Piece *getPieceAt(Board *board, int x, int y)
{
	//
	if (x < 0 || x > 7 || y < 0 || y > 7)
	{
		return NULL; // Return NULL if the coordinates are out of bounds
	}

	return &board->grid[7 - y][x];
}

void freeBoard(Board *board)
{
	// frees the memory allocated for the board
	for (int row = 0; row < 8; row++)
	{
		free(board->grid[row]); // free each row of the grid
	}
	free(board->grid); // free the grid itself
	free(board);	   // free the board struct
}

void printBoard(Board *board)
{
	// Human readable representation of the board for the sake of debugging and stuff
	// 0b11111111 = empty cell
	Piece pieceAtCell;
	char *pieceString;
	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			pieceAtCell = board->grid[row][col];	  // get the piece at the current cell
			pieceString = pieceToString(pieceAtCell); // convert the piece to a string
			printf("%s ", pieceString);				  // print each cell of the grid
			free(pieceString);
		}
		printf("\n"); // new line after each row
	}
}

void movePiece(Board *board, Piece *piece, int dest[2])
{
	// moves a piece from one position to another
	board->grid[dest[0]][dest[1]] = *piece; // set the destination cell to the piece
	*piece = EMPTY_CELL;					// set the source cell to an empty cell
	piece = &board->grid[dest[0]][dest[1]]; // set the piece to the destination cell
}

void setBoard(Board *board, Piece boardData[8][8])
{
	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			board->grid[row][col] = boardData[row][col];
		}
	}
}



/// @brief returns a non-zero if the piece can move to the destiny the function is further divided
/// into several other functions where each specializes on a piece type.
/// This function cannot tell whether or not a move is being blocked.
/// @param board The current state of the board
/// @param piece The piece to do the check on
/// @param src coordinates of the piece
/// @param dest coordinates of the destiny
/// @return a bool on whether or not the move is legal. False (NOT_VALID), True (VALID), Capture (CAPTURE)
int isValidMove(Board *board, Piece *piece, int src[2], int dest[2])
{

	// general cases for all pieces
	// check if the destination is within the bounds of the board
	if (dest[0] < 0 || dest[0] > 7 || dest[1] < 0 || dest[1] > 7) // this part is not necessary, but depending on how the controller is implemented, it might be useful to check.
	{
		return 0; // invalid move
	}

	Piece *pieceAtDest = &board->grid[dest[0]][dest[1]];

	if (getColor(piece) == getColor(pieceAtDest) && *pieceAtDest != EMPTY_CELL)
	{
		return 0; // invalid move, the piece is of the same color as the destination piece
	}
	int capture = 1;
	if (!areAllies(piece, pieceAtDest) && *pieceAtDest != EMPTY_CELL )
		capture = 2;

	switch (getClass(piece)) // check the piece type
	{
	case KING: // King
		return isValidMoveKing(board, piece, src, dest) * capture;
	case QUEEN: // Queen
		return isValidMoveQueen(board, piece, src, dest) * capture;
	case ROOK: // Rook
		return isValidMoveRook(board, piece, src, dest) * capture;
	case BISHOP: // Bishop
		return isValidMoveBishop(board, piece, src, dest) * capture;
	case KNIGHT: // Knight
		return isValidMoveKnight(board, piece, src, dest) * capture;
	case PAWN: // Pawn
		return isValidMovePawn(board, piece, src, dest) * capture;

	case 0b1111111: // Empty cell
		printf("cannot move an empty square type\n");
		return 0; // invalid piece type
	}
	return 0;
}

int isValidMoveKing(Board *board, Piece *, int src[2], int dest[2]) // #TODO: add the macros
{
	// L\left(X,\ Y\right) = \sqrt{\left(\left(X\left[1\right]-Y\left[1\right]\right)^{2}+\left(X\left[2\right]-Y\left[2\right]\right)^{2}\right)}
	// should work regardless of the piece color,
	// # TODO:The king can still move to a square that is attacked by an enemy piece, but this is not implemented yet.

	return ((int)sqrt(pow(src[0] - dest[0], 2) + pow(src[1] - dest[1], 2))) == 1; // check if the move is valid for a king
}
int isValidMoveQueen(Board *, Piece *, int src[2], int dest[2]) // #TODO: add the macros
{
	float slope = (float)(dest[1] - src[1]) / (float)(dest[0] - src[0]);					   // calculate the slope of the line between the two points
	return slope == 0 || slope == 1 || slope == -1 || slope == -INFINITY || slope == INFINITY; // check if the move is valid for a queen
}
int isValidMoveRook(Board *, Piece *, int src[2], int dest[2])
{
	float slope = (float)(dest[1] - src[1]) / (float)(dest[0] - src[0]); // calculate the slope of the line between the two points

	return slope == 0 || slope == -INFINITY || slope == INFINITY; // check if the move is valid for a rook
}
int isValidMoveBishop(Board *, Piece *, int src[2], int dest[2]) // #TODO: add the macros
{
	/* int slope = (float)(dest[1] - src[1]) / (float)(dest[0] - src[0]); // calculate the slope of the line between the two points

	return abs(slope) == 1; // check if the move is valid for a bishop

	*/

	// Calculate the absolute differences in rows and columns
	int rowDiff = abs(dest[0] - src[0]);
	int colDiff = abs(dest[1] - src[1]);

	// A bishop moves diagonally if the row and column differences are equal
	return rowDiff == colDiff;
}
int isValidMoveKnight(Board *, Piece *, int src[2], int dest[2]) // #TODO: add the macros
{
	/* float slope = (float)(dest[1] - src[1]) / (float)(dest[0] - src[0]);        // calculate the slope of the line between the two points
	 float distance = sqrt(pow(src[0] - dest[0], 2) + pow(src[1] - dest[1], 2));  // calculate the distance between the two points

	 return (((int)distance == 3 && abs(slope) == 3) || (abs(slope) == 1 / 3 && (int)distance == 3));*/

	// Calculate the absolute differences in rows and columns
	int rowDiff = abs(dest[0] - src[0]);
	int colDiff = abs(dest[1] - src[1]);

	// A knight moves in an "L" shape: 2 squares in one direction and 1 in the other
	return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
}
int isValidMovePawn(Board *board, Piece *piece, int src[2], int dest[2]) // #TODO: add the macros
{
	/*    int isEmpty = isSpaceFree(board, dest); // check if the destination cell is empty

	if (getColor(&board->grid[src[0]][src[1]]) == 1) // check if the pawn is black
	{
		// inverting the x and y axis for the black pawn
		src[0] = 7 - src[0];   // invert the x axis
		dest[0] = 7 - dest[0]; // invert the x axis
	}
	float slope = (float)(dest[1] - src[1]) / (float)(dest[0] - src[0]);        // calculate the slope of the line between the two points
	float distance = sqrt(pow(src[0] - dest[0], 2) + pow(src[1] - dest[1], 2)); // calculate the distance between the two points
	int x_axis = src[0];
	int y_axis = src[1];

	if (!((slope != 0 && slope != 1) ||          // check if the slope is valid for a pawn
		  (int)distance > (1 + (y_axis == 1)) || // check if the distance is valid for a pawn
		  !isEmpty))                             // check if the destination cell is empty
	{
		return 1; // invalid move, the pawn can only move forward or diagonally
	}*/

	int isEmpty = isSpaceFree(board, dest); // Check if the destination cell is empty
	int color = getColor(piece);			// Get the pawn's color (1 for black, 0 for white)

	// Determine the direction of movement based on the pawn's color
	int direction = (color == 1) ? 1 : -1;

	// Calculate the row and column differences
	int rowDiff = dest[0] - src[0];
	int colDiff = abs(dest[1] - src[1]);

	if (colDiff > 1 || rowDiff > 2)
		return 0;

	// Check for forward movement
	if (colDiff == 0 && isEmpty)
	{
		// Pawns can move forward one square, or two squares if on their starting row
		if (rowDiff == direction || (rowDiff == 2 * direction && src[0] == (color == 0 ? 6 : 1)))
		{
			return 1;
		}
	}

	// Check for diagonal capture
	if (colDiff == 1 && rowDiff == direction && !isEmpty)
	{
		return 1;
	}

	return 0;
}

int isSpaceFree(Board *board, int pos[2])
{
	return board->grid[pos[0]][pos[1]] == EMPTY_CELL; // check if the space is empty
}