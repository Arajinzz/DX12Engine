#include "stdafx.h"
#include "WindowsApplication.h"

#include <fstream>

HWND Core::WindowsApplication::m_hwnd = nullptr;
std::chrono::steady_clock::time_point Core::WindowsApplication::m_startTime;
unsigned Core::WindowsApplication::m_frameCount;
double Core::WindowsApplication::deltaTime;

namespace Core
{
  int WindowsApplication::Run(DirectXApplication* pApp, HINSTANCE hInstance, int nCmdShow)
  {
    m_startTime = std::chrono::steady_clock::now();
    m_frameCount = 0;
    deltaTime = 1 / 60;

    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pApp->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"ApplicationClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pApp->GetWidth()), static_cast<LONG>(pApp->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
      windowClass.lpszClassName,
      pApp->GetTitle(),
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      windowRect.right - windowRect.left,
      windowRect.bottom - windowRect.top,
      nullptr,        // We have no parent window.
      nullptr,        // We aren't using menus.
      hInstance,
      pApp);

    // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
    pApp->OnInit();

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
      // Process any messages in the queue.
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    pApp->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
  }

  LRESULT WindowsApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    DirectXApplication* pApp = reinterpret_cast<DirectXApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
      // Save the DXSample* passed in to CreateWindow.
      LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
      if (pApp)
      {
        pApp->OnKeyDown(static_cast<UINT8>(wParam));
      }
      return 0;

    case WM_KEYUP:
      if (pApp)
      {
        pApp->OnKeyUp(static_cast<UINT8>(wParam));
      }
      return 0;

    case WM_PAINT:
    {
      if (pApp)
      {
        pApp->OnUpdate();
        pApp->OnRender();
      }

      // update fps
      auto currentTime = std::chrono::steady_clock::now();
      double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - m_startTime).count();
      m_frameCount++;

      if (elapsedTime >= 1.0)
      {
        double fps = m_frameCount / elapsedTime;
        m_frameCount = 0;
        m_startTime = currentTime;
        deltaTime = 1 / fps;

        if (pApp)
        {
          std::wstringstream ss;
          ss << std::wstring(pApp->GetTitle()) << L" - " << fps;
          SetWindowText(m_hwnd, ss.str().c_str());
        }
      }
      return 0;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
}
