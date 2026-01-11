#include "Globals.h"
#include "ModuleD3D12.h"
#include "Application.h"

ModuleD3D12::ModuleD3D12(HWND hWnd)
{
	windowHandler = hWnd;
}

bool ModuleD3D12::init()
{
	//Debug
	#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugInterface;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
		debugInterface->EnableDebugLayer();
	#endif


	ComPtr<IDXGIFactory6> factory;
	#if defined(_DEBUG)
		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
	#else
		CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	#endif

	getWindowSize(windowHeight, windowWidth);

	//create the device
	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

	#if defined(_DEBUG)
		//info queue
		ComPtr<ID3D12InfoQueue> infoQueue;
		device.As(&infoQueue);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	#endif

	//Command queue
	D3D12_COMMAND_QUEUE_DESC queue = {};
	queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	bool ok = SUCCEEDED(device->CreateCommandQueue(&queue, IID_PPV_ARGS(&commandQueue)));

	//Swap chain
	ComPtr<IDXGISwapChain1> swapChain1;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 1920;                                    // Width of the back buffer in pixels
	swapChainDesc.Height = 1080;                                   // Height of the back buffer in pixels
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             // 32-bit RGBA format (8 bits per channel)
	                                                               // UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0)      
	swapChainDesc.Stereo = FALSE;                                  // Set to TRUE for stereoscopic 3D rendering (VR/3D Vision)
	swapChainDesc.SampleDesc = { 1, 0 };                           // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   // This buffer will be used as a render target
	swapChainDesc.BufferCount = 2;                                 // Double buffering:
	                                                               // - 1 front buffer (displayed)
                                                                   // - 1 back buffer (rendering)
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;                  // How to scale when window size doesn't match buffer size:
	                                                               // STRETCH = Stretch the image to fit the window
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;      // Modern efficient swap method:
	                                                               // - FLIP: Uses page flipping (no copying)
                                                                   // - DISCARD: Discard previous back buffer contents
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;         // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
	swapChainDesc.Flags = 0;                                       // Additional swap chain options : 0 = No special flags
		                                                           // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Allow full-screen mode switches
	                                                               // DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING: Allow tearing in windowed mode (VSync off)

	//swap chain
	ok = ok && SUCCEEDED(factory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandler, &swapChainDesc, nullptr, nullptr, &swapChain1));
	ok = ok && SUCCEEDED(swapChain1.As(&swapChain4));

	ok = ok && createRenderTargets();
	ok = ok && createDepthStencil();

	//Command allocator
	for (size_t i = 0; i < 2; i++)
	{
		ok = ok && SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])));
	}

	//Command list
	ok = ok && SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)));
	commandList->Close();

	//Fence
	ok = ok && SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	if (ok) {
		drawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ok = ok && drawEvent != NULL;
	}

	//Breakpoint
	_ASSERT_EXPR(ok, L"Something happened in ModuleD3d12::init");

	return ok;
}

void ModuleD3D12::preRender()
{

	currenBackBufferIndex = swapChain4->GetCurrentBackBufferIndex();
	if (drawFenceValues[currenBackBufferIndex] != 0) {
		fence->SetEventOnCompletion(drawFenceValues[currenBackBufferIndex], drawEvent);
		WaitForSingleObject(drawEvent, INFINITE);

		lastCompletedFrame = std::max(frameValues[currenBackBufferIndex], lastCompletedFrame);
	}

	frameIndex++;
	frameValues[currenBackBufferIndex] = frameIndex;
	commandAllocators[currenBackBufferIndex]->Reset();

}

void ModuleD3D12::render()
{
}

void ModuleD3D12::postRender()
{
	swapChain4->Present(allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 1, 0);
	signalDrawQueue();
	
}

void ModuleD3D12::flush()
{
	commandQueue->Signal(fence.Get(), ++drawFenceCounter);
	fence->SetEventOnCompletion(drawFenceCounter, drawEvent);
	WaitForSingleObject(drawEvent, INFINITE);

}

void ModuleD3D12::resize()
{
	unsigned height = 0, width = 0;

	getWindowSize(height, width);

	if (width != windowWidth || height != windowHeight)
	{
		windowWidth = width;
		windowHeight = height;

		flush();

		for (unsigned i = 0; i < 2; i++)
		{
			backBuffers[i].Reset();
			drawFenceValues[i] = 0;
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChain4->GetDesc(&swapChainDesc);
		swapChain4->ResizeBuffers(2, windowWidth, windowHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

		if (windowWidth > 0 && windowHeight > 0)
		{
			createRenderTargets();
			createDepthStencil();
		}
	}
}

void ModuleD3D12::executeCommandList()
{
	commandList->ResourceBarrier(1, &barrierPresent);

	if (SUCCEEDED(commandList->Close())) {
		ID3D12CommandList* commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);
	}
}

UINT ModuleD3D12::signalDrawQueue()
{
	drawFenceValues[currenBackBufferIndex] = ++drawFenceCounter;
	commandQueue->Signal(fence.Get(), drawFenceValues[currenBackBufferIndex]);
	return drawFenceCounter;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleD3D12::getRenderTargetDescriptor()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), currenBackBufferIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleD3D12::getDepthStencilDescriptor()
{
	return stencilheap->GetCPUDescriptorHandleForHeapStart();
}

void ModuleD3D12::getWindowSize(unsigned &height, unsigned &width)
{
	RECT clientRect = {};
	GetClientRect(windowHandler, &clientRect);

	width = unsigned(clientRect.right - clientRect.left);
	height = unsigned(clientRect.bottom - clientRect.top);
}



bool ModuleD3D12::createRenderTargets()
{
	//Heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;

	bool ok = SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	//Resources
	if (ok) {
		UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(heap->GetCPUDescriptorHandleForHeapStart());

		for (size_t i = 0; i < 2; i++)
		{
			ok = ok && SUCCEEDED(swapChain4->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i])));

			if (ok) {
				backBuffers[i]->SetName(L"backbuffer");

				device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvHandle);
			}

			rtvHandle.ptr += rtvDescriptorSize;
		}

	}

	return ok;
}

bool ModuleD3D12::createDepthStencil()
{
	D3D12_CLEAR_VALUE clearValue= {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
	
	bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthStencilBuffer)));
	
	if (ok) 
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ok = SUCCEEDED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&stencilheap)));
	}

	if (ok) 
	{
		device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, stencilheap->GetCPUDescriptorHandleForHeapStart());
	}

	return ok;
}

