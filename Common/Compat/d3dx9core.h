#pragma once

// D3D9 compatibility shim -- core/texture/shader subset.
//
// Shares the name of DXVK-Native's <d3dx9core.h> and shadows it on the include
// path (see d3dx9math.h for why the real DXVK D3DX9 headers can't be used here).
// Declares only the small subset of the D3DX API the engine actually uses; the
// implementations live in d3dx9core.cpp / d3dx9tex.cpp.

#include "windows.h"
#include <d3d9.h>
#include <limits.h>

#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT                     UINT_MAX
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum eD3DXFilters
{
    D3DX_FILTER_NONE,
    D3DX_FILTER_TRIANGLE,
    D3DX_FILTER_BOX,
} eD3DXFilters;

// ---------------------------------------------------------------------------
// Textures / surfaces (d3dx9tex.cpp)
// ---------------------------------------------------------------------------

HRESULT WINAPI
D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	LPDIRECT3DTEXTURE9 *ppTexture);

typedef enum _D3DXIMAGE_FILEFORMAT
{
    D3DXIFF_BMP = 0,
    D3DXIFF_JPG = 1,
    D3DXIFF_TGA = 2,
    D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4,
    D3DXIFF_PPM = 5,
    D3DXIFF_DIB = 6,
    D3DXIFF_HDR = 7,
    D3DXIFF_PFM = 8,
    D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

typedef struct D3DXIMAGE_INFO
{
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
    D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

HRESULT WINAPI
D3DXCreateTextureFromFileExA(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcFile,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,

	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO *pSrcInfo,
	PALETTEENTRY *pPalette,

	LPDIRECT3DTEXTURE9 *ppTexture);

HRESULT WINAPI
D3DXLoadSurfaceFromSurface(
	LPDIRECT3DSURFACE9 pDestSurface,
	CONST PALETTEENTRY *pDestPalette,
	CONST RECT *pDestRect,
	LPDIRECT3DSURFACE9 pSrcSurface,
	CONST PALETTEENTRY *pSrcPalette,
	CONST RECT *pSrcRect,
	DWORD Filter,
	D3DCOLOR ColorKey);

// D3DX8-era helper with no D3DX9 equivalent (D3D9 code is meant to use dxerr9's
// DXGetErrorString9A). WW3D2's error logging still calls it, so keep the shim.
HRESULT WINAPI
D3DXGetErrorStringA(
	HRESULT hr,
	LPSTR pBuffer,
	UINT BufferLen);

// Taken and adopted from Wine 3.21
HRESULT WINAPI
D3DXFilterTexture(
	LPDIRECT3DBASETEXTURE9 pBaseTexture,
	CONST PALETTEENTRY *pPalette,
	UINT SrcLevel,
	DWORD Filter);

HRESULT WINAPI
D3DXCreateCubeTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Size,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	LPDIRECT3DCUBETEXTURE9 *ppCubeTexture);

HRESULT WINAPI
D3DXCreateVolumeTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	UINT Depth,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture);

// ---------------------------------------------------------------------------
// Shaders / misc (d3dx9core.cpp)
// ---------------------------------------------------------------------------

typedef struct D3DXBUFFER *LPD3DXBUFFER;

HRESULT WINAPI
D3DXAssembleShader(
	LPCVOID pSrcData,
	UINT SrcDataLen,
	DWORD Flags,
	LPD3DXBUFFER *ppConstants,
	LPD3DXBUFFER *ppCompiledShader,
	LPD3DXBUFFER *ppCompilationErrors);

HRESULT WINAPI
D3DXAssembleShaderFromFileA(
	LPCSTR pSrcFile,
	DWORD Flags,
	LPD3DXBUFFER *ppConstants,
	LPD3DXBUFFER *ppCompiledShader,
	LPD3DXBUFFER *ppCompilationErrors);

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF);

#ifdef __cplusplus
}
#endif
