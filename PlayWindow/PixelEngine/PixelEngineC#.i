%module PixelEngine
%{
/* 생성된 래퍼 파일에 포함될 헤더들 */
#include <windows.h>
#include "Export/PixelEngineAPI.h"
#include "Vector3.h"
#include "GameObject.h"
%}

/* 1. 표준 라이브러리 지원 모듈 추가 */
%include "windows.i"
%include "std_string.i"

/* 2. HWND 타이핑 (공백 오류 제거 및 IntPtr 연결) */
%typemap(ctype) HWND "void *"
%typemap(imtype) HWND "IntPtr"
%typemap(cstype) HWND "IntPtr"
%typemap(in) HWND %{ $1 = (HWND)$input; %}
%typemap(out) HWND %{ $result = (void *)$1; %}
%typemap(csin) HWND "$csinput"
%typemap(csout) HWND %{ return $imcall; %}

#define PIXEL_ENGINEDLL
/* SWIG가 파싱할 대상 헤더 파일들 */
%include "Export/PixelEngineAPI.h"
%include "Vector3.h"
%include "GameObject.h"