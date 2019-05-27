#pragma once
#include "WindowWrapper.h"
#include <vector>
#include <wrl.h>
#include <dxgidebug.h>
#include <string>

class DXGIInfoManager
{
public:
	DXGIInfoManager();
	~DXGIInfoManager();
	DXGIInfoManager(const DXGIInfoManager&) = delete;
	DXGIInfoManager& operator=(const DXGIInfoManager&) = delete;
	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDXGIInfoQueue = nullptr;

	//struct //IDXGIInfoQueue* pDXGIInfoQueue = nullptr;
};

