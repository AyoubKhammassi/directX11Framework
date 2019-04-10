#pragma once
#include "WindowWrapper.h"
#include <d3d11.h>
class Graphics
{
public:
	Graphics(HWND hWnd);
	Graphics(Graphics&) = delete;
	Graphics& operator=(Graphics&) = delete;
	void EndFrame();
	~Graphics();
private:
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwap = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
};


