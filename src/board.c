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
int enPassantCol = -1;    // Column where en passant capture is possible (-1 if none)
int enPassantRow = -1;    // Row of the pawn that can be captured en passant
int wCastle = 3;          // White castling rights (3 = both)
int bCastle = 3;          // Black castling rights (3 = both)
int wKingMoved = 0;
int bKingMoved = 0;
int wRookKMoved = 0;      // White kingside rook (h1)
int wRookQMoved = 0;      // White queenside rook (a1)
int bRookKMoved = 0;      // Black kingside rook (h8)
int bRookQMoved = 0;      // Black queenside rook (a8)

// Private function prototypes
int isValidMoveKing(Board *board, Piece *piece, int src[2], int dest[2]);   // checks if the move is valid for a king
int isValidMoveQueen(Board *board, Piece *piece, int src[2], int dest[2]);  // checks if the move is valid for a queen
int isValidMoveRook(Board *board, Piece *piece, int src[2], int dest[2]);   // checks if the move is valid for a rook
int isValidMoveBishop(Board *board, Piece *piece, int src[2], int dest[2]); // checks if the move is valid for a bishop
int isValidMoveKnight(Board *board, Piece *piece, int src[2], int dest[2]); // checks if the move is valid for a knight
int isValidMovePawn(Board *board, Piece *piece, int src[2], int dest[2]);   // checks if the move is valid for a pawn
static int isPathClear(Board *board, int src[2], int dest[2]);              // checks if path is clear
static int isSquareAttacked(Board *board, int row, int col, int byColor);   // checks if square is attacked by color

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
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return NULL; // Return NULL if the coordinates are out of bounds
    }

    return &board->grid[7-y][x];
}

void freeBoard(Board *board)
{
    // frees the memory allocated for the board
    for (int row = 0; row < 8; row++)
    {
        free(board->grid[row]); // free each row of the grid
    }
    free(board->grid); // free the grid itself
    free(board);       // free the board struct
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
            pieceAtCell = board->grid[row][col];      // get the piece at the current cell
            pieceString = pieceToString(pieceAtCell); // convert the piece to a string
            printf("%s ", pieceString);               // print each cell of the grid
            free(pieceString);
        }
        printf("\n"); // new line after each row
    }
}

