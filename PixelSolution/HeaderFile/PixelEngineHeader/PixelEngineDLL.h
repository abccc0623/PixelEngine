#pragma once
#include <windows.h>
typedef unsigned char byte;

#ifdef ENGINE_INTERFACE
#define PIXEL_ENGINEDLL __declspec(dllexport)
#else
#define PIXEL_ENGINEDLL __declspec(dllimport)
	#ifdef _DEBUG
		#pragma comment(lib ,"PixelEngine.lib")
	#else
		#pragma comment(lib ,"PixelEngine.lib")
	#endif
#endif

#ifdef PIXEL_EDITOR
// 에디터 모드일 때: 에디터 함수는 정상, 인게임 전용 함수는 경고
#define EDITOR_FUNC 
#define RUNTIME_FUNC [[deprecated("이 함수는 런타임 전용입니다.")]]
#else
// 인게임 모드일 때: 에디터 전용 함수는 경고(혹은 에러), 인게임 함수는 정상
#define EDITOR_FUNC [[deprecated("이 함수는 에디터 전용입니다. 빌드에 포함될 수 없습니다.")]]
#define RUNTIME_FUNC 
#endif