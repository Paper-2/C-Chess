

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "piece.h"

const char *colors[] = {"White", "Black"};
const char *classes[] = {"King", "Queen", "Rook", "Bishop", "Knight", "Pawn"};

inline void setPieceBitMask(Piece *bitmask, uint8_t pieceBitmask)
{
	// changes the bitmask of a piece to a given value
	*bitmask = pieceBitmask;
}
inline void setColor(Piece *bitmask, uint8_t color)
{
	// sets the color
	*bitmask = (*bitmask & 0b11110) | color; // bits 1 to 3 will be kept the same, bit 0 will not.
}
inline void setClass(Piece *bitmask, uint8_t pieceType)
{
	// sets the class
	*bitmask = (*bitmask & 0b0001) | (pieceType << 1); // bit 0 will be kept the same, bits 1 to 3 will be changed.
}

void printPiece(Piece *bitmask)
{
	// prints the piece

	uint8_t color = getColor(bitmask);
	uint8_t classType = getClass(bitmask) >> 1;

	printf("Piece: %d\n", *bitmask);
	printf("Color: %s\n", colors[color]);
	printf("Class: %s\n", classes[classType]);
}

inline void killPiece(Piece *bitmask)
{
	// kills a piece by deleting it
	free(bitmask);
}

inline int getColor(Piece *bitmask)
{
	// gets the color of a piece
	return *bitmask & 0b0001;
}
inline int getClass(Piece *bitmask)
{
	// gets the type of a piece
	return *bitmask & 0b1110;
}

// int main()
// {
// 	Piece *piece = malloc(sizeof(Piece));
// 	setPieceBitMask(piece, 0b0000);
// 	setColor(piece, 1);
// 	setClass(piece, 2);
// 	printPiece(piece);
// 	getColor(piece);
// 	killPiece(piece);
// 	return 0;
// }