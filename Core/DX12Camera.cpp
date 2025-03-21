#include "stdafx.h"
#include "DX12Camera.h"

#include "Core/WindowsApplication.h"

namespace Core
{
  DX12Camera::DX12Camera(float fov, float nearPlane, float farPlane)
    : m_fov(fov)
    , m_near(nearPlane)
    , m_far(farPlane)
  {
    m_view = XMMatrixTranspose(XMMatrixLookAtLH(
      XMVectorSet(0.0, 0.0, -10.0, 0.0), // camera position
      XMVectorSet(0.0, 0.0, 0.0, 0.0), // lookat position
      XMVectorSet(0.0, 1.0, 0.0, 0.0) // up vector
    ));

    m_projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, 16/9, m_near, m_far));
  }

  DX12Camera::~DX12Camera()
  {
  }

  void DX12Camera::Translate(XMFLOAT3 translation)
  {
  }

  void DX12Camera::RotateX(float angle)
  {
  }

  void DX12Camera::RotateY(float angle)
  {
  }

  void DX12Camera::RotateZ(float angle)
  {
  }

  void DX12Camera::Update()
  {
    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    auto aspectRatio = static_cast<double>(width) / height;
    m_projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_near, m_far));
  }
}