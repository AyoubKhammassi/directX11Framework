#pragma once
#include "WindowWrapper.h"
#include "DXException.h"
#include <d3d11.h>
#include <vector>
#include "DXGIInfoManager.h"

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
		HrException(int line, const char* file, HRESULT hr,OPTIONAL std::vector<std::string> info) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class DeviceRemovedException : public  HrException
	{
	public:
		using HrException::HrException;
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
	Graphics(HWND hWnd);
	Graphics(Graphics&) = delete;
	Graphics& operator=(Graphics&) = delete;
	void EndFrame();
	void ClearBuffer(float r, float g, float b) noexcept;
	~Graphics() = default;
private:
#ifndef NDEBUG
	DXGIInfoManager infoManager;
#endif
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwap = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	ID3D11RenderTargetView* pTargetView = nullptr;
};


