#pragma once
#include "pch.h"

class Renderer
{
public:
	Renderer(HWND windowInstance, int w, int h);
	~Renderer();

	void PrepareRender();
	void Present(bool vsync);
	void SetClearColor(DirectX::SimpleMath::Vector3 color);
	void Render();
	void ResizePresentBuffer(int w, int h, bool fullscream);
	void ResizeFramebuffer(int w, int h);
public:
	IDXGISwapChain* mSwapChain = nullptr;
	ID3D11DeviceContext* mDContext = nullptr;
	ID3D11Device* mDevice = nullptr;

	ID3D11Texture2D* mFramebuffer = nullptr;
	ID3D11ShaderResourceView* mFramebufferTexResource = nullptr;;
	ID3D11RenderTargetView* mFramebufferView = nullptr;;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;;

	ID3D11RenderTargetView* mPresentBufferView = nullptr;;
	ID3D11DepthStencilView* mPresentDepthStencilView = nullptr;;

	ID3D11DepthStencilState* mDepthStencilState = nullptr;;
	ID3D11RasterizerState* mRasterizerState = nullptr;;
private:
	FLOAT clearColor[4] = {0.3f, 0.3f, 0.3f, 1.0f};
public:
	HWND window;
	int mWidth, mHeight;
};

