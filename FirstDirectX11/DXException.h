#pragma once
#include <exception>
#include <string>
#include <sstream>



class DXException : public std::exception
{
public:
	DXException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;

private:
	int line;
	std::string file;

protected:
	//mutable so that const functions can write to it
	mutable std::string whatBuffer;
};

