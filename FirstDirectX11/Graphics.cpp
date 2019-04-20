#include "Graphics.h"
#include "dxerr.h"
#include <sstream>

#pragma comment(lib, "d3d11.lib")

#define GFX_THROW_FAILED(hrcall) if( FAILED(hr=(hrcall))) throw Graphics::HrException(__LINE__, __FILE__,hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__, __FILE__,hr)

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//use the window height and width
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Width = 0;
	//layout of pixels of the buffer
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//use the salready picked refresh rate
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	//we're using the height and wedth of the window so no need to specify the scaling effect
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//same for scanline mode, no need to specify
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	
	//4xMSAA config, no antialiasing 1 and 0
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	//number of back buffers used in the swap chain
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//the client window that will display the frames
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;

	//the swap effect used in the presentation of the frame, this one is the most used, the vanilla basically
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//no flags for now
	sd.Flags = 0;

	//hr that the GFX_THROW_FAILED MACRO uses must be defined in the local scope
	HRESULT hr;


	GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, nullptr, &pContext));

	//gain access to texture subresourcs in swap chain
	ID3D11Resource* pBackBuffer = nullptr;
	pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTargetView);
	pBackBuffer->Release();

}

void Graphics::EndFrame()
{
	HRESULT hr;
	if (FAILED(hr = pSwap->Present(1u, 0u)))
	{
		if (hr = DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			GFX_THROW_FAILED(hr);
		}
	}
}

void Graphics::ClearBuffer(float r, float g, float b) noexcept
{
	const float color[] = { r,g,b,1.0f };
	pContext->ClearRenderTargetView(pTargetView, color);

}

Graphics::~Graphics()
{
	if (pTargetView != nullptr)
		pTargetView->Release();
	if (pSwap != nullptr)
		pSwap->Release();
	if (pContext != nullptr)
		pContext->Release();
	if (pDevice !=nullptr)
		pDevice->Release();
}

Graphics::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << "(" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String]" << GetErrorString() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl
		<< GetOriginString();

	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
