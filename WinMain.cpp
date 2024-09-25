#include <Windows.h>
#include "WindowsMessageMap.h"
#include <wchar.h>
#include <sstream>
#include <string>
#include "Window.h"
#include "FUCK.h"

MSG msg;
BOOL gResult;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
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
	catch (const FUCK& e)
	{
		MessageBox(nullptr, (LPCWSTR)e.what(), (LPCWSTR)e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, (LPCWSTR)e.what(), L"EAT MY ASS", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, L"Pussy ass mofukka", L"Ummmmm", MB_OK | MB_ICONEXCLAMATION);
	}

	
	return -1;
}
