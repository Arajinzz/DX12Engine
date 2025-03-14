#pragma once

#include "Core/DirectXApplication.h"

#include <chrono>

namespace Core
{
  class DirectXApplication;

  class WindowsApplication
  {
  public:
    static int Run(DirectXApplication* pApp, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }
    static bool Resizable() { return m_shouldResize; }
    
    static double deltaTime;

  protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    static HWND m_hwnd;
    static std::chrono::steady_clock::time_point m_startTime;
    static unsigned m_frameCount;
    static bool m_shouldResize;
  };
}

