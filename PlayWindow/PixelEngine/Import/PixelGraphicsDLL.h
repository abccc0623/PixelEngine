#pragma once

#ifdef PICNICGRAPHICS_EXPORTS
#define PICNIC_GRAPHICS_DLL extern "C" __declspec(dllexport)
#else
#define PICNIC_GRAPHICS_DLL extern "C" __declspec(dllimport)
	#ifdef _DEBUG
		#pragma comment(lib ,"PixelGraphics")
	#else
		#pragma comment(lib ,"PixelGraphics")
	#endif
#endif 
