#pragma once
#include "Module.h"
#include "DebugDrawPass.h"

class Exercise3 : public Module
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

	bool createVertexBuffer();
	bool createRootSignature();
	bool createPSO();
};

