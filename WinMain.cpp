#include <Windows.h>
#include "ChiliWin.h"
#include "WindowsMessageMap.h"
#include <wchar.h>
#include <sstream>
#include <string>
#include "Window.h"
#include "App.h"
#include "ChiliException.h"

MSG msg;
BOOL gResult;


// Function to convert UTF-8 string to UTF-16
std::wstring utf8ToUtf16(const std::string& utf8Str) {
	int utf16Size = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
	if (utf16Size == 0) {
		throw std::runtime_error("Error converting UTF-8 to UTF-16");
	}
	std::wstring utf16Str(utf16Size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &utf16Str[0], utf16Size);
	return utf16Str;
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		return App{}.Go();
	}
	catch (const ChiliException& e)
	{
		std::wstring message = utf8ToUtf16(e.what());
		std::wstring type = utf8ToUtf16(e.GetType());
		MessageBox(nullptr, message.c_str(), type.c_str(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		std::wstring message = utf8ToUtf16(e.what());
		
		MessageBox(nullptr,message.c_str(), L"EAT MY ASS", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, L"Pussy ass mofukka", L"Ummmmm", MB_OK | MB_ICONEXCLAMATION);
	}

	
	return -1;
}
