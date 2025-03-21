#include "stdafx.h"
#include "WindowsApplication.h"

#include <fstream>
#include <algorithm>

HWND Core::WindowsApplication::m_hwnd = nullptr;
std::chrono::steady_clock::time_point Core::WindowsApplication::m_startTime;
std::chrono::steady_clock::time_point Core::WindowsApplication::m_lastTime;
unsigned Core::WindowsApplication::m_frameCount;
double Core::WindowsApplication::deltaTime;
bool Core::WindowsApplication::m_shouldResize;
MousePosition Core::WindowsApplication::m_lastMousePos;
float Core::WindowsApplication::m_yaw;
float Core::WindowsApplication::m_pitch;
const float Core::WindowsApplication::sensitivity = 0.00001f;

namespace Core
{
  int WindowsApplication::Run(DirectXApplication* pApp, HINSTANCE hInstance, int nCmdShow)
  {
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_lastMousePos = { 0.0f, 0.0f };
    m_shouldResize = false;

    m_startTime = std::chrono::steady_clock::now();
    m_lastTime = std::chrono::steady_clock::now();
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

    case WM_SIZE:
      if (pApp)
      {
        RECT rect;
        GetClientRect(hWnd, &rect);
        auto width = rect.right - rect.left;
        auto height = rect.bottom - rect.top;
        pApp->OnResize(width, height);
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

    case WM_LBUTTONDOWN:
      if (pApp)
      {
        auto xPos = LOWORD(lParam);
        auto yPos = HIWORD(lParam);

        m_lastMousePos = { static_cast<float>(xPos), static_cast<float>(yPos) };

        SetCapture(hWnd);
      }
      return 0;
    
    case WM_LBUTTONUP:
      if (pApp)
      {
        ReleaseCapture();
      }
      return 0;

    case WM_MOUSEMOVE:
      if (pApp && (wParam & MK_LBUTTON)) // only when LBUTTON is held
      {
        auto xPos = LOWORD(lParam);
        auto yPos = HIWORD(lParam);

        auto deltaX = xPos - m_lastMousePos.xPos;
        auto deltaY = yPos - m_lastMousePos.yPos;
        
        m_yaw += deltaX * sensitivity;
        m_pitch += deltaY * sensitivity;

        m_yaw = min(m_yaw, 89.0f);
        m_yaw = max(m_yaw, -89.0f);
        m_pitch = min(m_pitch, 89.0f);
        m_pitch = max(m_pitch, -89.0f);

        pApp->OnMouseMove(m_yaw, m_pitch);
        
        m_lastMousePos = { static_cast<float>(xPos), static_cast<float>(yPos) };
      }
      return 0;

    case WM_PAINT:
    {
      if (pApp)
      {
        pApp->OnUpdate();
        pApp->OnRender();
      }

      // enable resizing
      m_shouldResize = true;

      // update fps
      auto currentTime = std::chrono::steady_clock::now();
      double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - m_startTime).count();
      m_frameCount++;

      // calculate delta time
      deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - m_lastTime).count();
      m_lastTime = currentTime;

      if (elapsedTime >= 1.0)
      {
        double fps = m_frameCount / elapsedTime;
        m_frameCount = 0;
        m_startTime = currentTime;

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
