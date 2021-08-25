// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "VideoTextureGenerator.h"
#include "HololensResearchModeUtility.h"
#include "Misc/App.h"

#if PLATFORM_HOLOLENS

FVideoTextureGenerator::FVideoTextureGenerator()
{
	Init();
}

FVideoTextureGenerator::~FVideoTextureGenerator()
{
	ClearAll();
}

void FVideoTextureGenerator::Init()
{
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_1;

	TComPtr<ID3D11Device> D3DDevice;
	TComPtr<ID3D11DeviceContext> D3DDeviceContext;


	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &FeatureLevel, 1, D3D11_SDK_VERSION, &D3DDevice, nullptr, &D3DDeviceContext);
	if (FAILED(hr))
	{
		ClearAll();
		FHololensResearchModeUtility::PrintComError(hr, TEXT("D3D11CreateDevice"));
		return;
	}

	hr = D3DDevice->QueryInterface(IID_PPV_ARGS(&D3DDevice1));
	if (FAILED(hr))
	{
		ClearAll();
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Device::QueryInterface to ID3D11Device1"));
		return;
	}

	hr = D3DDeviceContext->QueryInterface(IID_PPV_ARGS(&D3DDeviceContext1));
	if (FAILED(hr))
	{
		ClearAll();
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11DeviceContext::QueryInterface to ID3D11DeviceContext1"));
		return;
	}
}

bool FVideoTextureGenerator::IsTextureNeedsToBeInitilized(uint32 InWidth, uint32 InHeight) const
{
	return !(InWidth == Width && InHeight == Height);
}


bool FVideoTextureGenerator::InitTexture(uint32 InWidth, uint32 InHeight)
{
	if (!D3DDevice1 || !D3DDeviceContext1)
	{
		return false;
	}

	ClearTexture();

	Width = InWidth;
	Height = InHeight;

	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	TextureDesc.MipLevels = 0;
	TextureDesc.ArraySize = 1;
	TextureDesc.BindFlags = 0;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;


	HRESULT hr = D3DDevice1->CreateTexture2D(&TextureDesc, nullptr, &StagingTexture);

	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Device::CreateTexture2D with CPU access"));
		ClearTexture();
		return false;
	}

	TextureDesc = {};
	TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.MipLevels = 0;
	TextureDesc.ArraySize = 1;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = D3DDevice1->CreateTexture2D(&TextureDesc, nullptr, &SharedTexture);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Device::CreateTexture2D with sharing"));
		ClearTexture();
		return false;
	}

	TComPtr<IDXGIResource1> DXGIResource;

	hr = SharedTexture->QueryInterface(__uuidof(IDXGIResource1), (void**)&DXGIResource);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Texture2D::QueryInterface to IDXGIResource1"));
		ClearTexture();
		return false;
	}

	hr = DXGIResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ, nullptr, &hSharedTexture);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("IDXGIResource1::CreateSharedHandle"));
		ClearTexture();
		return false;
	}

	return true;
}

void FVideoTextureGenerator::ClearAll()
{
	ClearTexture();
	D3DDevice1 = nullptr;
	D3DDeviceContext1 = nullptr;
}

void FVideoTextureGenerator::ClearTexture()
{
	Height = Width = 0;
	StagingTexture = nullptr;
	SharedTexture = nullptr;
	hSharedTexture = nullptr;
}


void* FVideoTextureGenerator::MapWriteCPUTexture()
{
	if (!D3DDevice1 || !D3DDeviceContext1 || !StagingTexture)
	{
		return nullptr;
	}

	HRESULT hr = D3DDeviceContext1->Map(StagingTexture, 0, D3D11_MAP_WRITE, 0, &MappedSubresource);
	if (FAILED(hr))
	{
		FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11DeviceContext::Map"));
		return nullptr;
	}

	return MappedSubresource.pData;
}

void FVideoTextureGenerator::UnmapCPUTextureAndCopyOnDevice()
{
	if (!D3DDevice1 || !D3DDeviceContext1 || !StagingTexture || !SharedTexture)
	{
		return;
	}

	D3DDeviceContext1->Unmap(StagingTexture, 0);

	FMemory::Memset(MappedSubresource, 0);

	D3DDeviceContext1->CopyResource(SharedTexture, StagingTexture);
	D3DDeviceContext1->Flush();
}


#endif
