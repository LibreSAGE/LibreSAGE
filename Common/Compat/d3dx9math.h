#pragma once

// D3D9 compatibility shim -- math subset.
//
// This header deliberately shares the name of DXVK-Native's <d3dx9math.h> and
// shadows it on the include path (Common/Compat is a -I dir, searched before the
// DXVK -isystem dir). DXVK ships the Wine-derived D3DX9 headers, but they assume a
// fuller Win32 environment than this toolchain provides (they need types such as
// DOUBLE / LF_FACESIZE that our Common/Compat/windows.h shim does not define) and
// there is no D3DX9 implementation library to link. So we declare only the small
// subset of the D3DX math API the engine actually uses and implement it ourselves
// (d3dx9math.cpp, on top of GLM).

#include "windows.h"
#include <d3d9.h>

#ifndef D3DX_PI
#define D3DX_PI 3.141592654f
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct D3DXMATRIX : D3DMATRIX
{
#ifdef __cplusplus
  D3DXMATRIX() = default;
  D3DXMATRIX(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33);
  D3DXMATRIX operator *(const D3DXMATRIX &other) const;
  D3DXMATRIX operator *= (const D3DXMATRIX& other);
#endif
} D3DXMATRIX;

typedef D3DVECTOR D3DXVECTOR3;

typedef struct D3DXVECTOR4
{
#ifdef __cplusplus
  D3DXVECTOR4() {}
  D3DXVECTOR4(FLOAT x, FLOAT y, FLOAT z, FLOAT w) : x(x), y(y), z(z), w(w) {}

  operator FLOAT* () { return &x; }
#endif
  FLOAT x, y, z, w;
} D3DXVECTOR4;

D3DXMATRIX *WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixScaling(D3DXMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz);
D3DXMATRIX *WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z);
D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2);
D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixRotationZ(D3DXMATRIX *pOut, FLOAT angle);
D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM);
FLOAT WINAPI D3DXVec4Dot(CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

inline D3DXMATRIX D3DXMATRIX::operator*(const D3DXMATRIX &other) const
{
  D3DXMATRIX result;
  D3DXMatrixMultiply(&result, this, &other);
  return result;
}
inline D3DXMATRIX D3DXMATRIX::operator *= (const D3DXMATRIX& other)
{
  D3DXMatrixMultiply(this, this, &other);
  return *this;
}

#endif
