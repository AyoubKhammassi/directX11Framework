#include "DXException.h"


DXException::DXException(int line, const char * file) noexcept
:
line(line),
file(file)
{}

const char * DXException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString() << std::endl;
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char * DXException::GetType() const noexcept
{
	return "Basic Exception";
}

int DXException::GetLine() const noexcept 
{ 
	return line; 
}

const std::string & DXException::GetFile() const noexcept  
{ 
	return file; 
}

std::string DXException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
	<< "[Line] " << line;

	return oss.str();
}

