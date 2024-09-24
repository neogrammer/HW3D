#include <Windows.h>

int messageHandler();



LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(69);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);

}
BOOL gResult{};
MSG e;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	const auto pClassName = L"hw3dButts";
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = (LPCWSTR)pClassName;
	wc.hIconSm = nullptr;
	//register window class
	RegisterClassEx(&wc);
	// create window
	HWND hWnd = CreateWindowEx(
		0,
		pClassName,
		L"Happy Hard Window",
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		200,200,640,480,
		nullptr, nullptr,  hInstance, nullptr
	);

	ShowWindow(hWnd, SW_SHOW);

	
	messageHandler();

	if (gResult == -1)
	{
		return -1;
	}
	else
	{
		return (int)e.wParam;
	}
}

int messageHandler()
{
	while (gResult = (GetMessage(&e, nullptr, 0, 0)) > 0)
	{
		TranslateMessage(&e);
		DispatchMessage(&e);

	}
	return gResult;
}
