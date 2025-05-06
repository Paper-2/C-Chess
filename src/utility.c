
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

char *pieceToString(Piece piece)
{
    char *string = malloc(3 * sizeof(char));
    string[2] = '\0';

    /*
    White		0
    Black		1
    */
    if (piece == 0b11111111)
    {
        string[0] = '-'; // Empty cell
        string[1] = '-'; // Empty cell
        return string;
    }

    switch (piece & 0b1)
    {
    case 0:
        string[0] = 'w';
        break;
    case 1:
        string[0] = 'b';
        break;
    }
    /*
    king		0
    queen		1
    rook		2
    bishop		3
    knight		4
    pawn		5
    */

    switch (piece >> 1)
    {
    case 0:
        string[1] = 'K';
        break;
    case 1:
        string[1] = 'Q';
        break;
    case 2:
        string[1] = 'R';
        break;
    case 3:
        string[1] = 'B';
        break;
    case 4:
        string[1] = 'N';
        break;
    case 5:
        string[1] = 'P';
        break;
    }
    return string;
}

// Piece fenToPieceArr(char string[])
// {
//     for (int i = 0; i < 64; i++)
//     {
//         if (string[i] = ' ')
//         {
//             string[i] = '\0';
//             break;
//         }
//     }
// }
