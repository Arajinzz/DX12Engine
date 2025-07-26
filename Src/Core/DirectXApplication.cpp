#include "stdafx.h"
#include "DirectXApplication.h"

namespace Core
{
  DirectXApplication::DirectXApplication(UINT width, UINT height, std::wstring name)
    : m_width(width)
    , m_height(height)
    , m_title(name)
    , m_useWarpDevice(false)
  {
    WCHAR assetsPath[512];
    Utilities::GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;

    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
  }

  DirectXApplication::~DirectXApplication()
  {
  }

  // Helper function for parsing any supplied command line args.
  _Use_decl_annotations_
  void DirectXApplication::ParseCommandLineArgs(WCHAR* argv[], int argc)
  {
    for (int i = 1; i < argc; ++i)
    {
      if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
        _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
      {
        m_useWarpDevice = true;
        m_title = m_title + L" (WARP)";
      }
    }
  }

  std::wstring DirectXApplication::GetAssetFullPath(LPCWSTR assetName)
  {
    return m_assetsPath + assetName;
  }

  void DirectXApplication::SetCustomWindowText(LPCWSTR text)
  {
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(WindowsApplication::GetHwnd(), windowText.c_str());
  }
}
