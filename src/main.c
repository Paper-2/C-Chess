#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "board.h"
#include "sprite.h"

#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);

#if RAND_MAX == 32767
#define rand32() ((rand() << 15) + (rand() << 1) + (rand() & 1))
#else
#define rand32() rand()
#endif

#define MIN_SIZE_FACTOR 40
#define SIZE_SCALE 1.6
#define WINHeight 9 * (SIZE_SCALE * MIN_SIZE_FACTOR)
#define WINWidth 16 * (SIZE_SCALE * MIN_SIZE_FACTOR)

bool quit = false;
HWND window_handle;
BITMAPINFO bitmap_info;
HBITMAP bitmap;
HDC bitmap_device_context;

struct
{
	union
	{
		int w, width;
	};
	union
	{
		int h, height;
	};

	uint32_t *pixels;
} frame = {0};

bool keyboard[256] = {0};
struct
{
	int x, y;
	uint8_t buttons;
} mouse;
enum
{
	MOUSE_LEFT = 0b1,
	MOUSE_MIDDLE = 0b10,
	MOUSE_RIGHT = 0b100,
	MOUSE_X1 = 0b1000,
	MOUSE_X2 = 0b10000
};

// arrays to hold the sprites
Sprite* blackPieces[6] = {0};
Sprite* whitePieces[6] = {0};



LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);



// This function is responsible of loading the sprites the global variables [color]Pieces
// The directory should be set to C-Chess/src when running the program or else the path to the sprites will be invalid
void loadSprites()
{

	blackPieces[0] = loadSprite("./assets/black/b_king.png");
	blackPieces[1] = loadSprite("./assets/black/b_queen.png");
	blackPieces[2] = loadSprite("./assets/black/b_rook.png");
	blackPieces[3] = loadSprite("./assets/black/b_bishop.png");
	blackPieces[4] = loadSprite("./assets/black/b_knight.png");
	blackPieces[5] = loadSprite("./assets/black/b_pawn.png");

	whitePieces[0] = loadSprite("./assets/white/w_king.png");
	whitePieces[1] = loadSprite("./assets/white/w_queen.png");
	whitePieces[2] = loadSprite("./assets/white/w_rook.png");
	whitePieces[3] = loadSprite("./assets/white/w_bishop.png");
	whitePieces[4] = loadSprite("./assets/white/w_knight.png");
	whitePieces[5] = loadSprite("./assets/white/w_pawn.png");

/*	FIXME: the sprites use the RGBA pixel format yet the window uses ARGB this should not break the sprite, yet it does... 
		Not applying RGBAToARGB is fine, the pixels would just use the alpha value of the previous as rgbARGBa
		
		for (int i = 0; i < 6; i++) {
			if (blackPieces[i]) {
				RGBAToARGB(blackPieces[i]);
			}
			if (whitePieces[i]) {
				RGBAToARGB(whitePieces[i]);
			}
		}
*/
	};

void drawSquare(uint32_t *pixels, int x, int y, int width, int height, int color)
{

	// inverting (x, y) to match the window coordinates
	// x =  frame.width - x - width;
	// y =  frame.height - y - height;
	int pixelToAccess;
	// int pixelLimit = frame.width * frame.height;
	if (x < 0 || y < 0 || x + width > frame.width || y + height > frame.height)
	{
		PRINT_ERROR("drawSquare() failed. Out of bounds.\n");
		return;
	}
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			pixelToAccess = (x + i) + (y + j) * frame.width;
			pixels[pixelToAccess] = color;
		}
	}
}

