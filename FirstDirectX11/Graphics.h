#pragma once
#include "WindowWrapper.h"
#include "DXException.h"
#include <d3d11.h>
class Graphics
{
public:
	class Exception : public DXException
	{
		using DXException::DXException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class DeviceRemovedException : HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	};
	Graphics(HWND hWnd);
	Graphics(Graphics&) = delete;
	Graphics& operator=(Graphics&) = delete;
	void EndFrame();
	void ClearBuffer(float r, float g, float b) noexcept;
	~Graphics();
private:
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwap = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	ID3D11RenderTargetView* pTargetView = nullptr;
};


