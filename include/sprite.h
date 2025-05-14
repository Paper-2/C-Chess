#ifndef SPRITE_H
#define SPRITE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>

typedef struct Sprite
{
    union
    {
        int width, w;
    };
    union
    {
        int height, h;
    };
    union
    {
        uint32_t *pixels, *p;
    };
} Sprite;


Sprite *loadSprite(char const path[]);



void RGBAToARGB(Sprite *sprite);

void freeSprite(Sprite *sprite);

#endif // !SPRITE_H