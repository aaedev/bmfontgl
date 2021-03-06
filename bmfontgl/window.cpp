/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "gl_basics.h"
#include "log.h"
#include "globals.h"
#include "bmfont.h"

#pragma warning (disable : 4996)

HWND hWnd;
RECT MyWindow;

//Default Window Size
int WinWidth = 1024;
int WinHeight = 768;

#pragma comment (lib, "opengl32.lib")  
#pragma comment (lib, "glu32.lib")  

//Fonts
BMFont *Snap;
BMFont *Times;
BMFont *Lucida;


HWND win_get_window(void)
{
	return hWnd;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Simple, generic window init //
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int iCmdShow)
{
	WNDCLASS wc;

	MSG msg;
	bool Quit = FALSE;
	DWORD       dwExStyle;                      // Window Extended Style
	DWORD       dwStyle;                        // Window Style

	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"BMFontGL";
	RegisterClass(&wc);

	dwExStyle = WS_EX_APPWINDOW;   // Window Extended Style    
	dwStyle = WS_OVERLAPPEDWINDOW | WS_THICKFRAME;                    // Windows Style
	RECT WindowRect;                                                 // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;                                         // Set Left Value To 0
	WindowRect.right = (long)WinWidth;                                 // Set Right Value To Requested Width
	WindowRect.top = (long)0;                                          // Set Top Value To 0
	WindowRect.bottom = (long)WinHeight;                               // Set Bottom Value To Requested Height
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);      // Adjust Window To True Requested Size


	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"BMFontGL",							// Class Name
		L"OpenGL BMFont Sample Implementation",						// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		CW_USEDEFAULT, 0,   				// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		// Reset The Display
		MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	//********** Program Initializations *************
	//Enable Logging
	LogOpen("glfonttest.log");
	// enable OpenGL for the window
	CreateGLContext();
	//Basic Window Init
	WRLOG("Starting Program");
	ShowWindow(hWnd, SW_SHOW);                                         // Show The Window
	SetForegroundWindow(hWnd);									    // Slightly Higher Priority
	SetFocus(hWnd);													// Sets Keyboard Focus To The Window
	ReSizeGLScene(WinWidth, WinHeight);										    // Set Up Our Perspective GL Screen
	//Get the Supported OpenGl Version
	CheckGLVersionSupport();
	//Fill in the Window Rect;
	GetClientRect(hWnd, &MyWindow);
	//Set the OpenGl View
	ViewOrtho(WinWidth, WinHeight);

	//Load and Initialize the Fonts
	wrlog("Starting to parse fonts.");

	Lucida = new BMFont(WinWidth, WinHeight);
	if (!Lucida->LoadFont("lucida.fnt"))
	{
		MessageBox(NULL, L"Error, font file not found, exiting", L"File Not Found", MB_ICONERROR | MB_OK);
		Quit = TRUE;
		//PostQuitMessage(-1);
	}
	LOG_DEBUG("Font Loaded Sucessfully");

	Snap = new BMFont(WinWidth, WinHeight);
	if (!Snap->LoadFont("snap.fnt"))
	{
		MessageBox(NULL, L"Error, font file not found, exiting", L"File Not Found", MB_ICONERROR | MB_OK);
		Quit = TRUE;
		//PostQuitMessage(-1);
	}
	LOG_DEBUG("Font Loaded Sucessfully");

	Times = new BMFont(WinWidth, WinHeight);
	if (!Times->LoadFont("times.fnt"))
	{
		MessageBox(NULL, L"Error, font file not found, exiting", L"File Not Found", MB_ICONERROR | MB_OK);
		Quit = TRUE;
		// PostQuitMessage(-1);
	}
	LOG_DEBUG("Font Loaded Sucessfully");

	// ********** Program Main Loop **********

	while (!Quit) {

		// check for messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			// handle or dispatch messages
			if (msg.message == WM_QUIT) {
				Quit = TRUE;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		else {

			// ********** OpenGL drawing code, very simple, just to show off the font. **********

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			// setup texture mapping
			glEnable(GL_TEXTURE_2D);

			glLoadIdentity();
			Times->SetAlign(BMFont::AlignCenter);
			Times->SetScale(1.5f);
			Times->SetColor(250, 251, 252, 255);
			Times->Print(0, 280, "A different font, centered. Kerning: To Ti");

			/*
			Lucida->setColor(250,250,55,254);
			Lucida->PrintCenter(240,"This is another font.");

			Snap->setColor(255,255,255,255);
			Snap->setScale(1.0f);
			Snap->Print(0, 0, "This is standard printing.");
			*/
			Snap->SetColor(RGB_WHITE);
			Snap->SetAngle(0);
			Snap->SetScale(2.0f);
			Snap->SetAlign(BMFont::AlignNear);
			Snap->Print(40.0, 180, "Scaling makes it Big!");

			Snap->SetColor(RGB_WHITE);
			Snap->SetScale(1.0f);
			Snap->SetAngle(90);
			Snap->Print(130.0, 100, "The next line is right here.");
			Snap->SetAngle(0);
			Snap->Print(130.0, 350, "Another long line here.");
			/*
			Snap->setScale(1.0f);
			Snap->Print(2, Snap->getHeight()*4, "Or it can make it smaller!");
			Snap->setScale(1.0f);
			Snap->setColor(25,155,255,255);
			Snap->PrintCenter(320, "Centered printing: To Ti");
		   */
			Times->Render();
			Snap->Render();
			GlSwap();
		}
	}

	// ********** Cleanup and exit gracefully. **********
	// shutdown OpenGL
	delete Lucida;
	delete Snap;
	delete Times;
	DeleteGLContext();
	LogClose();
	// destroy the window 
	DestroyWindow(hWnd);

	return (int)msg.wParam;
}


//Simple Message Pump Proc ///////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			PostQuitMessage(0);  return 0;

		case VK_F1: {SnapShot(); return 0; }

		case VK_F2: {Times->UseKerning(false); Snap->UseKerning(false); Lucida->UseKerning(false); LOG_DEBUG("Times kerning disabled"); return 0; }

		case VK_F3: {Times->UseKerning(true); Snap->UseKerning(true); Lucida->UseKerning(true); LOG_DEBUG("Times kerning enabled"); return 0; }

		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}

}

