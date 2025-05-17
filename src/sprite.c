#include <stdint.h>
#include <zlib.h>
#include <string.h>
#include "sprite.h"
#include <stdio.h>
#include <stdlib.h>


#define IHDR 0x49484452 // present in every PNG file after the signature
#define IEND 0x49454E44 // Always the last chunk
#define PLTE 0x504C5445 
#define IDAT 0x49444154 // image data. it contains the image data.
#define tRNS 0x74524E53
#define cHRM 0x6348524D
#define gAMA 0x67414D41
#define iCCP 0x69434350
#define sBIT 0x73424954
#define sRGB 0x73524742
#define tEXt 0x74455874
#define zTXt 0x7A545874
#define iTXt 0x69545874
#define bKGD 0x624B4744
#define hIST 0x68495354
#define pHYs 0x70485973
#define sPLT 0x73504C54
#define tIME 0x74494D45


#include <stdlib.h>
#include <stdio.h>

#ifndef free
void free(void* ptr) {
    (void) ptr;
    printf("free isn't defined \n");
}
#endif
#ifndef fclose
int fclose(FILE *_File) {
    (void) _File;
    printf("fclose isn't defined \n");
    return 0;
}
#endif


/*
The first 8 bytes of a PNG file always start with:
dec     137     80    78    71    13    10    26     10
ascii   \211    P     N     G     \r    \n    \32    \n

The second 8 bytes contain the IHDR header

The rest are chunks

A chunk consist of
    Length  --  4 bytes for that determines the length in bytes of the data part. I'll named it DATA_LENGTH (It can be zero)

    Type    --  Next 4 bytes are letters they determine what the chunk is
            *encoder should treat the codes as fixed binary values, not character strings*
                first 5th    is ancillary    (not needed)
                second 5th   is private      (not needed)
                third 5th    is reserved     (not needed)
                fourth 5th   is Safe to copy (not needed)

    DATA    --  Next DATA_LENGTH bytes are data bytes appropriate to the chunk type, if any.
                This field can be of zero length

    CRC     --  4 bytes "Cyclic Redundancy Check" (whatever that means)
            calculated on the preceding bytes in the chunk, including the chunk type code and chunk data fields,
            but not including the length field. The CRC is always present, even for chunks containing no data.

                Can be ignored if file integrity is not important
                Ensures data integrity by detecting errors

The IHDR chunk must appear FIRST. It contains:

Width:              4 bytes
Height:             4 bytes
Bit depth:          1 byte
Color type:         1 byte
Compression method: 1 byte
Filter method:      1 byte
Interlace method:   1 byte


The data bytes in the IDAT chunk contains the following:

    Compression method/flags code: 1 byte
    Additional flags/check bits:   1 byte
    Compressed data blocks:        n bytes
    Check value:                   4 bytes



*/


int readChunk(FILE *filePtr, Sprite *spritePrt);

void processScanlines(uint8_t *uncompressedData, uint32_t width, uint32_t height, uint8_t bytesPerPixel, uint32_t *pixels) {
    uint32_t stride = width * bytesPerPixel; // Number of bytes per scanline (excluding filter byte)
    uint8_t *prevScanline = NULL;           // Pointer to the previous scanline (for filters)
    uint8_t *currentScanline = uncompressedData;

    for (uint32_t y = 0; y < height; y++) {
        uint8_t filterType = currentScanline[0]; // First byte is the filter type
        uint8_t *scanlineData = currentScanline + 1; // Remaining bytes are the pixel data

        // Apply the filter
        switch (filterType) {
            case 0: // None
                // No filtering; copy the scanline data directly
                memcpy(&pixels[y * width], scanlineData, stride);
                break;

            case 1: // Sub
                for (uint32_t x = 0; x < stride; x++) {
                    uint8_t left = (x >= bytesPerPixel) ? scanlineData[x - bytesPerPixel] : 0;
                    scanlineData[x] = (scanlineData[x] + left) & 0xFF;
                }
                memcpy(&pixels[y * width], scanlineData, stride);
                break;

            case 2: // Up
                for (uint32_t x = 0; x < stride; x++) {
                    uint8_t above = (prevScanline) ? prevScanline[x] : 0;
                    scanlineData[x] = (scanlineData[x] + above) & 0xFF;
                }
                memcpy(&pixels[y * width], scanlineData, stride);
                break;

            case 3: // Average
                for (uint32_t x = 0; x < stride; x++) {
                    uint8_t left = (x >= bytesPerPixel) ? scanlineData[x - bytesPerPixel] : 0;
                    uint8_t above = (prevScanline) ? prevScanline[x] : 0;
                    scanlineData[x] = (scanlineData[x] + ((left + above) / 2)) & 0xFF;
                }
                memcpy(&pixels[y * width], scanlineData, stride);
                break;

            case 4: // Paeth
                for (uint32_t x = 0; x < stride; x++) {
                    uint8_t left = (x >= bytesPerPixel) ? scanlineData[x - bytesPerPixel] : 0;
                    uint8_t above = (prevScanline) ? prevScanline[x] : 0;
                    uint8_t topLeft = (prevScanline && x >= bytesPerPixel) ? prevScanline[x - bytesPerPixel] : 0;

                    int p = left + above - topLeft;
                    int pa = abs(p - left);
                    int pb = abs(p - above);
                    int pc = abs(p - topLeft);

                    uint8_t predictor = (pa <= pb && pa <= pc) ? left : (pb <= pc) ? above : topLeft;
                    scanlineData[x] = (scanlineData[x] + predictor) & 0xFF;
                }
                memcpy(&pixels[y * width], scanlineData, stride);
                break;

            default:
                fprintf(stderr, "Unknown filter type: %d\n", filterType);
                exit(1);
        }

        // Move to the next scanline
        prevScanline = currentScanline + 1;
        currentScanline += stride + 1; // +1 for the filter byte
    }
}