void movePiece(Board *board, Piece *piece, int dest[2])
{
    // moves a piece from one position to another
    board->grid[dest[0]][dest[1]] = *piece; // set the destination cell to the piece
    *piece = EMPTY_CELL;                    // set the source cell to an empty cell
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

int isValidMove(Board *board, Piece *piece, int src[2], int dest[2])
{

    // general case for all pieces
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

    switch (getClass(piece)) // check the piece type
    {
    case KING: // King
        return isValidMoveKing(board, piece, src, dest);
    case QUEEN: // Queen
        return isValidMoveQueen(board, piece, src, dest);
    case ROOK: // Rook
        return isValidMoveRook(board, piece, src, dest);
    case BISHOP: // Bishop
        return isValidMoveBishop(board, piece, src, dest);
    case KNIGHT: // Knight
        return isValidMoveKnight(board, piece, src, dest);
    case PAWN: // Pawn
        return isValidMovePawn(board, piece, src, dest);

    case 0b1111111: // Empty cell
        printf("cannot move an empty square type\n");
        return 0; // invalid piece type
    }
    return 0;
}

// Helper function to check if path is clear for sliding pieces (rook, bishop, queen)
static int isPathClear(Board *board, int src[2], int dest[2])
{
    int rowDir = 0, colDir = 0;
    
    // Determine direction of movement
    if (dest[0] > src[0]) rowDir = 1;
    else if (dest[0] < src[0]) rowDir = -1;
    
    if (dest[1] > src[1]) colDir = 1;
    else if (dest[1] < src[1]) colDir = -1;
    
    // Check each square between src and dest (exclusive)
    int row = src[0] + rowDir;
    int col = src[1] + colDir;
    
    while (row != dest[0] || col != dest[1])
    {
        if (board->grid[row][col] != EMPTY_CELL)
        {
            return 0; // Path is blocked
        }
        row += rowDir;
        col += colDir;
    }
    
    return 1; // Path is clear
}

// Helper function to check if a square is attacked by a given color
// This is used for check detection and castling validation
static int isSquareAttacked(Board *board, int row, int col, int byColor)
{
    // Check for pawn attacks
    int pawnDir = (byColor == 1) ? -1 : 1; // Direction pawns attack from
    int pawnRow = row + pawnDir;
    if (pawnRow >= 0 && pawnRow <= 7)
    {
        if (col - 1 >= 0)
        {
            Piece *p = &board->grid[pawnRow][col - 1];
            if (*p != EMPTY_CELL && getColor(p) == byColor && getClass(p) == PAWN)
                return 1;
        }
        if (col + 1 <= 7)
        {
            Piece *p = &board->grid[pawnRow][col + 1];
            if (*p != EMPTY_CELL && getColor(p) == byColor && getClass(p) == PAWN)
                return 1;
        }
    }
    
    // Check for knight attacks
    int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (int i = 0; i < 8; i++)
    {
        int r = row + knightMoves[i][0];
        int c = col + knightMoves[i][1];
        if (r >= 0 && r <= 7 && c >= 0 && c <= 7)
        {
            Piece *p = &board->grid[r][c];
            if (*p != EMPTY_CELL && getColor(p) == byColor && getClass(p) == KNIGHT)
                return 1;
        }
    }
    
    // Check for king attacks (adjacent squares)
    for (int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; dc++)
        {
            if (dr == 0 && dc == 0) continue;
            int r = row + dr;
            int c = col + dc;
            if (r >= 0 && r <= 7 && c >= 0 && c <= 7)
            {
                Piece *p = &board->grid[r][c];
                if (*p != EMPTY_CELL && getColor(p) == byColor && getClass(p) == KING)
                    return 1;
            }
        }
    }
    
    // Check for rook/queen attacks (straight lines)
    int straightDirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (int i = 0; i < 4; i++)
    {
        int r = row + straightDirs[i][0];
        int c = col + straightDirs[i][1];
        while (r >= 0 && r <= 7 && c >= 0 && c <= 7)
        {
            Piece *p = &board->grid[r][c];
            if (*p != EMPTY_CELL)
            {
                if (getColor(p) == byColor && (getClass(p) == ROOK || getClass(p) == QUEEN))
                    return 1;
                break; // Blocked by a piece
            }
            r += straightDirs[i][0];
            c += straightDirs[i][1];
        }
    }
    
    // Check for bishop/queen attacks (diagonals)
    int diagDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    for (int i = 0; i < 4; i++)
    {
        int r = row + diagDirs[i][0];
        int c = col + diagDirs[i][1];
        while (r >= 0 && r <= 7 && c >= 0 && c <= 7)
        {
            Piece *p = &board->grid[r][c];
            if (*p != EMPTY_CELL)
            {
                if (getColor(p) == byColor && (getClass(p) == BISHOP || getClass(p) == QUEEN))
                    return 1;
                break; // Blocked by a piece
            }
            r += diagDirs[i][0];
            c += diagDirs[i][1];
        }
    }
    
    return 0;
}

