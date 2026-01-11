#pragma once
#include "Module.h"

class ModuleShaderDescriptors : public Module
{
	friend class ShaderTableDesc;
public:
	bool init() override;
	ID3D12DescriptorHeap* getHeap() { return heap.Get(); }

	UINT getHandle() { return handle - 1; };

	void createTextureSRV(ID3D12Resource* resource, UINT8 slot);
	UINT createNullTexture2DSRV();

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT slot) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuStart, slot, descriptorSize); };
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT slot) const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuStart, slot, descriptorSize); };

	void reset() { handle = 0; };

private:
	enum { NUM_DESCRIPTORS = 4096};
	ComPtr<ID3D12DescriptorHeap> heap;
	UINT descriptorSize = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = { 0 };
	UINT handle = 0;
};

