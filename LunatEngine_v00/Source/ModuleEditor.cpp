#include "Globals.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleEditor.h"


ModuleEditor::ModuleEditor(HWND _hwnd)
{
	hwnd = _hwnd;
}

bool ModuleEditor::init()
{
	d3d12 = app->getD3D12();
	ID3D12Device5* device = d3d12->getDevice();

	//Imguipass

	imGuiPass = new ImGuiPass(device, hwnd, cpuHandle, gpuHandle);

	return true;
}

void ModuleEditor::preRender()
{
	imGuiPass->startFrame();

	ImGui::ShowDemoWindow();
}

void ModuleEditor::postRender()
{
	
}

void ModuleEditor::render()
{
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
	imGuiPass->record(commandList);
	d3d12->executeCommandList();
}

bool ModuleEditor::cleanUp()
{
	delete imGuiPass;
	return true;
}

void ModuleEditor::MySaveFunction()
{

}
