/*
 * clip2qr (pure Windows WIN32 variant)
 *
 * Copyright (c) Peter Lawrence. (MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <windows.h>
#include "resource.h"
#include "qrcodegen.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HINSTANCE hInst;
static TCHAR szAppName[] = TEXT("Clip2QR");
static enum qrcodegen_Ecc ecc_level = qrcodegen_Ecc_MEDIUM;
static HMENU hMenu;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	WNDCLASS wndclass =
	{
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WndProc,
		.cbClsExtra    = 0,
		.cbWndExtra    = 0,
		.hInstance     = hInstance,
		.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON)),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH),
		.lpszMenuName  = NULL,
		.lpszClassName = szAppName,
	};
	
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("not compatible with OS"), szAppName, MB_ICONERROR);
		return 0;
	}

	hInst = hInstance;

	HWND hwnd = CreateWindow(szAppName, 
	                      szAppName,
	                      WS_OVERLAPPEDWINDOW,
	                      CW_USEDEFAULT, CW_USEDEFAULT,
	                      CW_USEDEFAULT, CW_USEDEFAULT,
	                      NULL, NULL, hInstance, NULL);
	
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

static void update_checkboxes(void)
{
	for (WORD level = IDM_ECC_LOW; level <= IDM_ECC_HIGH; level++)
		CheckMenuItem(hMenu, level, (ecc_level == (level - IDM_ECC_LOW)) ? MF_CHECKED : MF_UNCHECKED);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndNextViewer;

	switch (message)
	{
	case WM_CREATE:

		hwndNextViewer = SetClipboardViewer(hwnd);
		hMenu = LoadMenu(hInst, szAppName);
		hMenu = GetSubMenu(hMenu, 0);
		update_checkboxes();

		return 0;
		
	case WM_CHANGECBCHAIN:

		if ((HWND)wParam == hwndNextViewer)
			hwndNextViewer = (HWND)lParam;
		
		else if (hwndNextViewer)
			SendMessage(hwndNextViewer, message, wParam, lParam);
		
		return 0;
		
	case WM_DRAWCLIPBOARD:

		if (hwndNextViewer)
			SendMessage(hwndNextViewer, message, wParam, lParam);
		
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
		
	case WM_PAINT: ;

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rect;
		GetClientRect(hwnd, &rect);
		OpenClipboard(hwnd);

		HGLOBAL hGlobal = GetClipboardData(CF_TEXT);
		if (hGlobal != NULL)
		{
			static uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
			static uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
			RECT tile;
			bool ok;

			PTSTR pGlobal = (PTSTR)GlobalLock(hGlobal);

			ok = qrcodegen_encodeText(pGlobal, tempBuffer,
				qrcode, ecc_level,
				qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
				qrcodegen_Mask_0, true);

			GlobalUnlock(hGlobal);

			if (ok)
			{
				HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
				HBRUSH hWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);

				const int size = qrcodegen_getSize(qrcode);
				const int span = (rect.bottom < rect.right) ? rect.bottom : rect.right;
				const int tile_width = span / (size + 2);
				const int width = tile_width * (size + 2);
				const int x_offset = (rect.right - width) / 2;
				const int y_offset = (rect.bottom - width) / 2;

				SetRect(&tile,
					x_offset, y_offset,
					x_offset + (size + 2) * tile_width, y_offset + (size + 2) * tile_width);
				FillRect(hdc, &tile, hWhite);

				for (int y = 0; y < size; y++)
					for (int x = 0; x < size; x++)
					{
						if (!qrcodegen_getModule(qrcode, x, y)) continue;
						SetRect(&tile,
							x_offset + tile_width * (x + 1),
							y_offset + tile_width * (y + 1),
							x_offset + tile_width * (x + 2),
							y_offset + tile_width * (y + 2));
						FillRect(hdc, &tile, hBlack);
					}
			}
		}
		
		CloseClipboard();
		EndPaint(hwnd, &ps);
		return 0;
		
	case WM_RBUTTONUP: ;

		POINT point = { .x = LOWORD(lParam), .y = HIWORD(lParam) };
		ClientToScreen (hwnd, &point);
		TrackPopupMenu (hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
		return 0;

	case WM_COMMAND: ;

		WORD id = LOWORD (wParam);
		switch (id)
		{
		case IDM_ECC_LOW:
		case IDM_ECC_MEDIUM:
		case IDM_ECC_QUARTILE:
		case IDM_ECC_HIGH:

			ecc_level = id - IDM_ECC_LOW;
			update_checkboxes();
			InvalidateRect(hwnd, NULL, TRUE);

			return 0;
		}
		break;

	case WM_DESTROY:

		ChangeClipboardChain(hwnd, hwndNextViewer);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
