#include "stdafx.h"
#include "DX12Camera.h"

#include "Core/WindowsApplication.h"

#include <algorithm>

namespace
{
  float wrap_angle(float angle)
  {
    const float pi = 3.14159265f;
    const float twoPi = 2.0f * 3.14159265f;
    const float mod = std::fmod(angle, twoPi);
    if (mod > pi)
    {
      return mod - twoPi;
    } else if (mod < -pi)
    {
      return mod + twoPi;
    }
    return mod;
  }
}

namespace Core
{
  DX12Camera::DX12Camera(float fov, float nearPlane, float farPlane)
    : m_fov(fov)
    , m_near(nearPlane)
    , m_far(farPlane)
    , m_cameraPosition(0.0f, 0.0f, -10.0f)
    , m_lookAt(0.0f, 0.0f, 1.0f)
    , m_up(0.0f, 1.0f, 0.0)
    , m_pitch(0.0f)
    , m_yaw(0.0f)
    , m_translation(0.0f, 0.0f, 0.0f)
  {
  }

  DX12Camera::~DX12Camera()
  {
  }

  void DX12Camera::Translate(float x, float z)
  {
    XMFLOAT3 translation = {x, 0.0, z};
    auto rotatedTranslation = XMVector3Transform(XMLoadFloat3(&translation), XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f));
    XMStoreFloat3(&translation, rotatedTranslation);
    m_translation.x = translation.x;
    m_translation.y = translation.y;
    m_translation.z = translation.z;
  }

  void DX12Camera::Rotate(float dx, float dy)
  {
    const XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };
    const float pi = 3.14159265f;
    auto sensitivity = 0.005f;
    m_yaw = wrap_angle(m_yaw + dx * sensitivity);
    m_pitch = std::clamp(m_pitch + dy * sensitivity, 0.995f * -pi / 2.0f, 0.995f * pi / 2.0f);
    XMVECTOR forwardVec = XMLoadFloat3(&forward);

    // apply the camera rotations to a base vector
    const auto lookVector = XMVector3Transform(forwardVec,
      XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f)
    );
    XMStoreFloat3(&m_lookAt, lookVector);
  }

  void DX12Camera::Update()
  {
    float speed = 100.0f;
    XMFLOAT3 desiredPosition;
    desiredPosition.x = m_cameraPosition.x + m_translation.x * speed;
    desiredPosition.y = m_cameraPosition.y + m_translation.y * speed;
    desiredPosition.z = m_cameraPosition.z + m_translation.z * speed;
    float smoothFactor = 0.1f;
    XMVECTOR currentPos = XMLoadFloat3(&m_cameraPosition);
    XMVECTOR targetPos = XMLoadFloat3(&desiredPosition);
    XMVECTOR newPos = XMVectorLerp(currentPos, targetPos, 0.1f);
    // smoothed position
    XMStoreFloat3(&m_cameraPosition, newPos);

    XMFLOAT3 cameraTarget;
    cameraTarget.x = m_cameraPosition.x + m_lookAt.x;
    cameraTarget.y = m_cameraPosition.y + m_lookAt.y;
    cameraTarget.z = m_cameraPosition.z + m_lookAt.z;

    m_view = XMMatrixTranspose(XMMatrixLookAtLH(
      XMVectorSet(m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, 0.0), // camera position
      XMVectorSet(cameraTarget.x, cameraTarget.y, cameraTarget.z, 0.0), // lookat position
      XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0) // up vector
    ));

    // reset translation
    m_translation = { 0.0f, 0.0f, 0.0f };

    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    auto aspectRatio = static_cast<double>(width) / height;
    m_projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, static_cast<float>(aspectRatio), m_near, m_far));
  }
}