#include "FUCK.h"
#include <sstream>


FUCK::FUCK(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* FUCK::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* FUCK::GetType() const noexcept
{
	return "FUCK me runnning";
}

int FUCK::GetLine() const noexcept
{
	return line;
}

const std::string& FUCK::GetFile() const noexcept
{
	return file;
}

std::string FUCK::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}