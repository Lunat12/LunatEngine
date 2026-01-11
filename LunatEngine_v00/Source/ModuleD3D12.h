#pragma once
#include "Module.h"
#include "Globals.h"
#include "dxgi1_6.h"
#include "ModuleResources.h"

class ModuleD3D12 : public Module
{
public:
    ModuleD3D12(HWND hWnd);
    //Override module functions
    bool init() override;
    void preRender() override;
    void render() override;
    void postRender() override;
    void flush();
    void resize();
    void executeCommandList();
    UINT signalDrawQueue();
    

    ID3D12Device5* getDevice() { return device.Get(); };
    ID3D12GraphicsCommandList* getCommandList() { return commandList.Get(); };
    ID3D12CommandAllocator* getCommandAllocator() { return commandAllocators[currenBackBufferIndex].Get(); };
    ID3D12Resource* getBackBuffer() { return backBuffers[currenBackBufferIndex].Get(); };
    D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilDescriptor();
    ID3D12CommandQueue* getCommandQueue() { return commandQueue.Get(); };
    unsigned getWindowWidth() { return windowWidth; }
    unsigned getWindowHeight() { return windowHeight; }
    HWND getWindowHandler() { return windowHandler; }


private:

    HWND windowHandler;

    ComPtr<IDXGIAdapter> adapter;
    ComPtr<ID3D12Device5> device;
    ComPtr<IDXGISwapChain4> swapChain4;
    ComPtr<ID3D12CommandAllocator> commandAllocators[2];
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<ID3D12DescriptorHeap> heap;
    ComPtr<ID3D12DescriptorHeap> stencilheap;
    ComPtr<ID3D12Resource> backBuffers[2];
    ComPtr<ID3D12Fence1> fence;
    HANDLE drawEvent = NULL;
    unsigned currenBackBufferIndex = 0;
    unsigned drawFenceValues[2] = { 0,0 };
    unsigned lastCompletedFrame = 0;
    unsigned frameValues[2] = { 0,0 };
    unsigned frameIndex = 0;
    bool allowTearing = false;
    unsigned drawFenceCounter = 0;
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    unsigned windowHeight = 0;
    unsigned windowWidth = 0;
    CD3DX12_RESOURCE_BARRIER barrierRenderTarget;
    CD3DX12_RESOURCE_BARRIER barrierPresent; 
    ComPtr<ID3D12Resource>depthStencilBuffer;

    void getWindowSize(unsigned &height, unsigned &width);
    bool createRenderTargets();
    bool createDepthStencil();

    ModuleResources* moduleResources;
};

 