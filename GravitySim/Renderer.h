#pragma once
#include "pch.h"

class Renderer
{
public:
	Renderer(HWND windowInstance, int w, int h);
	~Renderer();

	void PrepareRender();
	void Present();
public:
	IDXGISwapChain* mSwapChain = nullptr;
	ID3D11DeviceContext* mDContext = nullptr;
	ID3D11Device* mDevice = nullptr;
	ID3D11RenderTargetView* mBackbuffer = nullptr;
};