void drawPiece(uint32_t *pixels, int x, int y, Sprite *piece) {
	for (int i = 0; i < piece->height; i++) {
		int destY = y + (piece->height - 1 - i); // Flip vertically
		if (destY < 0 || destY >= frame.height)
			continue;

		for (int j = 0; j < piece->width; j++) {
			int destX = x + j;
			if (destX < 0 || destX >= frame.width)
				continue;

			int srcIndex = i * piece->width + j;
			int destIndex = destY * frame.width + destX;

			uint32_t srcPixel = piece->pixels[srcIndex];
			uint32_t destPixel = pixels[destIndex];

			uint8_t srcAlpha = (srcPixel >> 24) & 0xFF;
			uint8_t srcRed = (srcPixel >> 16) & 0xFF;
			uint8_t srcGreen = (srcPixel >> 8) & 0xFF;
			uint8_t srcBlue = srcPixel & 0xFF;

			uint8_t destRed = (destPixel >> 16) & 0xFF;
			uint8_t destGreen = (destPixel >> 8) & 0xFF;
			uint8_t destBlue = destPixel & 0xFF;

			// Normalize alpha to [0, 1]
			float alpha = srcAlpha / 255.0f;

			// Blend each color channel
			uint8_t blendedRed = (uint8_t)((srcRed * alpha) + (destRed * (1 - alpha)));
			uint8_t blendedGreen = (uint8_t)((srcGreen * alpha) + (destGreen * (1 - alpha)));
			uint8_t blendedBlue = (uint8_t)((srcBlue * alpha) + (destBlue * (1 - alpha)));

			// Write the blended pixel back
			pixels[destIndex] = (0xFF << 24) | (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
		}
	}
}

/*
void drawPiece(uint32_t *pixels, int x, int y, Sprite *piece)
{
	for (int i = piece->height - 1; i >= 0; i--)
	{
		int destY = y + (piece->height - 1 - i);
		if (destY < 0 || destY >= frame.height)
			continue;

		for (int j = 0; j < piece->width; j++)
		{
			int destX = x + j;
			if (destX < 0 || destX >= frame.width)
				continue;

			int srcIndex = i * piece->width + j;
			int destIndex = destY * frame.width + destX;

			// Assuming the sprite has an alpha channel and 0xFF000000 is fully transparent
			if ((piece->pixels[srcIndex] & 0xFF000000) != 0)
			{
				memcpy(&pixels[destIndex], &piece->pixels[srcIndex], sizeof(uint32_t));
			}
		}
	}
}



void drawBoard(Board *board)
{

	/*

	Draws the current state of the board
	Its a private function
	
	*/

	// The first byte is the alpha channel
	int light = 0xFF7c4c3e;	// #7c4c3e
	int dark =  0xFF512a2a;	// #512a2a

	Sprite *pieceToBeDrawn = whitePieces[2];
	int squareSize = 60; 
	int xOffSet = 20; 
	int yOffSet = 30;
	Piece curPiece;



	Sprite** pieceArr; // this pointer points to an arr of sprites. currently not set.

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int xCord = (i * squareSize) + xOffSet;
			int yCord = (j * squareSize) + yOffSet;
			if ((i + j) % 2 != 0)
			{
				drawSquare(frame.pixels, xCord, yCord, squareSize, squareSize, light);
			}
			else
			{
				drawSquare(frame.pixels, xCord, yCord, squareSize, squareSize, dark);
			}

			curPiece = board->grid[7-j][7-i];
			switch (getColor(&curPiece))
			{
			case 0:
				pieceArr = whitePieces; // sets the pieceArr to the white pieces
				break;
			
			case 1:
				pieceArr = blackPieces; // sets the pieceArr to the black pieces

				break;
			}
			

			if (curPiece != EMPTY_CELL){ // if the square isn't empty don't attempt to draw it. 
				pieceToBeDrawn = pieceArr[getClass(&curPiece)];			
				drawPiece(frame.pixels, xCord + 5, yCord + 5, pieceToBeDrawn);	
			}
			//drawSprite(frame.pixels, xCord, yCord, board);
		}
	}
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{

	// boiler plate
	const wchar_t window_class_name[] = L"Window Class";
	static WNDCLASS window_class = {0};
	window_class.lpfnWndProc = WindowProcessMessage;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = window_class_name;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&window_class);

	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_device_context = CreateCompatibleDC(0);

	

	window_handle = CreateWindow(window_class_name, L"C-Chess", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
								 CW_USEDEFAULT, CW_USEDEFAULT, WINWidth, WINHeight, NULL, NULL, hInstance, NULL);
	if (window_handle == NULL)
	{
		PRINT_ERROR("CreateWindow() failed. Returned NULL.\n");
		return -1;
	}

	// end of boiler plate


	// Loading data game stuff
	loadSprites(); // loads the sprites to whitePieces and BlackPieces
	Board board;
	board.grid = malloc(8 * sizeof(Piece *)); // allocates memory for the grid



	for (int i = 0; i < 8; i++) 
	{
		board.grid[i] = malloc(8 * sizeof(Piece)); // allocates memory for a row of the grid
		memcpy(board.grid[i], initialBoardData[i], 8 * sizeof(Piece)); // copies a row from the defaultBoard to board
	}
	

	// Window loop
	while (!quit)
	{
		static MSG message = {0};

		// Events
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&message);
		}

		drawSquare(frame.pixels, 0, 0, frame.width, frame.height, 0xFFFFFFFF); // draws the white background, should run once i guess...
		drawBoard(&board); // updates the board

		
		InvalidateRect(window_handle, NULL, FALSE);
		UpdateWindow(window_handle); // this is what actually draws the pixels
	}

	// Free allocated memory for board grid
	for (int i = 0; i < 8; i++)
	{
		free(board.grid[i]);
	}
	free(board.grid);

	return 0;
}

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool has_focus = true;

	/*This is where the events are handled, I suppose this where the logic of some of the game will be handle*/
	switch (message)
	{
	case WM_QUIT:
	case WM_DESTROY:
	{
		quit = true;
	}
	break;

	case WM_PAINT:
	{
		static PAINTSTRUCT paint;
		static HDC device_context;
		device_context = BeginPaint(window_handle, &paint);
		BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
		EndPaint(window_handle, &paint);
	}
	break;

	case WM_SIZE:
	{
		frame.w = bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
		frame.h = bitmap_info.bmiHeader.biHeight = HIWORD(lParam);
		if (bitmap)
			DeleteObject(bitmap);
		bitmap = CreateDIBSection(NULL, &bitmap_info, DIB_RGB_COLORS, (void **)&frame.pixels, 0, 0);
		SelectObject(bitmap_device_context, bitmap);
	}
	break;

	case WM_KILLFOCUS:
	{
		has_focus = false;
		memset(keyboard, 0, 256 * sizeof(keyboard[0]));
		mouse.buttons = 0;
	}
	break;

	case WM_SETFOCUS:
		has_focus = true;
		break;

	case WM_MOUSEMOVE:
	{
		mouse.x = LOWORD(lParam);
		mouse.y = frame.h - 1 - HIWORD(lParam);
	}
	break;

	case WM_LBUTTONDOWN:
		mouse.buttons |= MOUSE_LEFT;
		break;
	case WM_LBUTTONUP:
		mouse.buttons &= ~MOUSE_LEFT;
		break;
	case WM_MBUTTONDOWN:
		mouse.buttons |= MOUSE_MIDDLE;
		break;
	case WM_MBUTTONUP:
		mouse.buttons &= ~MOUSE_MIDDLE;
		break;
	case WM_RBUTTONDOWN:
		mouse.buttons |= MOUSE_RIGHT;
		break;
	case WM_RBUTTONUP:
		mouse.buttons &= ~MOUSE_RIGHT;
		break;

	case WM_XBUTTONDOWN:
	{
		if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
		{
			mouse.buttons |= MOUSE_X1;
		}
		else
		{
			mouse.buttons |= MOUSE_X2;
		}
	}
	break;
	case WM_XBUTTONUP:
	{
		if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
		{
			mouse.buttons &= ~MOUSE_X1;
		}
		else
		{
			mouse.buttons &= ~MOUSE_X2;
		}
	}
	break;


	break;

	default:
		return DefWindowProc(window_handle, message, wParam, lParam);
	}
	return 0;
}