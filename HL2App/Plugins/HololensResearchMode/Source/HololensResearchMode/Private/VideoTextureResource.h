// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "TextureResource.h"
#include "RHI.h"
#include "SharedHandle.h"

class UHololensResearchModeVideoTexture;

#if PLATFORM_HOLOLENS
class FVideoTextureResource :
	public FTextureResource
{
public:
	FVideoTextureResource(UHololensResearchModeVideoTexture* InOwner);

	virtual ~FVideoTextureResource();

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
	virtual uint32 GetSizeX() const override;
	virtual uint32 GetSizeY() const override;

	void Init_RenderThread(FSharedHandle hInSharedTexture);

private:
	const UHololensResearchModeVideoTexture* Owner;
	FSharedHandle hSharedTexture;
	FIntPoint Size = FIntPoint::ZeroValue;
	bool bIsTextureCreated = false;

	bool bIsDx11;
	bool bIsDx12;

};
#endif

