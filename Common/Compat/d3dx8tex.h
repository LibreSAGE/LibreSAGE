#pragma once

#include "windows_base.h"

HRESULT WINAPI
D3DXFilterTexture(
	LPDIRECT3DBASETEXTURE8 pBaseTexture,
	CONST PALETTEENTRY *pPalette,
	UINT SrcLevel,
	DWORD Filter);

typedef enum eD3DXFilters
{
    D3DX_FILTER_NONE,
    D3DX_FILTER_TRIANGLE,
    D3DX_FILTER_BOX,
} eD3DXFilters;