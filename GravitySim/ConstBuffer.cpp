#include "pch.h"
#include "ConstBuffer.h"
#include "Quad2D.h"
#include "Utils.h"

HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = nullptr;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
}

HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

	return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
}


HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

	return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
}

ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugbuf = nullptr;

	D3D11_BUFFER_DESC desc = {};
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if (SUCCEEDED(pDevice->CreateBuffer(&desc, nullptr, &debugbuf)))
	{
		pd3dImmediateContext->CopyResource(debugbuf, pBuffer);
	}

	return debugbuf;
}

void PrepareComputeShader(Renderer& renderer, Quad2D& quad)
{
	auto computeShaderCSO = Utils::ReadFile("cso/ComputeShader.cso");
	renderer.mDevice->CreateComputeShader(computeShaderCSO.data(), computeShaderCSO.size(), nullptr, &ComputeShader);

	CreateStructuredBuffer(renderer.mDevice, sizeof(InstanceData), quad.instanceData.size(), quad.instanceData.data(), &ComputeInputBuffer);
	CreateStructuredBuffer(renderer.mDevice, sizeof(InstanceData), quad.instanceData.size(), nullptr, &ComputeOutputBuffer);

	CreateBufferSRV(renderer.mDevice, ComputeInputBuffer, &ComputeInputSRV);
	CreateBufferUAV(renderer.mDevice, ComputeOutputBuffer, &ComputeView);

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(AlignedInt);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride = 0;
	renderer.mDevice->CreateBuffer(&desc, nullptr, &ComputeConstBuffer);
}


void RunComputeShader(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ComputeShader* pComputeShader,
	UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews,
	ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
	ID3D11UnorderedAccessView* pUnorderedAccessView,
	UINT X, UINT Y, UINT Z)
{
	pd3dImmediateContext->CSSetShader(pComputeShader, nullptr, 0);
	pd3dImmediateContext->CSSetShaderResources(0, nNumViews, pShaderResourceViews);
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &pUnorderedAccessView, nullptr);
	if (pCBCS && pCSData)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dImmediateContext->Map(pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		memcpy(MappedResource.pData, pCSData, dwNumDataBytes);
		pd3dImmediateContext->Unmap(pCBCS, 0);
		ID3D11Buffer* ppCB[1] = { pCBCS };
		pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCB);
	}

	pd3dImmediateContext->Dispatch(X, Y, Z);

	pd3dImmediateContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);

	ID3D11ShaderResourceView* ppSRVnullptr[2] = { nullptr, nullptr };
	pd3dImmediateContext->CSSetShaderResources(0, 2, ppSRVnullptr);

	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}

void DispatchComputeShader(Renderer& r, Quad2D& quad, AlignedInt data) {

	D3D11_MAPPED_SUBRESOURCE dat{};
	r.mDContext->Map(ComputeConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dat);
	AlignedInt* mappedData = (AlignedInt*)dat.pData;
	*mappedData = data;
	r.mDContext->Unmap(ComputeConstBuffer, 0);

	r.mDContext->CSSetConstantBuffers(0, 1, &ComputeConstBuffer);
	RunComputeShader(r.mDContext, ComputeShader, 1, &ComputeInputSRV, nullptr, nullptr, 0, ComputeView, data.instanceCount / 100, 1, 1);
}
void ComputeShaderEndFrame(Renderer& r, Quad2D& quad) {
	ID3D11Buffer* outputBuf = CreateAndCopyToDebugBuf(r.mDevice, r.mDContext, ComputeOutputBuffer);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	InstanceData* p;
	r.mDContext->Map(outputBuf, 0, D3D11_MAP_READ, 0, &MappedResource);

	// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	// This is also a common trick to debug CS programs.
	p = (InstanceData*)MappedResource.pData;
	std::memcpy(quad.instanceData.data(), p, quad.instanceData.size() * sizeof(InstanceData));
	r.mDContext->Unmap(outputBuf, 0);
	outputBuf->Release();
}

void FreeComputeShader() {
	ComputeShader->Release();
	ComputeView->Release();
	ComputeInputSRV->Release();
	ComputeOutputBuffer->Release();
}

void UpdateShaderInput(Renderer& r, Quad2D& quad)
{
	//ComputeInputBuffer->Release();
	//ComputeInputSRV->Release();
	D3D11_MAPPED_SUBRESOURCE sr{};
	r.mDContext->Map(ComputeInputBuffer, 0, D3D11_MAP_WRITE, 0, &sr);

	std::memcpy(sr.pData, quad.instanceData.data(), quad.instanceData.size() * sizeof(InstanceData));
	r.mDContext->Unmap(ComputeInputBuffer, 0);

	//CreateStructuredBuffer(r.mDevice, sizeof(InstanceData), quad.instanceData.size(), quad.instanceData.data(), &ComputeInputBuffer);
	//CreateBufferSRV(r.mDevice, ComputeInputBuffer, &ComputeInputSRV);
}
