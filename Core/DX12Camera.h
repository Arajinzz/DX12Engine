#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Camera
  {
  public:
    DX12Camera(float fov , float nearPlane, float farPlane);
    ~DX12Camera();
    
    void Translate(float x, float z);
    void Rotate(float yaw, float pitch);
    // update aspect ratio
    void Update();

    XMMATRIX GetView() { return m_view; }
    XMMATRIX GetProjection() { return m_projection; }

  private:
    XMMATRIX m_view;
    XMMATRIX m_projection;
    float m_fov;
    float m_near;
    float m_far;
    XMFLOAT3 m_cameraPosition;
    XMFLOAT3 m_lookAt;
    XMFLOAT3 m_up;

  private:
    DX12Camera(const DX12Camera&) = delete;
    DX12Camera& operator=(const DX12Camera&) = delete;
  };
}

