// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "VideoTextureResource.h"

#include "RenderingThread.h"

#if PLATFORM_HOLOLENS

//-------------------------------------------------------------------------------------------------
// D3D11
//-------------------------------------------------------------------------------------------------

#include "D3D11RHIPrivate.h"
#include "D3D11Util.h"


//-------------------------------------------------------------------------------------------------
// D3D12
//-------------------------------------------------------------------------------------------------

#define GetD3D11CubeFace GetD3D12CubeFace
#define VerifyD3D11Result VerifyD3D12Result
#define GetD3D11TextureFromRHITexture GetD3D12TextureFromRHITexture
#define FRingAllocation FRingAllocation_D3D12
#define GetRenderTargetFormat GetRenderTargetFormat_D3D12
#define ED3D11ShaderOffsetBuffer ED3D12ShaderOffsetBuffer
#define FindShaderResourceDXGIFormat FindShaderResourceDXGIFormat_D3D12
#define FindUnorderedAccessDXGIFormat FindUnorderedAccessDXGIFormat_D3D12
#define FindDepthStencilDXGIFormat FindDepthStencilDXGIFormat_D3D12
#define HasStencilBits HasStencilBits_D3D12
#define FVector4VertexDeclaration FVector4VertexDeclaration_D3D12
#define GLOBAL_CONSTANT_BUFFER_INDEX GLOBAL_CONSTANT_BUFFER_INDEX_D3D12
#define MAX_CONSTANT_BUFFER_SLOTS MAX_CONSTANT_BUFFER_SLOTS_D3D12
#define FD3DGPUProfiler FD3D12GPUProfiler
#define FRangeAllocator FRangeAllocator_D3D12

#include "D3D12RHIPrivate.h"
#include "D3D12Util.h"

#undef GetD3D11CubeFace
#undef VerifyD3D11Result
#undef GetD3D11TextureFromRHITexture
#undef FRingAllocation
#undef GetRenderTargetFormat
#undef ED3D11ShaderOffsetBuffer
#undef FindShaderResourceDXGIFormat
#undef FindUnorderedAccessDXGIFormat
#undef FindDepthStencilDXGIFormat
#undef HasStencilBits
#undef FVector4VertexDeclaration
#undef GLOBAL_CONSTANT_BUFFER_INDEX
#undef MAX_CONSTANT_BUFFER_SLOTS
#undef FD3DGPUProfiler
#undef FRangeAllocator


FVideoTextureResource::FVideoTextureResource(UHololensResearchModeVideoTexture* InOwner)
	: Owner(InOwner)
{
}

FVideoTextureResource::~FVideoTextureResource()
{
}

void FVideoTextureResource::InitRHI() 
{
	check(IsInRenderingThread());

	TextureRHI = nullptr;

	FString RHIString = FApp::GetGraphicsRHI();

	bIsDx11 = (RHIString == TEXT("DirectX 11"));
	bIsDx12 = (RHIString == TEXT("DirectX 12"));

	if (!bIsDx12 && !bIsDx11)
	{
		//check for Windows Mixed Reality RHI
		if (GD3D11RHI != nullptr)
		{
			bIsDx11 = true;
		}
	}

	FTexture2DRHIRef TextureToDisplay;
	FSamplerStateInitializerRHI SamplerStateInitializer(SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp);
	SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);

	do 
	{
		if (!hSharedTexture)
		{
			break;
		}

		if (!bIsDx12 && !bIsDx11)
		{
			UE_LOG(LogHLResearch, Error, TEXT("Can't get a right RHI, please use DirectX 11 or 12"));
			break;
		}

		if (bIsDx11)
		{
			FD3D11DynamicRHI* DX11RHI = StaticCast<FD3D11DynamicRHI*>(GDynamicRHI);

			TComPtr<ID3D11Device1> D3D11Device1;
			HRESULT hr = DX11RHI->GetDevice()->QueryInterface(IID_PPV_ARGS(&D3D11Device1));
			if (FAILED(hr))
			{
				FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Device::QueryInterface towards ID3D11Device1"));
				break;
			}

			TComPtr<ID3D11Texture2D> cameraImageTexture;
			hr = D3D11Device1->OpenSharedResource1(hSharedTexture, IID_PPV_ARGS(&cameraImageTexture));
			if (FAILED(hr))
			{
				FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D11Device1::OpenSharedResource1"));
				break;
			}

			TextureToDisplay = DX11RHI->RHICreateTexture2DFromResource(PF_B8G8R8A8, TexCreate_Dynamic | TexCreate_ShaderResource, FClearValueBinding::None, cameraImageTexture);
		}
		else if (bIsDx12)
		{
			FD3D12DynamicRHI* DX12RHI = StaticCast<FD3D12DynamicRHI*>(GDynamicRHI);

			TComPtr<ID3D12Resource> cameraImageTexture;
			HRESULT hr = DX12RHI->GetAdapter().GetD3DDevice()->OpenSharedHandle(hSharedTexture, IID_PPV_ARGS(&cameraImageTexture));
			if (FAILED(hr))
			{
				FHololensResearchModeUtility::PrintComError(hr, TEXT("ID3D12Device::OpenSharedHandle"));
				break;
			}

			TextureToDisplay = DX12RHI->RHICreateTexture2DFromResource(PF_B8G8R8A8, TexCreate_Dynamic | TexCreate_ShaderResource, FClearValueBinding::None, cameraImageTexture);
		}
	} while (false);

	if (TextureToDisplay)
	{
		Size = TextureToDisplay->GetSizeXY();
		TextureRHI = TextureToDisplay;
		bIsTextureCreated = true;
	}
	else
	{
		FRHIResourceCreateInfo CreateInfo;
		Size.X = Size.Y = 1;
		TextureRHI = RHICreateTexture2D(Size.X, Size.Y, PF_B8G8R8A8, 1, 1, TexCreate_ShaderResource, CreateInfo);
		bIsTextureCreated = false;
	}
	TextureRHI->SetName(Owner->GetFName());
	RHIBindDebugLabelName(TextureRHI, *Owner->GetName());
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, TextureRHI);
	if (bIsTextureCreated)
	{
		Owner->GetSensor()->NotifyCameraSensorTextureCreated();
	}
}


void FVideoTextureResource::ReleaseRHI()
{
	if (bIsTextureCreated && TextureRHI != nullptr && Owner && Owner->GetSensor())
	{
		Owner->GetSensor()->NotifyCameraSensorTextureDestroyed();
	}
	TextureRHI = nullptr;
	Size = FIntPoint::ZeroValue;
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
	hSharedTexture = nullptr;
	FTextureResource::ReleaseRHI();
}

uint32 FVideoTextureResource::GetSizeX() const 
{
	return Size.X;
}

uint32 FVideoTextureResource::GetSizeY() const 
{
	return Size.Y;
}

void FVideoTextureResource::Init_RenderThread(FSharedHandle hInSharedTexture)
{
	check(IsInRenderingThread());
	ReleaseRHI();
	hSharedTexture = hInSharedTexture;
	InitRHI();
}

#endif