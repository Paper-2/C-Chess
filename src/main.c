#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "board.h"
#include "sprite.h"
#include "utility.h"
#include "paths.h"

#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);

#if RAND_MAX == 32767
#define rand32() ((rand() << 15) + (rand() << 1) + (rand() & 1))
#else
#define rand32() rand()
#endif

#define MIN_SIZE_FACTOR 40
#define SIZE_SCALE 1.6
#define WINHeight 10 * (SIZE_SCALE * MIN_SIZE_FACTOR) + 50  // Extra space for UI
#define WINWidth 16 * (SIZE_SCALE * MIN_SIZE_FACTOR) + 100  // Extra space for captured pieces panel

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
Sprite *blackPieces[6] = {0};
Sprite *whitePieces[6] = {0};
Board *gameBoard;

// Game state
int currentTurn = 0; // 0 = White, 1 = Black
int capturedWhite[6] = {0}; // Count of captured white pieces [King, Queen, Rook, Bishop, Knight, Pawn]
int capturedBlack[6] = {0}; // Count of captured black pieces
int gameOver = 0; // 0 = playing, 1 = white wins, 2 = black wins, 3 = stalemate
int inCheck = 0; // 0 = no check, 1 = white in check, 2 = black in check

struct
{
	Piece *selectedPiece;
	int x, y;
	int possibleMoves[64][2];
	int moveQuantity;
	bool isCurrent; // whether or not the list of possible moves for the selected piece needs to be updated
} SelectedPieceInfo = {NULL, 0, 0, {{0, 0}}};

int xOffSet = 20;
int yOffSet = 80; // Increased to make room for UI at top
int squareSize = 60;

// UI Layout constants
int uiTopHeight = 70;
int uiRightWidth = 200;
HFONT hFontLarge = NULL;
HFONT hFontSmall = NULL;

LRESULT CALLBACK WindowProcessMessage(HWND local_window_handle, UINT message, WPARAM wParam, LPARAM lParam);

void generateMoves();
void drawUI(HDC hdc);  // Forward declaration

