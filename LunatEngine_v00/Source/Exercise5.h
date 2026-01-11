#pragma once
#include "Module.h"
#include "DebugDrawPass.h"
#include <ImGuiPass.h>
#include "ModuleSamplers.h"
#include "BasicModel.h"

class Exercise5 : public Module
{
public:
	virtual bool init() override;
	virtual void render() override;
	virtual void preRender() override;

private:
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12Resource> bufferUploadHeap;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
	std::unique_ptr<DebugDrawPass> debugDraw;
	ComPtr<ID3D12Resource> depthStencilBuffer;
	Matrix proj;
	Matrix view;
	ComPtr<ID3D12Resource> textureDog;
	Matrix mvp;
	std::unique_ptr<BasicModel> model;
	std::vector<ComPtr<ID3D12Resource>> materialBuffers;

	bool showGrid = true;
	bool showAxis = true;
	int sampler = int(ModuleSamplers::LINEAR_WRAP); //controls in wich filter we are.

	std::unique_ptr<ImGuiPass> imguiPass;

	bool createVertexBuffer();
	bool createRootSignature();
	bool createPSO();
	bool loadModel();
};



