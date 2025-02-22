#include "stdafx.h"

#include "Core/Application.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  Core::Application app(1280, 720, L"D3D12 Engine");
  return Core::WindowsApplication::Run(&app, hInstance, nCmdShow);
}