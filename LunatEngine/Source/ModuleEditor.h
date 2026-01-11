#pragma once
#include "Module.h"
#include "Globals.h"
#include "dxgi1_6.h"
#include "ImGuiPass.h"

class ModuleD3D12;

class ModuleEditor : public Module
{
public:
	ModuleEditor(HWND _hwnd);

	bool init() override;

	void preRender() override;

	void postRender() override;

	void render() override;

	bool cleanUp() override;

private:

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

	HWND hwnd;
	ImGuiPass* imGuiPass;

	ModuleD3D12* d3d12;

	void MySaveFunction();
};

