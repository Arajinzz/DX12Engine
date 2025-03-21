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
    
    void Translate(XMFLOAT3 translation);
    // in degrees
    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);
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

  private:
    DX12Camera(const DX12Camera&) = delete;
    DX12Camera& operator=(const DX12Camera&) = delete;
  };
}

