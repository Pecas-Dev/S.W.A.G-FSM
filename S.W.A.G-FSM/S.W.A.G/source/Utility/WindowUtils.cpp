#include <windows.h>
#include <Commctrl.h> 
#pragma comment(lib, "Comctl32.lib")


// Custom window procedure that intercepts and blocks window movement messages, this ensures the window cannot be moved by dragging the title bar or using system commands.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	(void)uIdSubclass;
	(void)dwRefData;

	switch (uMsg)
	{
	case WM_NCLBUTTONDOWN:
		if (wParam == HTCAPTION)
		{
			return 0;
		}
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MOVE)
		{
			return 0;

		}
		break;
	}

	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// Makes a window completely non-movable and non-resizable, removes resize/minimize/maximize capabilities and blocks all attempts to move the window.
extern "C" void MakeWindowNonMovableByHandle(void* windowHandle)
{
	HWND hwnd = (HWND)windowHandle;

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	style &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	SetWindowLongPtr(hwnd, GWL_STYLE, style);

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	SetWindowSubclass(hwnd, WindowProc, 1, 0);
}