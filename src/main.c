#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);

#if RAND_MAX == 32767
#define rand32() ((rand() << 15) + (rand() << 1) + (rand() & 1))
#else
#define rand32() rand()
#endif

bool quit = false;
HWND window_handle;
BITMAPINFO bitmap_info;
HBITMAP bitmap;
HDC bitmap_device_context;

struct {
	union { int w, width; };
	union { int h, height; };
	uint32_t *pixels;
} frame = {0};

bool keyboard[256] = {0};
struct {
	int x, y;
	uint8_t buttons;
} mouse;
enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	const wchar_t window_class_name[] = L"Window Class";
	static WNDCLASS window_class = { 0 };
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

	window_handle = CreateWindow(window_class_name, L"Learn to Program Windows", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if(window_handle == NULL) {
		PRINT_ERROR("CreateWindow() failed. Returned NULL.\n");
		return -1;
	}

	while(!quit) {
		static MSG message = { 0 };
		while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&message); }

		static int keyboard_x = 0, keyboard_y = 0;
		if(keyboard[VK_RIGHT] || keyboard['D']) ++keyboard_x;
		if(keyboard[VK_LEFT]  || keyboard['A']) --keyboard_x;
		if(keyboard[VK_UP]    || keyboard['W']) ++keyboard_y;
		if(keyboard[VK_DOWN]  || keyboard['S']) --keyboard_y;

		if(keyboard_x < 0)			keyboard_x = 0;
		if(keyboard_x > frame.w-1)	keyboard_x = frame.w-1;
		if(keyboard_y < 0)			keyboard_y = 0;
		if(keyboard_y > frame.h-1)	keyboard_y = frame.h-1;

		for(int i = 0; i < 1000; ++i) frame.pixels[rand32() % (frame.w * frame.h)] = 0;

		frame.pixels[keyboard_x + keyboard_y*frame.w] = 0x00ffffff;
		if(mouse.buttons & MOUSE_LEFT) frame.pixels[mouse.x + mouse.y*frame.w] = 0x00ffffff;

		InvalidateRect(window_handle, NULL, FALSE);
		UpdateWindow(window_handle);
	}

	return 0;
}

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool has_focus = true;

	switch(message) {
		case WM_QUIT:
		case WM_DESTROY: {
			quit = true;
		} break;

		case WM_PAINT: {
			static PAINTSTRUCT paint;
			static HDC device_context;
			device_context = BeginPaint(window_handle, &paint);
			BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
			EndPaint(window_handle,&paint);
		} break;

		case WM_SIZE: {
			frame.w = bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
			frame.h = bitmap_info.bmiHeader.biHeight = HIWORD(lParam);
			if(bitmap) DeleteObject(bitmap);
			bitmap = CreateDIBSection(NULL, &bitmap_info, DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
			SelectObject(bitmap_device_context, bitmap);
		} break;

		case WM_KILLFOCUS: {
			has_focus = false;
			memset(keyboard, 0, 256 * sizeof(keyboard[0]));
			mouse.buttons = 0;
		} break;

		case WM_SETFOCUS: has_focus = true; break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			if(has_focus) {
				static bool key_is_down, key_was_down;
				key_is_down  = ((lParam & (1 << 31)) == 0);
				key_was_down = ((lParam & (1 << 30)) != 0);
				if(key_is_down != key_was_down) {
					keyboard[(uint8_t)wParam] = key_is_down;
					if(key_is_down) {
						switch(wParam) {
							case VK_ESCAPE: quit = true; break;
						}
					}
				}
			}
		} break;

		case WM_MOUSEMOVE: {
			mouse.x = LOWORD(lParam);
			mouse.y = frame.h - 1 - HIWORD(lParam);
		} break;

		case WM_LBUTTONDOWN: mouse.buttons |=  MOUSE_LEFT;   break;
		case WM_LBUTTONUP:   mouse.buttons &= ~MOUSE_LEFT;   break;
		case WM_MBUTTONDOWN: mouse.buttons |=  MOUSE_MIDDLE; break;
		case WM_MBUTTONUP:   mouse.buttons &= ~MOUSE_MIDDLE; break;
		case WM_RBUTTONDOWN: mouse.buttons |=  MOUSE_RIGHT;  break;
		case WM_RBUTTONUP:   mouse.buttons &= ~MOUSE_RIGHT;  break;

		case WM_XBUTTONDOWN: {
			if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
					 mouse.buttons |= MOUSE_X1;
			} else { mouse.buttons |= MOUSE_X2; }
		} break;
		case WM_XBUTTONUP: {
			if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
					 mouse.buttons &= ~MOUSE_X1;
			} else { mouse.buttons &= ~MOUSE_X2; }
		} break;

		case WM_MOUSEWHEEL: {
			printf("%s\n", wParam & 0b10000000000000000000000000000000 ? "Down" : "Up");
		} break;

		default: return DefWindowProc(window_handle, message, wParam, lParam);
	}
	return 0;
}