/// @brief Loads a sprite from a PNG file and returns a pointer to the Sprite structure.
/// @param path The file path to the PNG image.
/// @return A pointer to the loaded Sprite structure, or NULL if loading fails.
Sprite *loadSprite(char const path[])
{
    FILE *filePtr;
    unsigned char signature[8];
    Sprite *spritePtr = (Sprite *)malloc(sizeof(Sprite));
    if (!spritePtr)
    {
        printf("Failed to allocate memory for Sprite\n");
        return NULL;
    }
    if (fopen) printf("fopen is indeed defined \n");

    filePtr = fopen(path, "rb");
    if (filePtr == NULL) {
        perror(path);
        perror("FILE DOES NOT EXIST \n ");
        
    }

    fread(signature, sizeof(signature), 1, filePtr);

    // Signature check
    if (signature[1] != 'P' || signature[2] != 'N' || signature[3] != 'G')
    {
        printf("only PNGs have been implemented\n");
        for (int i = 0; i < 8; i++) {
            printf("%02X ", signature[i]);
        }
        printf("\n");
        return 0;
    }
    int decodingFile = 1;

    while (decodingFile)
    {
        decodingFile = readChunk(filePtr, spritePtr);
    }

    fclose(filePtr);
    return spritePtr;
}


/// @brief Converts the pixel format of a sprite from RGBA to ARGB.
/// @param sprite Pointer to the sprite to be converted.
void RGBAToARGB(Sprite *sprite)
{

    uint32_t totalPixels = sprite->w * sprite->h;

    for (uint32_t i = 0; i < totalPixels; i++)
    {

        uint32_t pixel = sprite->pixels[i];

        uint8_t r = (pixel >> 24) & 0xFF;
        uint8_t g = (pixel >> 16) & 0xFF;
        uint8_t b = (pixel >> 8) & 0xFF;
        uint8_t a = pixel & 0xFF;

        // If the pixel is black (R, G, B all zero), make it fully transparent
        sprite->pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
}

/// @brief frees the sprite
/// @param sprite Pointer to the sprite that will be dealloc
void freeSprite(Sprite *sprite)
{
    free(sprite->pixels);
    free(sprite);
}
int readChunk(FILE *filePtr, Sprite *spritePtr)
{
    unsigned char buffer[4];
    unsigned char *data;
    unsigned char chunkType[4];
    int dataLength;
    int typeValue;

    /*DATA COLLECTION*/

    fread(buffer, sizeof(buffer), 1, filePtr);
    dataLength = __builtin_bswap32(*(int32_t *)buffer);
    data = (unsigned char *)malloc(sizeof(unsigned char) * dataLength);

    fread(chunkType, sizeof(chunkType), 1, filePtr);
    
    if (chunkType[0] & 0b100000) // if the 5th bit of the type is 1 we can safely skip the chunk. See ancillary chunks
    {
        fseek(filePtr, dataLength + 4, SEEK_CUR);
        return 1;
    }
    fread(data, dataLength, 1, filePtr);
    fseek(filePtr, 4, SEEK_CUR); // skips the CRC.

    // Type casting the 4-byte char array to a 4-byte integer
    typeValue = __builtin_bswap32(*(int32_t *)chunkType); // bswap32 is will switch endians

    // Check for specific chunk types
    switch (typeValue)
    {
    case IHDR:
        spritePtr->width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        spritePtr->height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
        spritePtr->pixels = (uint32_t *)malloc(sizeof(uint16_t) * (spritePtr->h * spritePtr->w)); // allocates the space for the image
        break;

    case IDAT:

        /*
        1. Begin with image scanlines represented as described in Image layout; the layout and total size of this
        raw data are determined by the fields of IHDR. Done ✔️



        Filter the image data according to the filtering method specified by the IHDR chunk.
        (Note that with filter method 0, the only one currently defined, this implies prepending a filter-type byte to each scanline.)*/

        /*
        Compress the filtered data using the compression method specified by the IHDR chunk.

        To read the image data, reverse this process. (wtf)
        */
        uLongf decompressedSize = spritePtr->h * spritePtr->w * sizeof(uint32_t) * 2; // Allocate twice the expected size for safety
        unsigned char *decompressedData = (unsigned char *)malloc(decompressedSize);
        if (!decompressedData)
        {
            printf("Failed to allocate memory for decompressed data\n");
            free(data);
            return 0;
        }

        // decompress data
        // TODO: make my own uncompress function
        int status = uncompress(decompressedData, &decompressedSize, data, dataLength);

        if (status != Z_OK)
        {
            printf("Failed to decompress IDAT chunk\n");
            if (decompressedData) {
                free(decompressedData);
                decompressedData = NULL;
            }
            free(data);
            return 0;
        }
        // image reconstruction
        processScanlines(decompressedData, spritePtr->w, spritePtr->h, sizeof(uint32_t), spritePtr->pixels);


        free(decompressedData); 

        break;

    case IEND: // "IEND"
    
        free(data);
        return 0; // End of PNG file
    }

    free(data);

    return 1;
}


/*
int main(int argc, char const *argv[])
{
    example usage of loadSprite. pixels are in RGBA (uint32) format
    Sprite *sprite = loadSprite("./assets/white/w_king.png");

    free(sprite->pixels);
    free(sprite);
    return 0;
}

*/