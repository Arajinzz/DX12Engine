#include "stdafx.h"
#include "DX12Camera.h"

#include "Core/WindowsApplication.h"

namespace Core
{
  DX12Camera::DX12Camera(float fov, float nearPlane, float farPlane)
    : m_fov(fov)
    , m_near(nearPlane)
    , m_far(farPlane)
    , m_cameraPosition(0.0f, 0.0f, -10.0f)
    , m_lookAt(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0)
  {
  }

  DX12Camera::~DX12Camera()
  {
  }

  void DX12Camera::Translate(float x, float z)
  {
    m_cameraPosition.x += x;
    m_cameraPosition.z += z;

    // change lookat
    m_lookAt.x += x;
    m_lookAt.z += z;
  }

  void DX12Camera::Rotate(float yaw, float pitch)
  {
    XMMATRIX rotationYaw = XMMatrixRotationY(yaw);
    XMMATRIX rotationPitch = XMMatrixRotationX(pitch);
    XMMATRIX rotationMatrix = rotationPitch * rotationYaw;
    m_view = rotationMatrix * m_view;
  }

  void DX12Camera::Update()
  {
    m_view = XMMatrixTranspose(XMMatrixLookAtLH(
      XMVectorSet(m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, 0.0), // camera position
      XMVectorSet(m_lookAt.x, m_lookAt.y, m_lookAt.z, 0.0), // lookat position
      XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0) // up vector
    ));

    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    auto aspectRatio = static_cast<double>(width) / height;
    m_projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_near, m_far));
  }
}