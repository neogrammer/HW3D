
#include "Graphics.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <sstream>
#include <d3dcompiler.h>


#include "dxerr.h"

#include <array>

#pragma comment(lib, "D3DCompiler.lib")

namespace dx = DirectX;
namespace wrl = Microsoft::WRL;

#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__, __FILE__, (hr) )
#define GFX_THROW_FAILED(hrcall) if ( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__, __FILE__ , hr )
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__, __FILE__ , (hr) )
#define GFX_THROW_INFO_ONLY(call) (call); { std::string str{""}; auto e = Graphics::InfoException(__LINE__,__FILE__, str); str = e.what(); throw Graphics::InfoException(__LINE__, __FILE__, str); }


Graphics::Graphics(HWND hWnd, int width_, int height_)
	: width{width_}, height{height_}
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	
	sd.BufferDesc.RefreshRate.Numerator = 0;	
	sd.BufferDesc.RefreshRate.Denominator = 0;	
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	
	sd.SampleDesc.Count = 1;	
	sd.SampleDesc.Quality = 0;	
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	
	sd.BufferCount = 1;	
	sd.OutputWindow = hWnd;	
	sd.Windowed = TRUE;	
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	
	sd.Flags = 0;

	HRESULT hr;


	GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	));
	// gain access to texture subresource
	wrl::ComPtr<ID3D11Resource> pBackBuffer = nullptr;
	GFX_THROW_FAILED(pSwap->GetBuffer(0, _uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_FAILED(pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&pTarget
	));
}