int isValidMoveKing(Board *board, Piece *piece, int src[2], int dest[2])
{
    int color = getColor(piece);
    int rowDiff = abs(dest[0] - src[0]);
    int colDiff = abs(dest[1] - src[1]);
    
    // Normal king move: exactly one square in any direction
    if (rowDiff <= 1 && colDiff <= 1 && (rowDiff + colDiff) > 0)
    {
        return 1;
    }
    
    // Castling: king moves 2 squares horizontally
    if (rowDiff == 0 && colDiff == 2)
    {
        int kingRow = (color == 0) ? 7 : 0; // White king on row 7, black on row 0
        
        // King must be on starting position
        if (src[0] != kingRow || src[1] != 4)
            return 0;
        
        // Check if king has moved
        if ((color == 0 && wKingMoved) || (color == 1 && bKingMoved))
            return 0;
        
        // King cannot castle out of check
        int enemyColor = 1 - color;
        if (isSquareAttacked(board, src[0], src[1], enemyColor))
            return 0;
        
        // Determine if kingside (dest col = 6) or queenside (dest col = 2)
        if (dest[1] == 6)
        {
            // Kingside castling
            if ((color == 0 && wRookKMoved) || (color == 1 && bRookKMoved))
                return 0;
            
            // Check rook is in place
            if (board->grid[kingRow][7] == EMPTY_CELL || 
                getClass(&board->grid[kingRow][7]) != ROOK ||
                getColor(&board->grid[kingRow][7]) != color)
                return 0;
            
            // Path must be clear (f1/f8 and g1/g8)
            if (board->grid[kingRow][5] != EMPTY_CELL || board->grid[kingRow][6] != EMPTY_CELL)
                return 0;
            
            // Cannot castle through check (f1/f8) or into check (g1/g8)
            if (isSquareAttacked(board, kingRow, 5, enemyColor) ||
                isSquareAttacked(board, kingRow, 6, enemyColor))
                return 0;
            
            return 1; // Kingside castling is valid
        }
        else if (dest[1] == 2)
        {
            // Queenside castling
            if ((color == 0 && wRookQMoved) || (color == 1 && bRookQMoved))
                return 0;
            
            // Check rook is in place
            if (board->grid[kingRow][0] == EMPTY_CELL ||
                getClass(&board->grid[kingRow][0]) != ROOK ||
                getColor(&board->grid[kingRow][0]) != color)
                return 0;
            
            // Path must be clear (b1/b8, c1/c8, d1/d8)
            if (board->grid[kingRow][1] != EMPTY_CELL || 
                board->grid[kingRow][2] != EMPTY_CELL ||
                board->grid[kingRow][3] != EMPTY_CELL)
                return 0;
            
            // Cannot castle through check (c1/c8, d1/d8) or into check (c1/c8)
            if (isSquareAttacked(board, kingRow, 2, enemyColor) ||
                isSquareAttacked(board, kingRow, 3, enemyColor))
                return 0;
            
            return 1; // Queenside castling is valid
        }
    }
    
    return 0;
}

int isValidMoveQueen(Board *board, Piece *piece, int src[2], int dest[2])
{
    int rowDiff = abs(dest[0] - src[0]);
    int colDiff = abs(dest[1] - src[1]);
    
    // Queen moves like rook (straight) or bishop (diagonal)
    // Straight: one of the diffs must be 0
    // Diagonal: both diffs must be equal
    if (!((rowDiff == 0 || colDiff == 0) || (rowDiff == colDiff)))
    {
        return 0; // Invalid direction
    }
    
    // Check if path is clear
    return isPathClear(board, src, dest);
}

int isValidMoveRook(Board *board, Piece *piece, int src[2], int dest[2])
{
    int rowDiff = abs(dest[0] - src[0]);
    int colDiff = abs(dest[1] - src[1]);
    
    // Rook moves in straight lines only (horizontal or vertical)
    if (rowDiff != 0 && colDiff != 0)
    {
        return 0; // Invalid direction - must be straight line
    }
    
    // Check if path is clear
    return isPathClear(board, src, dest);
}

int isValidMoveBishop(Board *board, Piece *piece, int src[2], int dest[2])
{
    int rowDiff = abs(dest[0] - src[0]);
    int colDiff = abs(dest[1] - src[1]);

    // Bishop moves diagonally - row and column differences must be equal
    if (rowDiff != colDiff || rowDiff == 0)
    {
        return 0; // Invalid direction
    }
    
    // Check if path is clear
    return isPathClear(board, src, dest);
}

int isValidMoveKnight(Board *board, Piece *piece, int src[2], int dest[2])
{
    // Calculate the absolute differences in rows and columns
    int rowDiff = abs(dest[0] - src[0]);
    int colDiff = abs(dest[1] - src[1]);

    // A knight moves in an "L" shape: 2 squares in one direction and 1 in the other
    // Knights can jump over pieces, so no path checking needed
    return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
}

