#include "Globals.h"
#include "Exercise4.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleD3D12.h"
#include "ReadData.h"
#include "DebugDrawPass.h"
#include "ModuleEditorCamera.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleSamplers.h"
#include "Timer.h"

bool Exercise4::init()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    bool ok = createVertexBuffer();
    ok = ok && createRootSignature();
    ok = ok && createPSO();

    if (ok) 
    {
        ModuleResources* resources = app->getResources();
        ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();

        std::wstring imageUrl = std::wstring(L"Assets/Textures/dog.dds");
        textureDog = resources->CreateTextureFromFile(imageUrl);

        if ((ok = textureDog) == true) 
        {
            descriptors->createTextureSRV(textureDog.Get(),0);
        }
    }

    if (ok)
    {
        debugDraw = std::make_unique<DebugDrawPass>(d3d12->getDevice(), d3d12->getCommandQueue());
        imguiPass = std::make_unique<ImGuiPass>(d3d12->getDevice(), d3d12->getWindowHandler());
    }

    return ok;

}

void Exercise4::render()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
    ModuleShaderDescriptors* shaderDescriptors = app->getShaderDescriptors();
    ModuleSamplers* samplers = app->getSamplers();

    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Texture Viewer Options");
    ImGui::Text("FPS = %g", app->getFPS());
    ImGui::Checkbox("Show grid", &showGrid);
    ImGui::Checkbox("Show axis", &showAxis);
    ImGui::Combo("Sampler", &sampler, "Linear/Wrap\0Point/Wrap\0Linear/Clamp\0Point/Clamp", ModuleSamplers::COUNT);
    ImGui::End();

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    LONG width = (LONG)d3d12->getWindowWidth();
    LONG height = (LONG)d3d12->getWindowHeight();
    D3D12_VIEWPORT viewport{ 0.0, 0.0, float(width), float(height) , 0.0, 1.0 };
    D3D12_RECT scissor{ 0, 0, width, height };

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDepthStencilDescriptor();

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->ClearRenderTargetView(rtv, black, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0, 0, 0, nullptr);
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    ID3D12DescriptorHeap* descriptorHeaps[] = { shaderDescriptors->getHeap(), samplers->getHeap() };
    commandList->SetDescriptorHeaps(2, descriptorHeaps);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);
    commandList->SetGraphicsRootDescriptorTable(1, shaderDescriptors->GetGPUHandle(shaderDescriptors->getHandle()));
    commandList->SetGraphicsRootDescriptorTable(2, samplers->getGPUHandle(ModuleSamplers::Type(sampler)));
    commandList->DrawInstanced(6, 1, 0, 0);
   
    if(showGrid) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray); // Grid plane
    if(showAxis) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f); // XYZ axis
    
    debugDraw->record(commandList, width, height, view, proj);

    imguiPass->record(commandList);

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList };
    d3d12->getCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);
}

void Exercise4::preRender()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ModuleEditorCamera* camera = app->getCamera();
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
    imguiPass->startFrame();
    commandList->Reset(d3d12->getCommandAllocator(), pso.Get());

    LONG width = (LONG)d3d12->getWindowWidth();
    LONG height = (LONG)d3d12->getWindowHeight();

    Matrix model = Matrix::Identity;
    view = camera->GetViewMatrix();
    proj = camera->GetProjectionMatrix(float(width) / float(height));

    mvp = (model * view * proj).Transpose();
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0);


}

bool Exercise4::createVertexBuffer()
{
    struct Vertex
    {
        Vector3 position;
        Vector2 uv;
    };

    Vertex vertex[6] = {
        {  Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
        { Vector3(-1.0f, 1.0f, 0.0f), Vector2(-0.2f, -0.2f) },
        { Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
        { Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
        { Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
        { Vector3(1.0f, -1.0f, 0.0f), Vector2(1.2f, 1.2f) }
    };

    vertexBuffer = app->getResources()->CreateDefaultBuffer(vertex, sizeof(vertex));

    //create vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(vertex);
    vertexBufferView.StrideInBytes = sizeof(Vertex);

    return true;
}

bool Exercise4::createRootSignature()
{
    //create root signature
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
    CD3DX12_DESCRIPTOR_RANGE srvRange, sampRange;

    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSamplers::COUNT, 0);

    rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

    rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob;
    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr))) return false;
    if (FAILED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) return false;

    return true;

}

bool Exercise4::createPSO()
{
    //create input element
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { 
        {"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    //Assign Vertex and Pixel Shader to the PSO
    auto dataVS = DX::ReadData(L"VertexShader.cso");
    auto dataPS = DX::ReadData(L"PixelShader.cso");

    //create pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
    pipelineStateDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
    pipelineStateDesc.pRootSignature = rootSignature.Get();
    pipelineStateDesc.VS = { dataVS.data(), dataVS.size() };
    pipelineStateDesc.PS = { dataPS.data(), dataPS.size() };
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateDesc.SampleDesc = { 1, 0 };
    pipelineStateDesc.SampleMask = 0xffffffff;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.NumRenderTargets = 1;


    return SUCCEEDED(app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pso)));
}

