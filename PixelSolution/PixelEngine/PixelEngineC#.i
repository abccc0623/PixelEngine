%module PixelEngine
%{
/* 생성된 래퍼 파일에 포함될 헤더들 */
#include <windows.h>
#include "Export/PixelEngineAPI.h"
/* Type */
#include "Export/Type/PVector3.h"
#include "Export/Type/PMatrix.h"
#include "Export/Type/GlobalEnum.h"

/* GameObject */
#include "Export/Core/PixelObject.h"
#include "Export/Core/Module.h"
#include "Export/Core/GameObject.h"

/* Module */
#include "Export/Module/Camera.h"
#include "Export/Module/DebugCamera.h"
#include "Export/Module/Transform.h"
#include "Export/Module/LuaScript.h"
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

/* PVector3 */
%ignore PVector3::operator=;
%rename(Add) PVector3::operator+;
%rename(Sub) PVector3::operator-;
%rename(Mul) PVector3::operator*;
%rename(Div) PVector3::operator/;
%rename(AddAssign) PVector3::operator+=;
%rename(SubAssign) PVector3::operator-=;
%rename(MulAssign) PVector3::operator*=;
%rename(DivAssign) PVector3::operator/=;
%rename(DotStatic) PVector3::Dot(const PVector3&, const PVector3);
%rename(CrossStatic) PVector3::Cross(const PVector3&, const PVector3&);
%rename(NormalizeStatic) PVector3::Normalize(const PVector3);
%include "Export/Type/PVector3.h"

/* PMatrix */
%rename(CreateTranslationStatic) Matrix::CreateTranslation(const PVector3&);
%rename(CreateScaleStatic) Matrix::CreateScale(const PVector3&);
%ignore PMatrix::operator=;
%feature("cs:operator", "*") PMatrix::operator*(const PMatrix&) const;
%feature("cs:operator", "*") PMatrix::operator*(float) const;
%rename(Multiply) PMatrix::operator*(const PMatrix&) const;
%rename(MultiplyScalar) PMatrix::operator*(float) const;
%rename(AddAssign) PMatrix::operator+=;
%rename(SubAssign) PMatrix::operator-=;
%include "Export/Type/PMatrix.h"

%include "Export/Type/GlobalEnum.h"

/* GameObject */
%include "Export/Core/PixelObject.h"
%include "Export/Core/Module.h"
%include "Export/Core/GameObject.h"

/* Module */
%include "Export/Module/Camera.h"
%include "Export/Module/DebugCamera.h"
%include "Export/Module/Transform.h"
%include "Export/Module/LuaScript.h"