int isValidMovePawn(Board *board, Piece *piece, int src[2], int dest[2])
{
    int isEmpty = isSpaceFree(board, dest); // Check if the destination cell is empty
    int color = getColor(piece); // Get the pawn's color (1 for black, 0 for white)

    // Direction: white pawns move from row 6 toward row 0, black pawns move from row 1 toward row 7
    // In board coordinates: white moves with decreasing row (-1), black with increasing row (+1)
    int direction = (color == 1) ? 1 : -1;

    // Calculate the row and column differences
    int rowDiff = dest[0] - src[0];
    int colDiff = abs(dest[1] - src[1]);
    
    // Must move in the correct direction
    if ((direction == 1 && rowDiff <= 0) || (direction == -1 && rowDiff >= 0))
    {
        return 0;
    }

    // Check for forward movement (must be to empty square)
    if (colDiff == 0)
    {
        // Single square forward
        if (rowDiff == direction && isEmpty)
        {
            return 1;
        }
        
        // Two squares forward from starting position
        int startingRow = (color == 0) ? 6 : 1; // White starts at row 6, black at row 1
        if (rowDiff == 2 * direction && src[0] == startingRow && isEmpty)
        {
            // Check that the intermediate square is also empty
            int midRow = src[0] + direction;
            if (board->grid[midRow][src[1]] == EMPTY_CELL)
            {
                return 1;
            }
        }
        return 0;
    }

    // Check for diagonal capture (must capture an enemy piece)
    if (colDiff == 1 && rowDiff == direction && !isEmpty)
    {
        return 1;
    }
    
    // En passant capture
    if (colDiff == 1 && rowDiff == direction && isEmpty)
    {
        // Check if en passant is available and this is the correct square
        if (enPassantCol != -1 && dest[1] == enPassantCol)
        {
            // The en passant target row depends on color
            // White captures en passant on row 2 (moving to row 2, capturing pawn on row 3)
            // Black captures en passant on row 5 (moving to row 5, capturing pawn on row 4)
            int enPassantTargetRow = (color == 0) ? 2 : 5;
            if (dest[0] == enPassantTargetRow && enPassantRow == src[0])
            {
                return 1; // Valid en passant capture
            }
        }
    }

    return 0;
}

int isSpaceFree(Board *board, int pos[2])
{
    return board->grid[pos[0]][pos[1]] == EMPTY_CELL; // check if the space is empty
}

// Find the king's position for a given color
void findKing(Board *board, int color, int pos[2])
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            Piece *p = &board->grid[row][col];
            if (*p != EMPTY_CELL && getColor(p) == color && getClass(p) == KING)
            {
                pos[0] = row;
                pos[1] = col;
                return;
            }
        }
    }
    // King not found (shouldn't happen in valid game)
    pos[0] = -1;
    pos[1] = -1;
}

// Check if the king of the given color is in check
int isKingInCheck(Board *board, int color)
{
    int kingPos[2];
    findKing(board, color, kingPos);
    
    if (kingPos[0] == -1) return 0; // King not found
    
    int enemyColor = 1 - color;
    return isSquareAttacked(board, kingPos[0], kingPos[1], enemyColor);
}

// Check if a move would leave the player's king in check
int wouldBeInCheck(Board *board, Piece *piece, int src[2], int dest[2])
{
    int color = getColor(piece);
    
    // Make a temporary move
    Piece srcPiece = board->grid[src[0]][src[1]];
    Piece destPiece = board->grid[dest[0]][dest[1]];
    
    board->grid[dest[0]][dest[1]] = srcPiece;
    board->grid[src[0]][src[1]] = EMPTY_CELL;
    
    // Check if king is in check after the move
    int inCheck = isKingInCheck(board, color);
    
    // Undo the move
    board->grid[src[0]][src[1]] = srcPiece;
    board->grid[dest[0]][dest[1]] = destPiece;
    
    return inCheck;
}

// Check if it's a castling move (returns 1 for kingside, 2 for queenside, 0 otherwise)
int isCastlingMove(Board *board, Piece *piece, int src[2], int dest[2])
{
    if (getClass(piece) != KING) return 0;
    
    int colDiff = dest[1] - src[1];
    int rowDiff = abs(dest[0] - src[0]);
    
    if (rowDiff != 0) return 0;
    if (abs(colDiff) != 2) return 0;
    
    if (colDiff == 2) return 1;  // Kingside
    if (colDiff == -2) return 2; // Queenside
    
    return 0;
}

