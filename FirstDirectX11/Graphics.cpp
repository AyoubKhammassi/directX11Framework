#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include<DirectXMath.h>


namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Graphics::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

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

	UINT swapCreateFalgs = 0u;
#ifndef NDEBUG
	swapCreateFalgs |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//hr that the GFX_THROW_FAILED MACRO uses must be defined in the local scope
	HRESULT hr;


	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, swapCreateFalgs, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, nullptr, &pContext));

	//gain access to texture subresourcs in swap chain
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTargetView));
	//GFX_THROW_INFO(pBackBuffer->Release());

#pragma region Depth and Stencil Buffer
	//CREATE DEPTH STENCIL STATE
	//Fill the description structure
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;//the smaller the value, the more near it is to the screen
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //Mask everything

	//Fill the pointer
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	//Binding the depth stencil state to the output merger state
	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	//CREATE THE DEPTH BUFFER TEXTURE
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u; //You can have an array of textures in one texture resource
	descDepth.Format = DXGI_FORMAT_D32_FLOAT; //32bit depth floating point values in each pixel
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth,nullptr,&pDepthStencil));

	//CREATE THE VIEW FOR THE DEPTH STENCIL TEXTURE
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDepthStencilView));

	//BIND DEPTH STENCIL VIEW TO THE OUTPUT MERGER STAGE
	pContext->OMSetRenderTargets(1u,pTargetView.GetAddressOf(),pDepthStencilView.Get());

#pragma endregion
}

void Graphics::EndFrame()
{
	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	if (FAILED(hr = pSwap->Present(1u, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float r, float g, float b) noexcept
{
	const float color[] = { r,g,b,1.0f };
	pContext->ClearRenderTargetView(pTargetView.Get(), color);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);


}

void Graphics::DrawTriangle(float angle, float x, float y)
{
	//hr that the GFX_THROW_FAILED MACRO uses must be defined in the local scope
	HRESULT hr;

#pragma region Vertex buffer
	struct vertex {
		struct
		{
			float x;
			float y;
			float z;
		}pos;
	};
	//CRETAE the vertices and the vertex buffer
	const vertex vertices[] =
	{
		//Back face
		{-1.0f,-1.0f,-1.0f},
		{1.0f,-1.0f,-1.0f},
		{-1.0f,1.0f,-1.0f},
		{1.0f,1.0f,-1.0f},

		//Front Face
		{-1.0f,-1.0f,1.0f},
		{1.0f,-1.0f,1.0f},
		{-1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f},
	};

	//creating the vertex buffer
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(vertices);
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = sizeof(vertex);
	bd.Usage = D3D11_USAGE_DEFAULT;


	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));

	//bind the created vertex buffer to the pipeline
	const UINT stride = sizeof(vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
#pragma endregion

#pragma region Index Buffer
	const unsigned short indices[]
	{

		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4

	};
	//creating the index buffer
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = sizeof(indices);
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.StructureByteStride = sizeof(unsigned short);
	ibd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;

	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	//bind the created index buffer to the pipeline
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
#pragma endregion

#pragma region Transform Constant Buffer 
	//Create constant buffer
	struct ConstantBuffer
	{
		/*struct
		{
			float element[4][4];
		} transformation;*/
		dx::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		/*{
			std::cos(angle), std::sin(angle), 0.0f,0.0f,
			-std::sin(angle),std::cos(angle),0.0f,0.0f,
			0.0f,			0.0f,			1.0f,0.0f,
			0.0f,			0.0f,			0.0f,1.0f,
		}*/
		dx::XMMatrixTranspose(
			dx::XMMatrixRotationZ(angle)*
			dx::XMMatrixRotationX(angle) *
			dx::XMMatrixTranslation(x,y,4.0f) *
			dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f,0.5f,10.0f))
	};
	//creating the constant buffer
	wrl::ComPtr<ID3D11Buffer> pConstBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.ByteWidth = sizeof(cb);
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.StructureByteStride = 0u;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	//NOTE: this a dynamic resource, it will be updated by the CPU each frame, so the CPU access flag need to be set to write

	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;

	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstBuffer));
	//pDevice->CreateBuffer(&cbd, &csd, &pConstBuffer);
	//bind the created constant buffer to the pipeline
	pContext->VSSetConstantBuffers(0u, 1u, pConstBuffer.GetAddressOf());
#pragma endregion

#pragma region Color Constant Buffer

	struct ConstantBuffer2
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		}face_colors[6];
	};

	const ConstantBuffer2 cb2
	{
		{
			{1.0f,0.0f,1.0f},
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,1.0f},
		}
	};

	//creating the constant buffer
	wrl::ComPtr<ID3D11Buffer> pConstBuffer2;
	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.StructureByteStride = 0u;
	cbd2.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;

	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd2, &csd2, &pConstBuffer2));
	//pDevice->CreateBuffer(&cbd2, &csd, &pConstBuffer);
	//bind the created constant buffer to the pipeline
	pContext->PSSetConstantBuffers(0u, 1u, pConstBuffer2.GetAddressOf());
#pragma endregion


#pragma region Pixel Shader
	//create pixel shader 
	wrl::ComPtr<ID3D11PixelShader> pPixelShder;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShder));

	//bind pixel shader to the pipeline
	pContext->PSSetShader(pPixelShder.Get(), nullptr, 0u);
#pragma endregion

#pragma region Vertex Shader
	//create vertex shader 
	wrl::ComPtr<ID3D11VertexShader> pVertexShder;
	GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShder));

	//bind vertex shader to the pipeline
	pContext->VSSetShader(pVertexShder.Get(), nullptr, 0u);
#pragma endregion

	//Input layout (vertex shader)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[]
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//{"COLOR",0, DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	GFX_THROW_INFO(pDevice->CreateInputLayout((ied), (UINT)std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));

	//bind vertex layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//set primitive topology to traingle list (groups of 3 vertoces) PS: check the doc for other topologies
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//configuring the viewport
	D3D11_VIEWPORT vp;
	vp.Height = 600;
	vp.Width = 800;
	vp.MaxDepth = 1;
	vp.MinDepth = 0;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	GFX_THROW_INFO_ONLY(pContext->DrawIndexed(36u,0u,0u));
}

/*Graphics::~Graphics()
{
	if (pTargetView != nullptr)
		pTargetView->Release();
	if (pSwap != nullptr)
		pSwap->Release();
	if (pContext != nullptr)
		pContext->Release();
	if (pDevice !=nullptr)
		pDevice->Release();
}*/

Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	for (const auto& i : infoMsgs)
	{
		info += i;
		info.push_back('\n');
	}
	//remove final newline if exists
	if (!info.empty())
		info.pop_back();
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << "(" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String]" << GetErrorString() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	
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

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return std::string();
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}



Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}


const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}

