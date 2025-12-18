#include "Globals.h"
#include "ModuleResources.h"
#include "ModuleD3D12.h"
#include "Application.h"
#include "DirectXTex.h"




bool ModuleResources::init()
{
	d3d12 = app->getD3D12();
	ID3D12Device5* device = d3d12->getDevice();

	bool ok = SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ok = ok && SUCCEEDED(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)));
	ok = ok && SUCCEEDED(commandList->Reset(commandAllocator.Get(), nullptr));

	return ok;
}

void ModuleResources::render()
{

}

ComPtr<ID3D12Resource> ModuleResources::CreateUploadBuffer(void* data, size_t bufferSize)
{
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	ID3D12Device5* device = d3d12->getDevice();
	ComPtr<ID3D12Resource> buffer;
	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	MapBuffer(data, bufferSize, buffer);

	return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::CreateUploadHeap(size_t bufferSize)
{
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	ID3D12Device5* device = d3d12->getDevice();
	ComPtr<ID3D12Resource> buffer;
	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::CreateDefaultBuffer(void* data, size_t bufferSize)
{
	ID3D12Device5* device = d3d12->getDevice();
	ID3D12CommandQueue* commandQueue = d3d12->getCommandQueue();

	ComPtr<ID3D12Resource> buffer;
	ComPtr<ID3D12Resource> stagingBuffer;

	// --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer));
	
	// --- CREATE THE STAGING BUFFER (UPLOAD HEAP) ---
	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));
	// --- CPU: FILL STAGING BUFFER ---
	MapBuffer(data, bufferSize, stagingBuffer);
	// --- GPU: COPY DATA ---
	//UDar su propia command list y command allocator. separar ejercicios.
	commandList->CopyResource(buffer.Get(), stagingBuffer.Get());

	commandList->Close();
	ID3D12CommandList* commandLists[] = { commandList.Get()};
	commandQueue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

	//flush command queue
	d3d12->flush();

	commandAllocator->Reset();

	commandList->Reset(commandAllocator.Get(), nullptr);

	return buffer;
}

void ModuleResources::MapBuffer(void* data, size_t dataSize, ComPtr<ID3D12Resource> resource)
{
	// Map the buffer: get a CPU pointer to its memory
	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
	resource->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	// Copy our application data into the GPU buffer
	memcpy(pData, data, dataSize);
	// Unmap the buffer (invalidate the pointer)
	resource->Unmap(0, nullptr);
}

ComPtr<ID3D12Resource> ModuleResources::CreateTextureFromFile(const std::filesystem::path& path)
{
	const wchar_t* filename = path.c_str();

	DirectX::ScratchImage image;
	bool ok = SUCCEEDED(LoadFromDDSFile(filename, DDS_FLAGS_NONE, nullptr, image));
	ok = ok || SUCCEEDED(LoadFromTGAFile(filename, nullptr, image));
	ok = ok || SUCCEEDED(LoadFromWICFile(filename, WIC_FLAGS_NONE, nullptr, image));
	
	if (ok) return CreateTextureFromImage(image, path.string().c_str()); 

	return nullptr;

}

ComPtr<ID3D12Resource> ModuleResources::CreateTextureFromImage(const ScratchImage& image, const char* name)
{
	ID3D12Device5* device = d3d12->getDevice();
	ComPtr<ID3D12Resource> texture;
	const TexMetadata& metadata = image.GetMetadata();

	_ASSERTE(metadata.dimension == TEX_DIMENSION_TEXTURE2D);

	if (metadata.dimension == TEX_DIMENSION_TEXTURE2D) 
	{
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, UINT64(metadata.width), UINT(metadata.height), UINT16(metadata.arraySize), UINT16(metadata.mipLevels));
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture)));
		ComPtr<ID3D12Resource> upload;
		if (ok) 
		{
			UINT64 size = GetRequiredIntermediateSize(texture.Get(), 0, image.GetImageCount());

			CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);


			device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload));
			
			ok = upload != nullptr;
		}

		if (ok)
		{
			std::vector<D3D12_SUBRESOURCE_DATA> subdata;
			subdata.reserve(image.GetImageCount());
			for (size_t item = 0; item < metadata.arraySize; item++)
			{
				for (size_t level = 0; level < metadata.mipLevels; level++)
				{
					const DirectX::Image* subImg = image.GetImage(level, item, 0);
					D3D12_SUBRESOURCE_DATA data = { subImg->pixels, (LONG_PTR)subImg->rowPitch, (LONG_PTR)subImg->slicePitch };
					subdata.push_back(data);
				}
			}

			ok = UpdateSubresources(commandList.Get(), texture.Get(), upload.Get(), 0, 0, UINT(image.GetImageCount()), subdata.data()) != 0;
		}

		if (ok)
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			commandList->ResourceBarrier(1, &barrier);
			commandList->Close();

			ID3D12CommandList* commandLists[] = { commandList.Get() };
			ID3D12CommandQueue* queue = d3d12->getCommandQueue();

			queue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

			d3d12->flush();

			commandAllocator->Reset();
			ok = SUCCEEDED(commandList->Reset(commandAllocator.Get(), nullptr));

			texture->SetName(std::wstring(name, name + strlen(name)).c_str());
			return texture;

		}

	}

	return ComPtr<ID3D12Resource>();
}