// Check if it's an en passant move
int isEnPassantMove(Board *board, Piece *piece, int src[2], int dest[2])
{
    if (getClass(piece) != PAWN) return 0;
    
    int color = getColor(piece);
    int colDiff = abs(dest[1] - src[1]);
    int direction = (color == 1) ? 1 : -1;
    int rowDiff = dest[0] - src[0];
    
    // Must be diagonal move to empty square
    if (colDiff != 1 || rowDiff != direction) return 0;
    if (board->grid[dest[0]][dest[1]] != EMPTY_CELL) return 0;
    
    // Check if it matches en passant target
    if (enPassantCol != -1 && dest[1] == enPassantCol)
    {
        int enPassantTargetRow = (color == 0) ? 2 : 5;
        if (dest[0] == enPassantTargetRow && enPassantRow == src[0])
        {
            return 1;
        }
    }
    
    return 0;
}

// Execute castling move (moves both king and rook)
void executeCastling(Board *board, int color, int side)
{
    int row = (color == 0) ? 7 : 0;
    
    if (side == 1) // Kingside
    {
        // Move king from e to g
        board->grid[row][6] = board->grid[row][4];
        board->grid[row][4] = EMPTY_CELL;
        // Move rook from h to f
        board->grid[row][5] = board->grid[row][7];
        board->grid[row][7] = EMPTY_CELL;
    }
    else if (side == 2) // Queenside
    {
        // Move king from e to c
        board->grid[row][2] = board->grid[row][4];
        board->grid[row][4] = EMPTY_CELL;
        // Move rook from a to d
        board->grid[row][3] = board->grid[row][0];
        board->grid[row][0] = EMPTY_CELL;
    }
    
    // Update castling rights
    if (color == 0)
    {
        wKingMoved = 1;
        wCastle = 0;
    }
    else
    {
        bKingMoved = 1;
        bCastle = 0;
    }
}

// Execute en passant capture (removes the captured pawn)
void executeEnPassant(Board *board, int src[2], int dest[2])
{
    // Move the capturing pawn
    board->grid[dest[0]][dest[1]] = board->grid[src[0]][src[1]];
    board->grid[src[0]][src[1]] = EMPTY_CELL;
    
    // Remove the captured pawn (it's on the same row as src, same col as dest)
    board->grid[src[0]][dest[1]] = EMPTY_CELL;
}

// Check if the given color is in checkmate
int isCheckmate(Board *board, int color)
{
    // First, must be in check
    if (!isKingInCheck(board, color))
        return 0;
    
    // Try all possible moves for all pieces of this color
    for (int srcRow = 0; srcRow < 8; srcRow++)
    {
        for (int srcCol = 0; srcCol < 8; srcCol++)
        {
            Piece *piece = &board->grid[srcRow][srcCol];
            if (*piece == EMPTY_CELL || getColor(piece) != color)
                continue;
            
            int src[2] = {srcRow, srcCol};
            
            // Try all destination squares
            for (int destRow = 0; destRow < 8; destRow++)
            {
                for (int destCol = 0; destCol < 8; destCol++)
                {
                    int dest[2] = {destRow, destCol};
                    
                    // Check if move is valid
                    if (isValidMove(board, piece, src, dest))
                    {
                        // Check if this move gets us out of check
                        if (!wouldBeInCheck(board, piece, src, dest))
                        {
                            return 0; // Found a legal move, not checkmate
                        }
                    }
                }
            }
        }
    }
    
    return 1; // No legal moves found while in check = checkmate
}

// Check if the given color is in stalemate
int isStalemate(Board *board, int color)
{
    // Must NOT be in check for stalemate
    if (isKingInCheck(board, color))
        return 0;
    
    // Try all possible moves for all pieces of this color
    for (int srcRow = 0; srcRow < 8; srcRow++)
    {
        for (int srcCol = 0; srcCol < 8; srcCol++)
        {
            Piece *piece = &board->grid[srcRow][srcCol];
            if (*piece == EMPTY_CELL || getColor(piece) != color)
                continue;
            
            int src[2] = {srcRow, srcCol};
            
            // Try all destination squares
            for (int destRow = 0; destRow < 8; destRow++)
            {
                for (int destCol = 0; destCol < 8; destCol++)
                {
                    int dest[2] = {destRow, destCol};
                    
                    // Check if move is valid
                    if (isValidMove(board, piece, src, dest))
                    {
                        // Check if this move doesn't put us in check
                        if (!wouldBeInCheck(board, piece, src, dest))
                        {
                            return 0; // Found a legal move, not stalemate
                        }
                    }
                }
            }
        }
    }
    
    return 1; // No legal moves found and not in check = stalemate
}