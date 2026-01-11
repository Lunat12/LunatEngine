#include "Globals.h"
#include "Exercise2.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleD3D12.h"
#include "ReadData.h"

bool Exercise2::init()
{
    bool ok = createVertexBuffer();
    ok = ok && createRootSignature();
    ok = ok && createPSO();

    return ok;
}

void Exercise2::render()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
    commandList->Reset(d3d12->getCommandAllocator(), pso.Get());

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    LONG width = (LONG)d3d12->getWindowWidth();
    LONG height = (LONG)d3d12->getWindowHeight();
    D3D12_VIEWPORT viewport{ 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
    D3D12_RECT scissor{ 0, 0, width, height };

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    commandList->ClearRenderTargetView(rtv, black, 0, nullptr);
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
    
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList };
    d3d12->getCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);
}

bool Exercise2::createVertexBuffer()
{
    struct Vertex
    {
        float x, y, z;
    };

    Vertex vertex[3] =
    {
        {-1.0f, -1.0f, 0.0f }, // 0
        { 0.0f,  1.0f, 0.0f }, // 1
        { 1.0f, -1.0f, 0.0f }  // 2
    };

    vertexBuffer = app->getResources()->CreateDefaultBuffer(vertex, sizeof(vertex));

    //create vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(vertex);
    vertexBufferView.StrideInBytes = sizeof(Vertex);

    return true;
}

bool Exercise2::createRootSignature()
{
    //create root signature
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob;
    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr))) return false;
    if (FAILED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) return false;

    return true;

}

bool Exercise2::createPSO()
{
    //create input element
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

    //Assign Vertex and Pixel Shader to the PSO
    auto dataVS = DX::ReadData(L"VertexShader.cso");
    auto dataPS = DX::ReadData(L"PixelShader.cso");

    //create pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
    pipelineStateDesc.pRootSignature = rootSignature.Get();
    pipelineStateDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
    pipelineStateDesc.VS = { dataVS.data(), dataVS.size() };
    pipelineStateDesc.PS = { dataPS.data(), dataPS.size() };
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.SampleDesc = { 1, 0 };
    pipelineStateDesc.SampleMask = 0xffffffff;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    return SUCCEEDED(app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pso)));
}
