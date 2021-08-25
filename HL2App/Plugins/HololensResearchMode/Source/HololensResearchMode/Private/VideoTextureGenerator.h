// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "SharedHandle.h"

#if PLATFORM_HOLOLENS

class FVideoTextureGenerator
{
public:

	FVideoTextureGenerator();
	~FVideoTextureGenerator();

	bool InitTexture(uint32 InWidth, uint32 InHeight);
	bool IsTextureNeedsToBeInitilized(uint32 InWidth, uint32 InHeight) const;
	void* MapWriteCPUTexture();
	void UnmapCPUTextureAndCopyOnDevice();
	void ClearAll();
	void ClearTexture();

	uint32 GetWidth() const { return Width; }
	uint32 GetHeight() const { return Height; }
	uint32 GetRowPitch() const { return MappedSubresource.RowPitch; }
	FSharedHandle GetTextureHandle() const { return hSharedTexture; }

private:
	void Init();

	TComPtr<ID3D11Device1> D3DDevice1;
	TComPtr<ID3D11DeviceContext1> D3DDeviceContext1;

	TComPtr<ID3D11Texture2D> StagingTexture;
	TComPtr<ID3D11Texture2D> SharedTexture;
	FSharedHandle hSharedTexture;

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;

	uint32 Width = 0;
	uint32 Height = 0;
};

#endif