void Graphics::EndFrame()
{
	HRESULT hr;
	if (FAILED(hr = pSwap->Present(1u, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			GFX_THROW_FAILED(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue,1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::DrawTestTriangle()
{
	namespace wrl = Microsoft::WRL;
	HRESULT hr;

	struct Vertex
	{
		float x;
		float y;
	};

	const Vertex vertices[] =
	{
		{ 0.f,0.5f },
		{ 0.5f,-0.5f },
		{ -0.5f,-0.5f }
	};


	// create vertex buffer
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = 0u;
	bufDesc.MiscFlags = 0u;
	bufDesc.ByteWidth = sizeof(vertices);
	bufDesc.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sData = {};
	sData.pSysMem = vertices;
	GFX_THROW_FAILED( pDevice->CreateBuffer(&bufDesc, &sData, &pVertexBuffer) );
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	// bind buffer to the pipeline
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);


	

	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_FAILED(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_FAILED(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);



	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_FAILED(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_THROW_FAILED(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),nullptr, &pVertexShader));
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	

	// input layout
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"Position",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	GFX_THROW_FAILED(pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout));

	//bind vertex layout
	pContext->IASetInputLayout(pInputLayout.Get());

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);

	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_VIEWPORT vp;
	vp.Width = 1366;
	vp.Height = 768;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	// draw it
	pContext->Draw((UINT)std::size(vertices), 0u);
}


// Graphics exception stuff
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
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

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
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
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	const WCHAR* ws = DXGetErrorString(hr);

	int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
	

	// Allocate a buffer for the resulting string
	std::string str(len, '\0');

	// Perform the conversion
	WideCharToMultiByte(CP_UTF8, 0, ws, -1, &str[0], len, nullptr, nullptr);

	// Remove the null terminator added by WideCharToMultiByte
	str.resize(len - 1);

	return str;
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	
	WCHAR buf[512];
	
	DXGetErrorDescription(hr, buf, ARRAYSIZE(buf));

		int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
		

		// Allocate a buffer for the resulting string
		std::string str(len, '\0');

		// Perform the conversion
		WideCharToMultiByte(CP_UTF8, 0, buf, -1, &str[0], len, nullptr, nullptr);

		// Remove the null terminator added by WideCharToMultiByte
		str.resize(len - 1);
	
	return str;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

//#include <sstream>
//#include <d3dcompiler.h>
//#include <cmath>
//#include <DirectXMath.h>
//#include <array>
//
//namespace dx = DirectX;
//
//#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"D3DCompiler.lib")
//
//
//Graphics::Graphics(HWND hWnd, int width, int height)
//	:
//	width(width),
//	height(height)
//{
////	DXGI_SWAP_CHAIN_DESC sd = {};
////	sd.BufferDesc.Width = width;
////	sd.BufferDesc.Height = height;
////	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
////	sd.BufferDesc.RefreshRate.Numerator = 0;
////	sd.BufferDesc.RefreshRate.Denominator = 0;
////	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
////	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
////	sd.SampleDesc.Count = 1;
////	sd.SampleDesc.Quality = 0;
////	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
////	sd.BufferCount = 1;
////	sd.OutputWindow = hWnd;
////	sd.Windowed = TRUE;
////	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
////	sd.Flags = 0;
////
////	UINT swapCreateFlags = 0u;
////#ifndef NDEBUG
////	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
////#endif
////
////	// for checking results of d3d functions
////	HRESULT hr;
////
////	// create device and front/back buffers, and swap chain and rendering context
//	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
//		nullptr,
//		D3D_DRIVER_TYPE_HARDWARE,
//		nullptr,
//		swapCreateFlags,
//		nullptr,
//		0,
//		D3D11_SDK_VERSION,
//		&sd,
//		&pSwap,
//		&pDevice,
//		nullptr,
//		&pContext
//	));
////
////	// gain access to texture subresource in swap chain (back buffer)
////	wrl::ComPtr<ID3D11Texture2D> pBackBuffer;
////	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer));
////	pTarget = std::shared_ptr<Bind::RenderTarget>{ new Bind::OutputOnlyRenderTarget(*this,pBackBuffer.Get()) };
////
////	// viewport always fullscreen (for now)
////	D3D11_VIEWPORT vp;
////	vp.Width = (float)width;
////	vp.Height = (float)height;
////	vp.MinDepth = 0.0f;
////	vp.MaxDepth = 1.0f;
////	vp.TopLeftX = 0.0f;
////	vp.TopLeftY = 0.0f;
////	pContext->RSSetViewports(1u, &vp);
////
////	// init imgui d3d impl
////	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
//}
//
//Graphics::~Graphics()
//{
//	//ImGui_ImplDX11_Shutdown();
//}
//
//void Graphics::EndFrame()
//{
////	// imgui frame end
////	if (imguiEnabled)
////	{
////		ImGui::Render();
////		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
////	}
////
////	HRESULT hr;
////#ifndef NDEBUG
////	infoManager.Set();
////#endif
////	if (FAILED(hr = pSwap->Present(1u, 0u)))
////	{
////		if (hr == DXGI_ERROR_DEVICE_REMOVED)
////		{
////			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
////		}
////		else
////		{
////			throw GFX_EXCEPT(hr);
////		}
////	}
//}
//
//void Graphics::BeginFrame(float red, float green, float blue) noexcept
//{
//	//// imgui begin frame
//	//if (imguiEnabled)
//	//{
//	//	ImGui_ImplDX11_NewFrame();
//	//	ImGui_ImplWin32_NewFrame();
//	//	ImGui::NewFrame();
//	//}
//	//// clearing shader inputs to prevent simultaneous in/out bind carried over from prev frame
//	//ID3D11ShaderResourceView* const pNullTex = nullptr;
//	//pContext->PSSetShaderResources(0, 1, &pNullTex); // fullscreen input texture
//	//pContext->PSSetShaderResources(3, 1, &pNullTex); // shadow map texture
//}
//
//void Graphics::DrawIndexed(UINT count) // noxnd
//{
//	//GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));
//}
//
//void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
//{
//	projection = proj;
//}
//
//DirectX::XMMATRIX Graphics::GetProjection() const noexcept
//{
//	return projection;
//}
//
//void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept
//{
//	camera = cam;
//}
//
//DirectX::XMMATRIX Graphics::GetCamera() const noexcept
//{
//	return camera;
//}
//
//void Graphics::EnableImgui() noexcept
//{
//	imguiEnabled = true;
//}
//
//void Graphics::DisableImgui() noexcept
//{
//	imguiEnabled = false;
//}
//
//bool Graphics::IsImguiEnabled() const noexcept
//{
//	return imguiEnabled;
//}
//
//UINT Graphics::GetWidth() const noexcept
//{
//	return width;
//}
//
//UINT Graphics::GetHeight() const noexcept
//{
//	return height;
//}
//
//std::shared_ptr<Bind::RenderTarget> Graphics::GetTarget()
//{
//	return pTarget;
//}
//
//
//// Graphics exception stuff
//Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
//	:
//	Exception(line, file),
//	hr(hr)
//{
//	// join all info messages with newlines into single string
//	for (const auto& m : infoMsgs)
//	{
//		info += m;
//		info.push_back('\n');
//	}
//	// remove final newline if exists
//	if (!info.empty())
//	{
//		info.pop_back();
//	}
//}
//
//const char* Graphics::HrException::what() const noexcept
//{
//	std::ostringstream oss;
//	oss << GetType() << std::endl
//		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
//		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
//		<< "[Error String] " << GetErrorString() << std::endl
//		<< "[Description] " << GetErrorDescription() << std::endl;
//	if (!info.empty())
//	{
//		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
//	}
//	oss << GetOriginString();
//	whatBuffer = oss.str();
//	return whatBuffer.c_str();
//}
//
//const char* Graphics::HrException::GetType() const noexcept
//{
//	return "Chili Graphics Exception";
//}
//
//HRESULT Graphics::HrException::GetErrorCode() const noexcept
//{
//	return hr;
//}
//
//std::string Graphics::HrException::GetErrorString() const noexcept
//{
//	return "";//DXGetErrorString(hr);
//}
//
//std::string Graphics::HrException::GetErrorDescription() const noexcept
//{
//	return "";
//	/*char buf[512];
//	DXGetErrorDescription(hr, buf, sizeof(buf));
//	return buf;*/
//}
//
//std::string Graphics::HrException::GetErrorInfo() const noexcept
//{
//	return info;
//}
//
//
//const char* Graphics::DeviceRemovedException::GetType() const noexcept
//{
//	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
//}

	

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

Graphics::InfoException::InfoException(int line, const char* file, std::string message)
	:
	Exception(line, file)
{
	info = message;
}
