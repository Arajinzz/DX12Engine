#pragma once

#include "Core/DirectXApplication.h"

namespace Core
{
  class DirectXApplication;

  class WindowsApplication
  {
  public:
    static int Run(DirectXApplication* pApp, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

  protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    static HWND m_hwnd;
  };
}

