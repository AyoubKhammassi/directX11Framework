#pragma once
#include "Graphics.h"

//This class is an interface that represents any type of resources that can be binded to the pipeline
class Bindable
{
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;

	//We don't have access to Graphics members unless we make every bindable a friend of Graphics.
	//That way we'll expose everything in Graphics to the Bindables
	//A workaround is making these static functions
protected:
	static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
	static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
	static DXGIInfoManager& GetInfoManager(Graphics* gfx) noexcept(!IS_DEBUG);

};