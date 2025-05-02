#include <stdlib.h> 
#include <stdio.h>

#include "utility.h"
#include "board.h"
#include "piece.h"

Piece initialBoardData[8][8] = {
        {    0b0101,     0b1001,     0b0111,     0b0011,     0b0001,     0b0111,     0b1001,     0b0101},
        {    0b1011,     0b1011,     0b1011,     0b1011,     0b1011,     0b1011,     0b1011,     0b1011},
        {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
        {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
        {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
        {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
        {    0b1010,     0b1010,     0b1010,     0b1010,     0b1010,     0b1010,     0b1010,     0b1010},
        {    0b0100,     0b1000,     0b0110,     0b0010,     0b0000,     0b0110,     0b1000,     0b1010}
};



Board *makeEmptyBoard()
{
    Board *board = malloc(sizeof(Board));
    board->grid = malloc(8 * sizeof(char *));

    for (int row = 0; row < 8; row++)
    {
        board->grid[row] = malloc(8 * sizeof(char)); // initialize each row of the grid

        for (int col = 0; col < 8; col++)
        {
            board->grid[row][col] = 0b11111111; // set each cell to 0b11111111, which doesn't represent any piece.
        }
    }

    return board;
}

// Board *defaultBoard()
// { #TODO
//     return nullptr;
// }

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
            pieceAtCell = board->grid[row][col]; // get the piece at the current cell
            pieceString = pieceToString(pieceAtCell); // convert the piece to a string
            printf("%s ", pieceString); // print each cell of the grid
            free(pieceString);
        }
        printf("\n"); // new line after each row
    }
}

void movePiece(Board *board, Piece *piece, int dest[2])
{
    // moves a piece from one position to another
    board->grid[dest[0]][dest[1]] = *piece; // set the destination cell to the piece
    *piece = 0b11111111;                    // set the source cell to an empty cell
    piece = &board->grid[dest[0]][dest[1]]; // set the piece to the destination cell
    
}

void setBoard(Board * board, Piece boardData[8][8])
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            board->grid[row][col] = boardData[row][col];
        }
    }
}

int isvalidMove(Board *, Piece *, int[2], int[2])
{
    return 0;
}

int isSpaceFree(Board *board, int pos[2])
{
    return board->grid[pos[0]][pos[1]] == 0b11111111; // check if the space is empty
}


int main()
{
    Board *board = makeEmptyBoard();
    setBoard(board, initialBoardData); // set the board to the initial state
    printBoard(board);
    
    freeBoard(board);
    return 0;
}