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
public:
	IDXGISwapChain* mSwapChain = nullptr;
	ID3D11DeviceContext* mDContext = nullptr;
	ID3D11Device* mDevice = nullptr;
	ID3D11RenderTargetView* mBackbuffer = nullptr;
private:
	FLOAT clearColor[4] = {0.3f, 0.3f, 0.3f, 1.0f};
};

