#pragma once
#include "WindowWrapper.h"
#include "DXException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>
#include "DXException.h"

class Graphics;

class Window
{
	//exception class
public:
	class Exception : public DXException
	{
	public:
		using DXException::DXException;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorString() const noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGraphicsException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
private:
	//singeton class for registering and unregistering the window class
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static constexpr const char* wndClassName = "Direct3D Engine Window";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};
public:
	Window(int width, int height, const char* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator= (const Window&) = delete;
	//settitle here if u ever need it

	static std::optional<int> ProcessMessages();
	Graphics& getGraphicsPointer() const noexcept;


private:
	static LRESULT CALLBACK	HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK	HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
private:
	int width;
	int height;
	HWND hWnd; 
	std::unique_ptr<Graphics> pGraphics;

public:
	Keyboard keyboard;
	Mouse mouse;

};


//error exception helper macros
#define DXEXPT(hr) Window::HrException::HrException(__LINE__,__FILE__,hr)
#define LAST_DXEXPT() Window::HrException::HrException(__LINE__,__FILE__, GetLastError())
#define WND_NOGRAPHICS_EXCEPT() Window::NoGraphicsException(__LINE__,__FILE__)
