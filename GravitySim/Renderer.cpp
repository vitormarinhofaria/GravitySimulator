#include "pch.h"
#include "Renderer.h"

static Renderer* renderer;

Renderer::Renderer(HWND windowInstance, int w, int h) : mWidth(w), mHeight(h)
{
	DXGI_SWAP_CHAIN_DESC scd{};
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = windowInstance;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = w;
	scd.BufferDesc.Height = h;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	
	int creationFlags = 0;
#ifndef NDEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL fLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	auto result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, fLevels, ARRAYSIZE(fLevels), D3D11_SDK_VERSION, &scd, &mSwapChain, &mDevice, nullptr, &mDContext);
	if (FAILED(result)) {
		std::cout << "Failed to create Device and Swapchain\n";
	}

#ifndef NDEBUG
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
#endif
	{
		D3D11_TEXTURE2D_DESC fbDesc{};
		fbDesc.Height = h;
		fbDesc.Width = w;
		fbDesc.ArraySize = 1;
		fbDesc.MipLevels = 1;
		fbDesc.SampleDesc.Count = 1;
		fbDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		fbDesc.Usage = D3D11_USAGE_DEFAULT;
		fbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		result = mDevice->CreateTexture2D(&fbDesc, nullptr, &mFramebuffer);
		assert(!FAILED(result));
		result = mDevice->CreateShaderResourceView(mFramebuffer, nullptr, &mFramebufferTexResource);
		assert(!FAILED(result));
		result = mDevice->CreateRenderTargetView(mFramebuffer, 0, &mFramebufferView);
		assert(!FAILED(result));

		fbDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		fbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ID3D11Texture2D* depthBuffer = nullptr;
		result = mDevice->CreateTexture2D(&fbDesc, nullptr, &depthBuffer);
		assert(!FAILED(result));
		result = mDevice->CreateDepthStencilView(depthBuffer, nullptr, &mDepthStencilView);
		assert(!FAILED(result));
	}

	{
		ID3D11Texture2D* frameBuffer = nullptr;
		result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer);
		assert(!FAILED(result));
		result = mDevice->CreateRenderTargetView(frameBuffer, nullptr, &mPresentBufferView);
		assert(!FAILED(result));

		D3D11_TEXTURE2D_DESC presentDepthDesc{};
		frameBuffer->GetDesc(&presentDepthDesc);
		presentDepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		presentDepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		
		ID3D11Texture2D* presentDepth = nullptr;
		result = mDevice->CreateTexture2D(&presentDepthDesc, nullptr, &presentDepth);
		assert(!FAILED(result));

		result = mDevice->CreateDepthStencilView(presentDepth, nullptr, &mPresentDepthStencilView);
		assert(!FAILED(result));

		frameBuffer->Release();
	}

	{
		D3D11_RASTERIZER_DESC rasterDesc{};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		result = mDevice->CreateRasterizerState(&rasterDesc, &mRasterizerState);
		assert(!FAILED(result));
	}
	{
		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		result = mDevice->CreateDepthStencilState(&depthDesc, &mDepthStencilState);
		assert(!FAILED(result));

	}
	
	/*D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = w;
	viewport.Height = h;
	mDContext->RSSetViewports(1, &viewport);*/
}

Renderer::~Renderer()
{
	mSwapChain->Release();
	mDevice->Release();
	mDContext->Release();
}

void Renderer::InitGlobal(HWND window, int w, int h)
{
	renderer = new Renderer(window, w, h);
}

Renderer* Renderer::Get()
{
	return renderer;
}

void Renderer::PrepareRender()
{
	mDContext->ClearRenderTargetView(mFramebufferView, clearColor);
	mDContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	RECT window_rect{};
	GetClientRect(window, &window_rect);
	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, mWidth, mHeight, 0.0f, 1.0f };
	mDContext->RSSetViewports(1, &viewport);
	mDContext->RSSetState(mRasterizerState);

	mDContext->OMSetRenderTargets(1, &mFramebufferView, mDepthStencilView);
	mDContext->OMSetDepthStencilState(mDepthStencilState, 0);
}

void Renderer::Present(bool vsync)
{
	mDContext->ClearRenderTargetView(mPresentBufferView, clearColor);
	mDContext->ClearDepthStencilView(mPresentDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	RECT window_rect{};
	GetClientRect(window, &window_rect);
	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, mWidth, mHeight, 0.0f, 1.0f };
	mDContext->RSSetViewports(1, &viewport);
	mDContext->RSSetState(mRasterizerState);

	mDContext->OMSetRenderTargets(1, &mPresentBufferView, mPresentDepthStencilView);
	mDContext->OMSetDepthStencilState(mDepthStencilState, 0);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	mSwapChain->Present(vsync, 0);
}

void Renderer::SetClearColor(DirectX::SimpleMath::Vector3 color)
{
	clearColor[0] = color.x;
	clearColor[1] = color.y;
	clearColor[2] = color.z;
}

void Renderer::Render()
{
}

void Renderer::ResizePresentBuffer(int w, int h, bool fullscreen)
{
	//mWidth = w;
	//mHeight = h;
	int scFlags = 0;
	if (fullscreen) {
		scFlags |= DXGI_SWAP_CHAIN_FLAG_FULLSCREEN_VIDEO;
	}
	mPresentBufferView->Release();
	mPresentDepthStencilView->Release();
	mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	{
		HRESULT result;
		ID3D11Texture2D* frameBuffer = nullptr;
		result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer);
		assert(!FAILED(result));
		result = mDevice->CreateRenderTargetView(frameBuffer, nullptr, &mPresentBufferView);
		assert(!FAILED(result));

		D3D11_TEXTURE2D_DESC presentDepthDesc{};
		frameBuffer->GetDesc(&presentDepthDesc);
		presentDepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		presentDepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* presentDepth = nullptr;
		result = mDevice->CreateTexture2D(&presentDepthDesc, nullptr, &presentDepth);
		assert(!FAILED(result));

		result = mDevice->CreateDepthStencilView(presentDepth, nullptr, &mPresentDepthStencilView);
		assert(!FAILED(result));

		frameBuffer->Release();
	}
	ResizeFramebuffer(w, h);
}

void Renderer::ResizeFramebuffer(int w, int h)
{
	mWidth = w;
	mHeight = h;
	{
		mFramebuffer->Release();
		mFramebufferTexResource->Release();
		mFramebufferView->Release();
		mDepthStencilView->Release();

		HRESULT result;
		D3D11_TEXTURE2D_DESC fbDesc{};
		fbDesc.Height = h;
		fbDesc.Width = w;
		fbDesc.ArraySize = 1;
		fbDesc.MipLevels = 1;
		fbDesc.SampleDesc.Count = 1;
		fbDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		fbDesc.Usage = D3D11_USAGE_DEFAULT;
		fbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		result = mDevice->CreateTexture2D(&fbDesc, nullptr, &mFramebuffer);
		assert(!FAILED(result));
		result = mDevice->CreateShaderResourceView(mFramebuffer, nullptr, &mFramebufferTexResource);
		assert(!FAILED(result));
		result = mDevice->CreateRenderTargetView(mFramebuffer, 0, &mFramebufferView);
		assert(!FAILED(result));

		fbDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		fbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ID3D11Texture2D* depthBuffer = nullptr;
		result = mDevice->CreateTexture2D(&fbDesc, nullptr, &depthBuffer);
		assert(!FAILED(result));
		result = mDevice->CreateDepthStencilView(depthBuffer, nullptr, &mDepthStencilView);
		assert(!FAILED(result));
	}
}
