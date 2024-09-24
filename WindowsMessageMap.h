#pragma once
#include <unordered_map>
#include <Windows.h>
#include <string>
class WindowsMessageMap
{
	std::unordered_map<DWORD, std::string> map;
public:
	WindowsMessageMap() noexcept;
	std::string operator()(DWORD msg, LPARAM lp, WPARAM wp) const noexcept;

};

