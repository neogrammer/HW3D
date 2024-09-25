#include <Windows.h>
#include "WindowsMessageMap.h"
#include <wchar.h>
#include <sstream>
#include <string>
#include "Window.h"
int messageHandler();


MSG msg;
BOOL gResult;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window wnd(1366, 768, "Donkey Fart Box");

	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	

	if (gResult == -1)
	{
		return -1;
	}
	else
	{
		return (int)msg.wParam;
	}
}
