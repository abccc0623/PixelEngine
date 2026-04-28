// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H


// 2. 링커 에러와 버전 미스매치를 일으키는 모든 범인들을 강제로 잠급니다.
#ifndef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#define JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#endif

#ifndef JPH_OBJECT_STREAM
#define JPH_OBJECT_STREAM
#endif

// 3. Jolt 필수 헤더 직접 포함 (pch 의존성 탈피)
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"
#include <string>
#include <windows.h>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <concepts>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <json.hpp>
#include <json_fwd.hpp>

#include <sol/forward.hpp>
#include "sol.hpp"
#include "Log.h"

#endif //PCH_H