// Load sprites using paths relative to executable location
void loadSprites()
{
	char path[MAX_PATH];
	
	// Initialize path resolution based on executable location
	if (paths_init() != 0) {
		PRINT_ERROR("Failed to initialize paths\n");
		return;
	}

	// Load black pieces
	blackPieces[0] = loadSprite(paths_get_asset("black/b_king.png", path, sizeof(path)));
	blackPieces[1] = loadSprite(paths_get_asset("black/b_queen.png", path, sizeof(path)));
	blackPieces[2] = loadSprite(paths_get_asset("black/b_rook.png", path, sizeof(path)));
	blackPieces[3] = loadSprite(paths_get_asset("black/b_bishop.png", path, sizeof(path)));
	blackPieces[4] = loadSprite(paths_get_asset("black/b_knight.png", path, sizeof(path)));
	blackPieces[5] = loadSprite(paths_get_asset("black/b_pawn.png", path, sizeof(path)));

	// Load white pieces
	whitePieces[0] = loadSprite(paths_get_asset("white/w_king.png", path, sizeof(path)));
	whitePieces[1] = loadSprite(paths_get_asset("white/w_queen.png", path, sizeof(path)));
	whitePieces[2] = loadSprite(paths_get_asset("white/w_rook.png", path, sizeof(path)));
	whitePieces[3] = loadSprite(paths_get_asset("white/w_bishop.png", path, sizeof(path)));
	whitePieces[4] = loadSprite(paths_get_asset("white/w_knight.png", path, sizeof(path)));
	whitePieces[5] = loadSprite(paths_get_asset("white/w_pawn.png", path, sizeof(path)));

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

// Initialize fonts for UI
void initFonts()
{
	if (!hFontLarge) {
		hFontLarge = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
	}
	if (!hFontSmall) {
		hFontSmall = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
	}
}

// Draw UI elements using GDI (called during WM_PAINT)
void drawUI(HDC hdc)
{
	initFonts();
	
	// Set up drawing
	SetBkMode(hdc, TRANSPARENT);
	
	// Draw turn indicator background
	RECT turnRect = {10, 10, 250, 60};
	HBRUSH turnBrush;
	if (gameOver) {
		turnBrush = CreateSolidBrush(RGB(100, 100, 100)); // Gray for game over
	} else if (currentTurn == 0) {
		turnBrush = CreateSolidBrush(RGB(240, 240, 240)); // Light for white's turn
	} else {
		turnBrush = CreateSolidBrush(RGB(60, 60, 60)); // Dark for black's turn
	}
	FillRect(hdc, &turnRect, turnBrush);
	DeleteObject(turnBrush);
	
	// Draw border
	HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(80, 40, 40));
	HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, turnRect.left, turnRect.top, turnRect.right, turnRect.bottom);
	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(borderPen);
	
	// Draw turn text
	SelectObject(hdc, hFontLarge);
	wchar_t turnText[64];
	if (gameOver == 1) {
		wcscpy(turnText, L"White Wins!");
		SetTextColor(hdc, RGB(0, 128, 0));
	} else if (gameOver == 2) {
		wcscpy(turnText, L"Black Wins!");
		SetTextColor(hdc, RGB(0, 128, 0));
	} else if (gameOver == 3) {
		wcscpy(turnText, L"Stalemate!");
		SetTextColor(hdc, RGB(128, 128, 0));
	} else if (currentTurn == 0) {
		wcscpy(turnText, L"White's Turn");
		SetTextColor(hdc, RGB(40, 40, 40));
	} else {
		wcscpy(turnText, L"Black's Turn");
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	RECT textRect = {turnRect.left + 10, turnRect.top + 5, turnRect.right - 10, turnRect.bottom - 5};
	DrawText(hdc, turnText, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	
	// Draw check indicator
	if (inCheck && !gameOver) {
		SelectObject(hdc, hFontSmall);
		SetTextColor(hdc, RGB(200, 0, 0));
		RECT checkRect = {turnRect.left + 10, turnRect.bottom - 20, turnRect.right - 10, turnRect.bottom - 2};
		DrawText(hdc, L"CHECK!", -1, &checkRect, DT_CENTER | DT_SINGLELINE);
	}
	
	// Draw captured pieces section
	int capturedX = 8 * squareSize + xOffSet + 30;
	int capturedY = yOffSet;
	
	// White captured pieces header
	SelectObject(hdc, hFontSmall);
	SetTextColor(hdc, RGB(40, 40, 40));
	RECT whiteHeader = {capturedX, capturedY, capturedX + 150, capturedY + 20};
	DrawText(hdc, L"Captured White:", -1, &whiteHeader, DT_LEFT | DT_SINGLELINE);
	
	// Draw captured white pieces counts
	const wchar_t* pieceSymbols[] = {L"\x2654", L"\x2655", L"\x2656", L"\x2657", L"\x2658", L"\x2659"}; // Unicode chess symbols
	const wchar_t* pieceNames[] = {L"K", L"Q", L"R", L"B", L"N", L"P"};
	int py = capturedY + 25;
	for (int i = 0; i < 6; i++) {
		if (capturedWhite[i] > 0) {
			wchar_t countText[16];
			swprintf(countText, 16, L"%s x%d", pieceNames[i], capturedWhite[i]);
			RECT pieceRect = {capturedX, py, capturedX + 100, py + 18};
			DrawText(hdc, countText, -1, &pieceRect, DT_LEFT | DT_SINGLELINE);
			py += 18;
		}
	}
	
	// Black captured pieces header
	int blackCapturedY = capturedY + 150;
	RECT blackHeader = {capturedX, blackCapturedY, capturedX + 150, blackCapturedY + 20};
	DrawText(hdc, L"Captured Black:", -1, &blackHeader, DT_LEFT | DT_SINGLELINE);
	
	// Draw captured black pieces counts
	py = blackCapturedY + 25;
	for (int i = 0; i < 6; i++) {
		if (capturedBlack[i] > 0) {
			wchar_t countText[16];
			swprintf(countText, 16, L"%s x%d", pieceNames[i], capturedBlack[i]);
			RECT pieceRect = {capturedX, py, capturedX + 100, py + 18};
			DrawText(hdc, countText, -1, &pieceRect, DT_LEFT | DT_SINGLELINE);
			py += 18;
		}
	}
	
	// Draw instructions at bottom
	SelectObject(hdc, hFontSmall);
	SetTextColor(hdc, RGB(80, 80, 80));
	RECT instrRect = {10, frame.height - 25, 400, frame.height - 5};
	DrawText(hdc, L"Left-click: Select/Move | Right-click: Deselect", -1, &instrRect, DT_LEFT | DT_SINGLELINE);
}

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

void drawCircle(uint32_t *pixels, int x, int y, int r, int color)
{
	for (int i = -r; i <= r; i++)
	{
		for (int j = -r; j <= r; j++)
		{
			double distance = ((double)(i * i + j * j)) / ((double)(r * r));
			if (distance <= 1.0)
			{
				int pixelX = x + i;
				int pixelY = y + j;

				if (pixelX >= 0 && pixelX < frame.width && pixelY >= 0 && pixelY < frame.height)
				{
					int pixelIndex = pixelX + pixelY * frame.width;

					uint8_t alpha = (uint8_t)((1.0 - distance) * 255);
					alpha = (uint8_t)((1.0 - distance) * ((color >> 24) & 0xFF));
					uint8_t srcRed = (color >> 16) & 0xFF;
					uint8_t srcGreen = (color >> 8) & 0xFF;
					uint8_t srcBlue = color & 0xFF;

					uint32_t destPixel = pixels[pixelIndex];
					uint8_t destRed = (destPixel >> 16) & 0xFF;
					uint8_t destGreen = (destPixel >> 8) & 0xFF;
					uint8_t destBlue = destPixel & 0xFF;

					float alphaFactor = alpha / 255.0f;

					uint8_t blendedRed = (uint8_t)((srcRed * alphaFactor) + (destRed * (1 - alphaFactor)));
					uint8_t blendedGreen = (uint8_t)((srcGreen * alphaFactor) + (destGreen * (1 - alphaFactor)));
					uint8_t blendedBlue = (uint8_t)((srcBlue * alphaFactor) + (destBlue * (1 - alphaFactor)));
					pixels[pixelIndex] = (0xFF << 24) | (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
				}
			}
		}
	}
}

void drawPiece(uint32_t *pixels, int x, int y, Sprite *piece)
{
	if (!piece || !piece->pixels) return; // NULL check
	for (int i = 0; i < piece->height; i++)
	{
		int destY = y + (piece->height - 1 - i); // Flip vertically
		if (destY < 0 || destY >= frame.height)
			continue;

		for (int j = 0; j < piece->width; j++)
		{
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
*/

void drawBoard(Board *board)
{

	/*

	Draws the current state of the board
	Its a private function

	*/

	// The first byte is the alpha channel
	int light = 0xFF7c4c3e; // #7c4c3e
	int dark = 0xFF512a2a;	// #512a2a

	int hint = 0x40080808; // #080808

	Sprite *pieceToBeDrawn = whitePieces[2];

	Piece curPiece;

	Sprite **pieceArr; // this pointer points to an arr of sprites. currently not set.

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int xCord = (i * squareSize) + xOffSet;
			int yCord = (j * squareSize) + yOffSet;
			
			// Check if this is the selected square
			bool isSelected = (SelectedPieceInfo.selectedPiece != NULL && 
							   SelectedPieceInfo.x == i && 
							   SelectedPieceInfo.y == j);
			
			int squareColor;
			if (isSelected) {
				squareColor = 0xFF4a7c4a; // Green highlight for selected piece
			} else if ((i + j) % 2 != 0) {
				squareColor = light;
			} else {
				squareColor = dark;
			}
			drawSquare(frame.pixels, xCord, yCord, squareSize, squareSize, squareColor);

			curPiece = board->grid[7 - j][i];
			switch (getColor(&curPiece))
			{
			case 0:
				pieceArr = whitePieces; // sets the pieceArr to the white pieces
				break;

			case 1:
				pieceArr = blackPieces; // sets the pieceArr to the black pieces

				break;
			}

			if (curPiece != EMPTY_CELL)
			{ // if the square isn't empty don't attempt to draw it.
				pieceToBeDrawn = pieceArr[getClass(&curPiece)];
				if (pieceToBeDrawn) {
					drawPiece(frame.pixels, xCord + 5, yCord + 5, pieceToBeDrawn);
				}
			}
			// drawSprite(frame.pixels, xCord, yCord, board);

			if (SelectedPieceInfo.isCurrent)
			{
				int x, y;
				for (int moveIndex = 0; moveIndex < SelectedPieceInfo.moveQuantity; moveIndex++)
				{
					x = SelectedPieceInfo.possibleMoves[moveIndex][1];
					y = 7 - SelectedPieceInfo.possibleMoves[moveIndex][0];

					int hintX = ((x)*squareSize) + xOffSet + squareSize / 2;
					int hintY = ((y)*squareSize) + yOffSet + squareSize / 2;
					drawCircle(frame.pixels, hintX, hintY, squareSize / 6, hint);
				}
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	(void)hPrevInstance;
	(void)pCmdLine;
	(void)nCmdShow;

	// boiler plate
	const wchar_t window_class_name[] = L"Window Class";
	static WNDCLASS window_class = {0};
	window_class.style = CS_OWNDC;  // Keep device context for better performance
	window_class.lpfnWndProc = WindowProcessMessage;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = window_class_name;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = NULL;  // No background brush to prevent flicker
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
	
	// Initialize game state
	currentTurn = 0;  // White starts
	turn = 0;         // Sync with board.h extern variable
	gameOver = 0;
	inCheck = 0;
	
	// Initialize castling and en passant tracking
	enPassantCol = -1;
	enPassantRow = -1;
	wKingMoved = 0;
	bKingMoved = 0;
	wRookKMoved = 0;
	wRookQMoved = 0;
	bRookKMoved = 0;
	bRookQMoved = 0;
	wCastle = 3;
	bCastle = 3;
	
	for (int i = 0; i < 6; i++) {
		capturedWhite[i] = 0;
		capturedBlack[i] = 0;
	}

	Board *board; // allocate memory for the Board object
	board = (Board *) malloc(sizeof(Board));
	gameBoard = board;
	board->grid = malloc(8 * sizeof(Piece *)); // allocates memory for the grid

	for (int i = 0; i < 8; i++)
	{
		board->grid[i] = malloc(8 * sizeof(Piece));						// allocates memory for a row of the grid
		memcpy(board->grid[i], initialBoardData[i], 8 * sizeof(Piece)); // copies a row from the defaultBoard to board
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

		if (!frame.pixels) {
			continue; // Wait for WM_SIZE to initialize the frame
		}

		drawSquare(frame.pixels, 0, 0, frame.width, frame.height, 0xFFFFFFFF); // draws the white background, should run once i guess...
		drawBoard(board);													   // updates the board

		InvalidateRect(window_handle, NULL, FALSE);
		UpdateWindow(window_handle); // this is what actually draws the pixels
		
		// Small sleep to reduce CPU usage and flickering
		Sleep(16); // ~60 FPS cap
	}

	// Free allocated memory for board grid
	for (int i = 0; i < 8; i++)
	{
		free(board->grid[i]);
	}
	free(board->grid);

	return 0;
}


// global vars for the controller i think...
Piece *prevSelected;
Piece *nextPiece = NULL;

LRESULT CALLBACK WindowProcessMessage(HWND local_window_handle, UINT message, WPARAM wParam, LPARAM lParam)
{

	/*This is where the events are handled, I suppose this where the logic of some of the game will be handle*/
	switch (message)
	{
	case WM_QUIT:
	case WM_DESTROY:
	{
		quit = true;
	}
	break;

	case WM_ERASEBKGND:
		// Return 1 to indicate we handled background erasing (prevents flicker)
		return 1;

	case WM_PAINT:
	{
		static PAINTSTRUCT paint;
		static HDC device_context;
		device_context = BeginPaint(local_window_handle, &paint);
		BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
		// Draw UI elements on top
		drawUI(device_context);
		EndPaint(local_window_handle, &paint);
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
		memset(keyboard, 0, 256 * sizeof(keyboard[0]));
		mouse.buttons = 0;
	}
	break;

	case WM_SETFOCUS:
		break;

	case WM_MOUSEMOVE:
	{
		mouse.x = LOWORD(lParam);
		mouse.y = frame.h - 1 - HIWORD(lParam);
	}
	break;

	case WM_LBUTTONDOWN:
		mouse.buttons |= MOUSE_LEFT;

		// Ignore clicks if game is over
		if (gameOver) break;

		int xCord = (mouse.x - xOffSet) / squareSize;
		int yCord = (mouse.y - yOffSet) / squareSize;

		// Check if click is within board bounds
		if (xCord < 0 || xCord > 7 || yCord < 0 || yCord > 7) break;

		if (SelectedPieceInfo.selectedPiece)
			prevSelected = SelectedPieceInfo.selectedPiece; // only update prev when if the nextPiece is not null
		nextPiece = getPieceAt(gameBoard, xCord, yCord);

		// Only allow selecting pieces of the current player's color
		if (!SelectedPieceInfo.selectedPiece && *nextPiece != EMPTY_CELL)
		{
			// Check if the piece belongs to the current player
			int pieceColor = getColor(nextPiece);
			if (pieceColor == currentTurn) {
				SelectedPieceInfo.selectedPiece = nextPiece;
				SelectedPieceInfo.x = xCord;
				SelectedPieceInfo.y = yCord;
			}
		}
		else if (areAllies(nextPiece, SelectedPieceInfo.selectedPiece) && (&nextPiece != &prevSelected) && (*nextPiece != EMPTY_CELL)) // if they are of the color but not the same piece.
		{
			SelectedPieceInfo.selectedPiece = nextPiece;
			SelectedPieceInfo.x = xCord;
			SelectedPieceInfo.y = yCord;
			SelectedPieceInfo.isCurrent = false;
			SelectedPieceInfo.moveQuantity = 0;
		}

		if (&nextPiece != &prevSelected) // nextPiece here will never be null due to previous if statements.
		{
			// updateMoves();

			// will generate the moveset if there's a piece selected and there is not already a moveset made for that specific piece.
			generateMoves();

			int dst[2] = {7 - yCord, xCord};
			int src[2] = {7 - SelectedPieceInfo.y, SelectedPieceInfo.x};

			bool isMovePossible = false;
			for (int moveIndex = 0; moveIndex < SelectedPieceInfo.moveQuantity; moveIndex++)
			{
				if (SelectedPieceInfo.possibleMoves[moveIndex][0] == dst[0] && SelectedPieceInfo.possibleMoves[moveIndex][1] == dst[1])
				{
					Piece *movingPiece = SelectedPieceInfo.selectedPiece;
					int pieceClass = getClass(movingPiece);
					int pieceColor = getColor(movingPiece);
					
					// Check for castling
					int castlingSide = isCastlingMove(gameBoard, movingPiece, src, dst);
					if (castlingSide > 0)
					{
						executeCastling(gameBoard, pieceColor, castlingSide);
						enPassantCol = -1;
						enPassantRow = -1;
					}
					// Check for en passant
					else if (isEnPassantMove(gameBoard, movingPiece, src, dst))
					{
						// Track captured pawn
						capturedBlack[PAWN]++;
						if (pieceColor == 1) {
							capturedWhite[PAWN]++;
							capturedBlack[PAWN]--;
						}
						executeEnPassant(gameBoard, src, dst);
						enPassantCol = -1;
						enPassantRow = -1;
					}
					else
					{
						// Normal move - check if we're capturing a piece
						Piece targetPiece = gameBoard->grid[dst[0]][dst[1]];
						if (targetPiece != EMPTY_CELL) {
							int capturedColor = getColor(&targetPiece);
							int capturedClass = getClass(&targetPiece);
							if (capturedColor == 0) {
								capturedWhite[capturedClass]++;
							} else {
								capturedBlack[capturedClass]++;
							}
						}
						
						// Track en passant opportunity if pawn moves two squares
						if (pieceClass == PAWN && abs(dst[0] - src[0]) == 2)
						{
							enPassantCol = dst[1];
							enPassantRow = dst[0];
						}
						else
						{
							enPassantCol = -1;
							enPassantRow = -1;
						}
						
						// Update castling rights based on piece moved
						if (pieceClass == KING)
						{
							if (pieceColor == 0) wKingMoved = 1;
							else bKingMoved = 1;
						}
						else if (pieceClass == ROOK)
						{
							if (pieceColor == 0)
							{
								if (src[0] == 7 && src[1] == 0) wRookQMoved = 1;
								if (src[0] == 7 && src[1] == 7) wRookKMoved = 1;
							}
							else
							{
								if (src[0] == 0 && src[1] == 0) bRookQMoved = 1;
								if (src[0] == 0 && src[1] == 7) bRookKMoved = 1;
							}
						}
						
						movePiece(gameBoard, SelectedPieceInfo.selectedPiece, dst);
					}
					
					// Switch turns
					currentTurn = 1 - currentTurn;
					
					// Check for check, checkmate, or stalemate
					if (isCheckmate(gameBoard, currentTurn))
					{
						gameOver = (currentTurn == 0) ? 2 : 1; // Opponent wins
						inCheck = 0;
					}
					else if (isStalemate(gameBoard, currentTurn))
					{
						gameOver = 3; // Stalemate
						inCheck = 0;
					}
					else if (isKingInCheck(gameBoard, currentTurn))
					{
						inCheck = currentTurn + 1; // 1 = white in check, 2 = black in check
					}
					else
					{
						inCheck = 0;
					}
					
					isMovePossible = true;
					SelectedPieceInfo.selectedPiece = NULL;
					SelectedPieceInfo.x = -1;
					SelectedPieceInfo.y = -1;
					SelectedPieceInfo.isCurrent = false;
					SelectedPieceInfo.moveQuantity = 0;
					break;
				}
			}
			// this if statement is not needed. I think.
			if ((!isMovePossible) && (dst[0] != 7 - SelectedPieceInfo.y && dst[1] != SelectedPieceInfo.x))
			{
				SelectedPieceInfo.selectedPiece = NULL;
				SelectedPieceInfo.x = -1;
				SelectedPieceInfo.y = -1;
				SelectedPieceInfo.isCurrent = false;
				SelectedPieceInfo.moveQuantity = 0;
			}
		}

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

		// Deselect current piece and clear all selection state
		SelectedPieceInfo.selectedPiece = NULL;
		SelectedPieceInfo.x = -1;
		SelectedPieceInfo.y = -1;
		SelectedPieceInfo.isCurrent = false;
		SelectedPieceInfo.moveQuantity = 0;
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
		return DefWindowProc(local_window_handle, message, wParam, lParam);
	}
	return 0;
}

void generateMoves()
{
	if (SelectedPieceInfo.selectedPiece && !SelectedPieceInfo.isCurrent)
	{

		int src[2] = {7 - SelectedPieceInfo.y, SelectedPieceInfo.x};
		int dst[2] = {0, 0};
		int counter = 0;
		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				dst[0] = 7 - y;
				dst[1] = x;

				if (isValidMove(gameBoard, SelectedPieceInfo.selectedPiece, src, dst))
				{
					// Filter out moves that would leave king in check
					if (!wouldBeInCheck(gameBoard, SelectedPieceInfo.selectedPiece, src, dst))
					{
						SelectedPieceInfo.possibleMoves[counter][0] = dst[0];
						SelectedPieceInfo.possibleMoves[counter][1] = dst[1];
						counter++;
					}
				}
			}
		}

		SelectedPieceInfo.moveQuantity = counter;
		SelectedPieceInfo.isCurrent = true;
	}
}
