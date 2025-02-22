#include "stdafx.h"

#include "Core/Application.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  Core::Application app(1280, 720, L"D3D12 Engine");
  return Core::WindowsApplication::Run(&app, hInstance, nCmdShow);
}