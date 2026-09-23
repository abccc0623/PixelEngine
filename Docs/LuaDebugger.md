# Lua 중단점과 변수 확인

PixelTool의 Lua 편집기에서 실행 중인 LuaJIT 코드를 디버깅할 수 있습니다.

## 사용 순서

1. 솔루션을 **Debug / x64**로 빌드하고 PixelTool을 실행합니다. 기존에 켜 둔 PixelTool은 종료 후 다시 실행해야 새 DLL이 적용됩니다.
2. `.lua`, `.pxm`, `.scene` 파일을 열고, 수정했다면 **Ctrl+R**로 저장하고 다시 불러옵니다.
3. 실행할 코드 줄의 **줄 번호 왼쪽 여백을 클릭**하거나 커서를 놓고 **F9**를 누릅니다. 빨간 원이 중단점이며, 다시 누르면 해제됩니다.
4. **Play 모드**로 실행합니다. 엔진 업데이트에서 해당 줄에 도달하면 실행 전에 멈추고 변수 창이 열립니다. Play 전환으로 다시 불러오는 스크립트도 대상입니다.
5. 왼쪽은 저장된 소스와 멈춘 줄, 오른쪽은 변수입니다. 테이블의 화살표를 누르면 항목을 펼칠 수 있습니다. 위쪽 호출 스택에서 다른 함수를 선택하면 그 함수의 변수를 확인합니다.

| 조작 | 동작 |
| --- | --- |
| F9 / 줄 번호 왼쪽 클릭 | 중단점 설정·해제 |
| 디버깅 켜짐·꺼짐 버튼 | 지정한 중단점을 유지하면서 디버깅 사용 여부 전환 |
| 변수 창에서 F5 | 다음 중단점까지 계속 실행 |
| 변수 창에서 F11 | 같은 Lua 실행 흐름의 다음 파일 소스 줄로 진입. 함수 안으로도 들어감 |
| 변수 창에서 Esc / 창 닫기 | 디버깅을 해제하고 게임 실행 재개 |

`local:`은 현재 함수의 지역 변수, `upvalue:`는 상위 함수에서 가져온 변수입니다. 선언 줄에서 멈췄다면 아직 그 선언은 실행되기 전이므로 변수가 보이지 않을 수 있습니다. 다음 줄에 중단점을 두거나 F11을 누르세요.

## 현재 지원 범위

- 지역 변수, 함수 인자, upvalue, 테이블 미리보기, 호출 스택, 코루틴 중단점을 지원합니다.
- 변수 창은 읽기 전용입니다. 임의 표현식 실행, 변수 값 변경, 전역 변수 전체 검색, 조건부 중단점, Step Over/Out은 포함하지 않습니다.
- 테이블은 3단계 깊이, 테이블당 64개 항목, 전체 2,048개 변수 노드까지 표시합니다. 문자열은 512바이트, 호출 스택은 32프레임까지 표시합니다. 순환 참조와 생략 부분은 표시됩니다.
- C++ userdata/FFI cdata의 내부 필드는 펼치지 않습니다. `__index`, `__tostring` 등 사용자 코드는 검사 중 실행하지 않습니다.
- 파일을 편집하면 해당 파일의 중단점을 해제합니다. **저장·재시작 후 다시 지정**하세요. 변수 창의 소스는 현재 디스크 파일이므로 외부에서 파일을 수정해도 실행 중인 코드가 자동 교체되지는 않습니다.
- 중단점은 이번 PixelTool 실행 동안 유지됩니다. 빈 줄·주석 등 실행되지 않는 줄에서는 멈추지 않습니다.
- 앱 최초 초기화처럼 엔진 업데이트 밖에서 호출되는 Lua는 중단하지 않습니다. Play 전환/재시작 후 엔진 업데이트에서 실행되는 Lua를 대상으로 합니다.
- 디버깅 중에는 정확한 줄 이벤트를 위해 JIT 최적화를 끕니다. 해제하면 이전 JIT 활성 상태와 디버그 훅을 복원합니다. 스크립트에서 `debug.sethook` 또는 `jit.on`을 직접 조작하는 경우와 동시 사용은 지원하지 않습니다.

## 구현과 검증

`LuaDebugger.h`가 Lua 실행 스레드에서 호출 스택과 값을 복사합니다. WPF는 그 복사본만 표시하며, 검사 중에는 원래 Lua 스택이 유지됩니다. 변수 창이 열려 있는 동안 편집기 본체는 모달 창의 소유자로 비활성화됩니다. 화면 크기/포커스 변경은 큐에 넣어 실행 스레드가 재개된 뒤 처리하며, 일시 정지 시간은 게임 시간 계산에서 제외합니다.

`Tests/LuaDebuggerTests.cpp`는 실제 프로젝트의 LuaJIT 라이브러리를 사용해 중단점, 변수 값, 테이블 순환 참조, 단계 실행, 코루틴, upvalue, 호출 스택, Lua 스택 보존, 검사 중 메타메서드 미실행, 디버깅 해제를 검증합니다.

Visual Studio의 **x64 Native Tools Command Prompt**에서 저장소 루트를 기준으로 실행합니다.

```bat
mkdir .tmp\lua-debug-native
cl /nologo /EHsc /std:c++20 /MD /utf-8 /I PixelSolution\PixelEngine /I PixelSolution\PixelEngine\Import Tests\LuaDebuggerTests.cpp /Fo.tmp\lua-debug-native\LuaDebuggerTests.obj /Fe.tmp\lua-debug-native\LuaDebuggerTests.exe /link PixelSolution\PixelEngine\Library\lua51.lib
copy PixelSolution\PixelEngine\Library\lua51.dll .tmp\lua-debug-native\lua51.dll
.tmp\lua-debug-native\LuaDebuggerTests.exe
```
