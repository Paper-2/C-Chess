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

int isValidMove(Board *, Piece *, int[2], int[2]); // checks if the move is valid (for all pieces types)

// Check and checkmate detection
int isKingInCheck(Board *board, int color);        // checks if the king of given color is in check
int isCheckmate(Board *board, int color);          // checks if the king of given color is in checkmate
int isStalemate(Board *board, int color);          // checks if the player is in stalemate
int wouldBeInCheck(Board *board, Piece *piece, int src[2], int dest[2]); // checks if move would leave king in check

// Special move detection
int isCastlingMove(Board *board, Piece *piece, int src[2], int dest[2]); // returns 1 for kingside, 2 for queenside, 0 otherwise
int isEnPassantMove(Board *board, Piece *piece, int src[2], int dest[2]); // checks if move is en passant capture

// Execute special moves (call these after isValidMove succeeds)
void executeCastling(Board *board, int color, int side); // side: 1=kingside, 2=queenside
void executeEnPassant(Board *board, int src[2], int dest[2]); // removes the captured pawn

// Find king position
void findKing(Board *board, int color, int pos[2]); // finds king position for given color

int isSpaceFree(Board *, int[2]); // checks if the space is empty

// Castling flags: bit 0 = kingside, bit 1 = queenside
// wCastle/bCastle: 3 = both available, 2 = queenside only, 1 = kingside only, 0 = none
extern Piece initialBoardData[8][8];
extern int turn;
extern int enPassantCol;      // Column where en passant capture is possible (-1 if none)
extern int enPassantRow;      // Row of the pawn that can be captured en passant
extern int wCastle;           // White castling rights
extern int bCastle;           // Black castling rights
extern int wKingMoved;        // Track if white king has moved
extern int bKingMoved;        // Track if black king has moved
extern int wRookKMoved;       // Track if white kingside rook has moved
extern int wRookQMoved;       // Track if white queenside rook has moved  
extern int bRookKMoved;       // Track if black kingside rook has moved
extern int bRookQMoved;       // Track if black queenside rook has moved

#endif // !BOARD_H
