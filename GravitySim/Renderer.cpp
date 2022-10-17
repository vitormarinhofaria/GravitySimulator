#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(HWND windowInstance, int w, int h)
{
	DXGI_SWAP_CHAIN_DESC scd{};
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = windowInstance;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = w;
	scd.BufferDesc.Height = h;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.SampleDesc.Count = 4;
	

	D3D_FEATURE_LEVEL fLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	auto result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, fLevels, ARRAYSIZE(fLevels), D3D11_SDK_VERSION, &scd, &mSwapChain, &mDevice, nullptr, &mDContext);
	if (FAILED(result)) {
		std::cout << "Failed to create Device and Swapchain\n";
	}

	ID3D11Debug* debug = nullptr;
	mDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
	if (debug)
	{
		ID3D11InfoQueue* info_q = nullptr;
		if (!FAILED(debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_q)))
		{
			info_q->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			info_q->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			info_q->Release();
		}
		debug->Release();
	}

	ID3D11Texture2D* backbuffer = nullptr;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

	mDevice->CreateRenderTargetView(backbuffer, nullptr, &mBackbuffer);
	backbuffer->Release();

	mDContext->OMSetRenderTargets(1, &mBackbuffer, nullptr);
	
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = w;
	viewport.Height = h;
	mDContext->RSSetViewports(1, &viewport);
}

Renderer::~Renderer()
{
	mSwapChain->Release();
	mBackbuffer->Release();
	mDevice->Release();
	mDContext->Release();
}

void Renderer::PrepareRender()
{
	FLOAT clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	mDContext->ClearRenderTargetView(mBackbuffer, clearColor);
}

void Renderer::Present()
{
	mSwapChain->Present(0, 0);
}
