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


void ModuleShaderDescriptors::createTextureSRV(ID3D12Resource* resource)
{
   
    if (resource)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE slot = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuStart, handle, descriptorSize);
        app->getD3D12()->getDevice()->CreateShaderResourceView(resource, nullptr, slot);
    }

    handle++;
}
