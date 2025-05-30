#pragma once

#include "Core/WindowsApplication.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  class DirectXApplication
  {
  public:
    DirectXApplication(UINT width, UINT height, std::wstring name);
    virtual ~DirectXApplication();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;
    virtual void OnResize(unsigned width, unsigned heigt) = 0;

    // Samples override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/) {}
    virtual void OnKeyUp(UINT8 /*key*/) {}
    virtual void OnMouseMove(float /* dx */, float /* dy */) {}

    // Accessors.
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

  protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);

    void SetCustomWindowText(LPCWSTR text);

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

  private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;
  };
}

