//
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <dxgi1_4.h>
#include <d3d12.h>

#include <Windows.h>
#include <wrl/client.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <array>
#include <type_traits>
#include <utility>
#include <chrono>
#include <string>
#include <exception>

using Microsoft::WRL::ComPtr;

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

typedef std::chrono::high_resolution_clock::time_point time_point;
typedef std::chrono::high_resolution_clock::duration duration;

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define CHECK_BOOL(expr)  do { if(!(expr)) { \
        assert(0 && #expr); \
        throw std::runtime_error(__FILE__ "(" LINE_STRING "): ( " #expr " ) == false"); \
    } } while(false)
#define CHECK_HR(expr)  do { if(FAILED(expr)) { \
        assert(0 && #expr); \
        throw std::runtime_error(__FILE__ "(" LINE_STRING "): FAILED( " #expr " )"); \
    } } while(false)

const uint32_t VENDOR_ID_AMD = 0x1002;
const uint32_t VENDOR_ID_NVIDIA = 0x10DE;
const uint32_t VENDOR_ID_INTEL = 0x8086;

template <typename T>
inline constexpr T CeilDiv(T x, T y)
{
    return (x+y-1) / y;
}
template <typename T>
inline constexpr T RoundDiv(T x, T y)
{
    return (x+y/(T)2) / y;
}

template <typename T>
inline constexpr T AlignUp(T val, T align)
{
    return (val + align - 1) / align * align;
}

static const float PI = 3.14159265358979323846264338327950288419716939937510582f;

static const D3D12_RANGE EMPTY_RANGE = {0, 0};

struct vec2
{
    float x, y;

    vec2() { }
    vec2(float x, float y) : x(x), y(y) { }

    float& operator[](uint32_t index) { return *(&x + index); }
    const float& operator[](uint32_t index) const { return *(&x + index); }

    vec2 operator+(const vec2& rhs) const { return vec2(x + rhs.x, y + rhs.y); }
    vec2 operator-(const vec2& rhs) const { return vec2(x - rhs.x, y - rhs.y); }
    vec2 operator*(float s) const { return vec2(x * s, y * s); }

    vec2 Normalized() const
    {
        return (*this) * (1.f / sqrt(x * x + y * y));
    }
};

struct vec3
{
    float x, y, z;

    vec3() { }
    vec3(float x, float y, float z) : x(x), y(y), z(z) { }

    float& operator[](uint32_t index) { return *(&x + index); }
    const float& operator[](uint32_t index) const { return *(&x + index); }

    vec3 operator+(const vec3& rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    vec3 operator-(const vec3& rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }

    vec3 Normalized() const
    {
        return (*this) * (1.f / sqrt(x * x + y * y + z * z));
    }
};

inline float Dot(const vec3& lhs, const vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}
inline vec3 Cross(const vec3& lhs, const vec3& rhs)
{
    return vec3(
        lhs.y * rhs.z - lhs.z * rhs.y,
	    lhs.z * rhs.x - lhs.x * rhs.z,
	    lhs.x * rhs.y - lhs.y * rhs.x);
}

struct vec4
{
    float x, y, z, w;

    vec4() { }
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    vec4(const vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) { }

    float& operator[](uint32_t index) { return *(&x + index); }
    const float& operator[](uint32_t index) const { return *(&x + index); }

    vec4 operator+(const vec4& rhs) const { return vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
    vec4 operator-(const vec4& rhs) const { return vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
    vec4 operator*(float s) const { return vec4(x * s, y * s, z * s, w * s); }
};

struct mat4
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4]; // [row][column]
    };

    mat4() { }

    mat4(
        float _11, float _12, float _13, float _14,
        float _21, float _22, float _23, float _24,
        float _31, float _32, float _33, float _34,
        float _41, float _42, float _43, float _44) :
        _11(_11), _12(_12), _13(_13), _14(_14),
        _21(_21), _22(_22), _23(_23), _24(_24),
        _31(_31), _32(_32), _33(_33), _34(_34),
        _41(_41), _42(_42), _43(_43), _44(_44)
    {
    }

    mat4(
        const vec4& row1,
        const vec4& row2,
        const vec4& row3,
        const vec4& row4) :
        _11(row1.x), _12(row1.y), _13(row1.z), _14(row1.w),
        _21(row2.x), _22(row2.y), _23(row2.z), _24(row2.w),
        _31(row3.x), _32(row3.y), _33(row3.z), _34(row3.w),
        _41(row4.x), _42(row4.y), _43(row4.z), _44(row4.w)
    {
    }

    mat4(const float* data) :
        _11(data[ 0]), _12(data[ 1]), _13(data[ 2]), _14(data[ 3]),
        _21(data[ 4]), _22(data[ 5]), _23(data[ 6]), _24(data[ 7]),
        _31(data[ 8]), _32(data[ 9]), _33(data[10]), _34(data[11]),
        _41(data[12]), _42(data[13]), _43(data[14]), _44(data[15])
    {
    }

    mat4 operator*(const mat4 &rhs) const
    {
        return mat4(
            _11 * rhs._11 + _12 * rhs._21 + _13 * rhs._31 + _14 * rhs._41,
            _11 * rhs._12 + _12 * rhs._22 + _13 * rhs._32 + _14 * rhs._42,
            _11 * rhs._13 + _12 * rhs._23 + _13 * rhs._33 + _14 * rhs._43,
            _11 * rhs._14 + _12 * rhs._24 + _13 * rhs._34 + _14 * rhs._44,

            _21 * rhs._11 + _22 * rhs._21 + _23 * rhs._31 + _24 * rhs._41,
            _21 * rhs._12 + _22 * rhs._22 + _23 * rhs._32 + _24 * rhs._42,
            _21 * rhs._13 + _22 * rhs._23 + _23 * rhs._33 + _24 * rhs._43,
            _21 * rhs._14 + _22 * rhs._24 + _23 * rhs._34 + _24 * rhs._44,

            _31 * rhs._11 + _32 * rhs._21 + _33 * rhs._31 + _34 * rhs._41,
            _31 * rhs._12 + _32 * rhs._22 + _33 * rhs._32 + _34 * rhs._42,
            _31 * rhs._13 + _32 * rhs._23 + _33 * rhs._33 + _34 * rhs._43,
            _31 * rhs._14 + _32 * rhs._24 + _33 * rhs._34 + _34 * rhs._44,

            _41 * rhs._11 + _42 * rhs._21 + _43 * rhs._31 + _44 * rhs._41,
            _41 * rhs._12 + _42 * rhs._22 + _43 * rhs._32 + _44 * rhs._42,
            _41 * rhs._13 + _42 * rhs._23 + _43 * rhs._33 + _44 * rhs._43,
            _41 * rhs._14 + _42 * rhs._24 + _43 * rhs._34 + _44 * rhs._44);
    }

    static mat4 Identity()
    {
        return mat4(
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 Translation(const vec3& v)
    {
        return mat4(
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            v.x, v.y, v.z, 1.f);
    }

    static mat4 Scaling(float s)
    {
        return mat4(
            s,   0.f, 0.f, 0.f,
            0.f, s,   0.f, 0.f,
            0.f, 0.f, s,   0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 Scaling(const vec3& s)
    {
        return mat4(
            s.x, 0.f, 0.f, 0.f,
            0.f, s.y, 0.f, 0.f,
            0.f, 0.f, s.z, 0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 RotationX(float angle)
    {
        const float s = sin(angle), c = cos(angle);
        return mat4(
            1.f, 0.f, 0.f, 0.f,
            0.f, c,   s,   0.f,
            0.f, -s,  c,   0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 RotationY(float angle)
    {
        const float s = sin(angle), c = cos(angle);
        return mat4(
            c,   s,  0.f,  0.f,
            -s,  c,  0.f,  0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 RotationZ(float angle)
    {
        const float s = sin(angle), c = cos(angle);
        return mat4(
            c,   0.f, -s,  0.f,
            0.f, 1.f, 0.f, 0.f,
            s,   0.f, c,   0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 Perspective(float fovY, float aspectRatio, float zNear, float zFar)
    {
        float yScale = 1.0f / tan(fovY * 0.5f);
        float xScale = yScale / aspectRatio;
        return mat4(
            xScale, 0.0f, 0.0f, 0.0f,
            0.0f, yScale, 0.0f, 0.0f,
            0.0f, 0.0f, zFar / (zFar - zNear), 1.0f,
            0.0f, 0.0f, -zNear * zFar / (zFar - zNear), 0.0f);
    }

    static mat4 LookAt(vec3 at, vec3 eye, vec3 up)
    {
        vec3 zAxis = (at - eye).Normalized();
        vec3 xAxis = Cross(up, zAxis).Normalized();
        vec3 yAxis = Cross(zAxis, xAxis);
        return mat4(
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            -Dot(xAxis, eye), -Dot(yAxis, eye), -Dot(zAxis, eye), 1.0f);
    }

    mat4 Transposed() const
    {
        return mat4(
            _11, _21, _31, _41,
            _12, _22, _32, _42,
            _13, _23, _33, _43,
            _14, _24, _34, _44);
    }
};

class RandomNumberGenerator
{
public:
    RandomNumberGenerator() : m_Value{GetTickCount()} {}
    RandomNumberGenerator(uint32_t seed) : m_Value{seed} { }
    void Seed(uint32_t seed) { m_Value = seed; }
    uint32_t Generate() { return GenerateFast() ^ (GenerateFast() >> 7); }
    bool GenerateBool() { return (GenerateFast() & 0x4) != 0; }

private:
    uint32_t m_Value;
    uint32_t GenerateFast() { return m_Value = (m_Value * 196314165 + 907633515); }
};

// Wrapper for RandomNumberGenerator compatible with STL "UniformRandomNumberGenerator" idea.
struct MyUniformRandomNumberGenerator
{
    typedef uint32_t result_type;
    MyUniformRandomNumberGenerator(RandomNumberGenerator& gen) : m_Gen(gen) { }
    static uint32_t min() { return 0; }
    static uint32_t max() { return UINT32_MAX; }
    uint32_t operator()() { return m_Gen.Generate(); }

private:
    RandomNumberGenerator& m_Gen;
};

void ReadFile(std::vector<char>& out, const wchar_t* fileName);
void SaveFile(const wchar_t* filePath, const void* data, size_t dataSize);

enum class CONSOLE_COLOR
{
    INFO,
    NORMAL,
    WARNING,
    ERROR_,
    COUNT
};

void SetConsoleColor(CONSOLE_COLOR color);

void PrintMessage(CONSOLE_COLOR color, const char* msg);
void PrintMessage(CONSOLE_COLOR color, const wchar_t* msg);

inline void Print(const char* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void Print(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void PrintWarning(const char* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintWarning(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintError(const char* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }
inline void PrintError(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }

void PrintMessageV(CONSOLE_COLOR color, const char* format, va_list argList);
void PrintMessageV(CONSOLE_COLOR color, const wchar_t* format, va_list argList);
void PrintMessageF(CONSOLE_COLOR color, const char* format, ...);
void PrintMessageF(CONSOLE_COLOR color, const wchar_t* format, ...);
void PrintWarningF(const char* format, ...);
void PrintWarningF(const wchar_t* format, ...);
void PrintErrorF(const char* format, ...);
void PrintErrorF(const wchar_t* format, ...);

