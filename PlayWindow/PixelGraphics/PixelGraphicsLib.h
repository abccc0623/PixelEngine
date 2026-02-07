#pragma once

#ifdef _DEBUG
#pragma comment( lib, "DirectXTK_x64d.lib" )
#pragma comment( lib, "DirectXTex_x64d.lib" )
#else
#pragma comment( lib, "DirectXTK_x64r.lib" )
#pragma comment( lib, "DirectXTex_x64r.lib" )
#endif
#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3dcompiler.h>
#include <d3d11.h>
#include "WICTextureLoader.h"
#include "DirectXTex.h"