#pragma once

// D3D9 compatibility shim -- umbrella header. Shadows DXVK-Native's <d3dx9.h>
// (which pulls in the full, non-compiling D3DX9 family). We only need the math
// and core/texture subsets.
#include <d3dx9math.h>
#include <d3dx9core.h>
