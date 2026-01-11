#include "Globals.h"
#include "ModuleShaderDescriptors.h"
#include "Application.h"
#include "ModuleD3D12.h"

bool ModuleShaderDescriptors::init()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12Device2* device = d3d12->getDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = NUM_DESCRIPTORS;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));

    heap->SetName(L"Shader Descriptors Heap");

    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    gpuStart = heap->GetGPUDescriptorHandleForHeapStart();
    cpuStart = heap->GetCPUDescriptorHandleForHeapStart();

    return true;
}


void ModuleShaderDescriptors::createTextureSRV(ID3D12Resource* resource, UINT8 slot)
{
   
    if (resource)
    {
        ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();
        app->getD3D12()->getDevice()->CreateShaderResourceView(resource, nullptr, descriptors->GetCPUHandle(slot));
    }

    handle++;
}

UINT ModuleShaderDescriptors::createNullTexture2DSRV()
{
    UINT index = handle++;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Standard format
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    ID3D12Device5* device = app->getD3D12()->getDevice();
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(cpuStart, index, descriptorSize);
    device->CreateShaderResourceView(nullptr, &srvDesc, handle);
    return index;
}
