#pragma once
#include "Module.h"

class ModuleShaderDescriptors : public Module
{
	friend class ShaderTableDesc;
public:
	bool init() override;
	ID3D12DescriptorHeap* getHeap() { return heap.Get(); }

	inline UINT getHandle() { return handle - 1; };

	void createTextureSRV(ID3D12Resource* resource);

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT slot) { return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuStart, slot, descriptorSize); };
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT slot) { return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuStart, slot, descriptorSize); };

	inline void reset() { handle = 0; };

private:
	enum { NUM_DESCRIPTORS = 4096};
	ComPtr<ID3D12DescriptorHeap> heap;
	UINT descriptorSize = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = { 0 };
	UINT handle = 0;
};

