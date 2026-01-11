#pragma once
#include "Module.h"
#include "dxgi1_6.h"
#include <filesystem>
#include "DirectXTex.h"

class ModuleD3D12;

class ModuleResources : public Module
{
public:
	bool init() override;
	void render() override;
	ComPtr<ID3D12Resource> CreateUploadBuffer(void* data, size_t bufferSize);
	ComPtr<ID3D12Resource> CreateUploadHeap(size_t bufferSize);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* data, size_t bufferSize);
	void MapBuffer(const void* data, size_t dataSize, ComPtr<ID3D12Resource> resource);
	ComPtr<ID3D12Resource> CreateTextureFromFile(const std::filesystem::path& path);
	ComPtr<ID3D12Resource> CreateTextureFromImage(const ScratchImage& image, const char* name);

private:
	ModuleD3D12* d3d12;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
};

