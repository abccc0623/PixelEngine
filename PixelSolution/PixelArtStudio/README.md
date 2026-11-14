# Pixel Art Studio

PixelSolution의 독립 .NET 8 WPF 앱입니다. 기존 PixelTool이나 게임 프로젝트는 수정하지 않습니다.

## 실행

```powershell
dotnet build .\PixelArtStudio.csproj -c Release
.\..\bin\Release\PixelArtStudio.exe
```

생성 환경 설치는 `Backend/README.md`를 참고하세요. 기본 Python은 `%LOCALAPPDATA%\PixelArtStudio\venv\Scripts\python.exe`입니다. 가상 환경, 모델, 출력은 Git 외부 `%LOCALAPPDATA%\PixelArtStudio`에 저장됩니다.

- 텍스트 설명 → SDXL + Pixel Art XL LoRA → 저해상도 PNG 미리보기
- PNG 열기 → 수정 요청 → img2img 변형 (정밀한 지시 편집이나 동일 캐릭터 보장은 아닙니다)
- 배경 제거 선택 → 투명 PNG 후처리. 작은 부위가 제거될 수 있으므로 미리보기 확인
- 생성 중 중지 또는 창 닫기 → 이 앱이 시작한 Python 프로세스만 종료
- 생성 단계는 4~50, 출력 32/64/128 정사각형. 영문 프롬프트 권장
- 생성 작업마다 새 프로세스를 사용하여 완료/취소 후 VRAM을 반환합니다. 모델 재로드 시간은 발생합니다.

## PixelTool 연동 계약

현재 PixelTool 메뉴/AI 도구에 버튼을 추가하지는 않았습니다. 독립 앱에 다음 실행 계약을 구현했습니다.

```powershell
PixelArtStudio.exe --request request.json --result result.json
# 사용자 클릭 없이 시작하려면 --generate 추가
```

요청 예:
```json
{"prompt":"pixel art, green slime, white background","width":64,"height":64,"steps":20,"seed":-1,"transparent":true,"referenceImage":null}
```

`--request`만 전달하면 입력과 미리보기만 준비하고 생성은 시작하지 않습니다. 완료 결과:
```json
{"status":"completed","outputPath":"C:\\...\\sprite.png","error":null}
```

취소는 `cancelled`, 실패는 `failed`와 원인입니다. 생성 결과는 앱 전용 폴더에 저장한 후 결과 JSON으로 반환합니다. 임의 게임 Asset을 덮어쓰지 않습니다. PNG 내보내기는 저장 대화상자의 덮어쓰기 확인을 따릅니다.

## 범위와 한계

로컬 추론에는 API 비용이 없지만 모델 다운로드/디스크/전력은 필요합니다. 모델별 라이선스는 Backend 문서에 있습니다. 애니메이션/스프라이트 시트, PixelTool 자동 등록, 한글 번역 LLM은 이번 버전에 포함하지 않습니다. GPU를 다른 앱/로컬 LLM과 공유하므로 동시 실행 시 느려질 수 있습니다.

## 이 PC에서 확인한 결과

- Release 빌드: 경고/오류 0개. 백엔드 입력 검증 테스트와 의존성 검사 통과.
- RTX 3080 10GB에서 64×64 투명 PNG 실제 생성. 첫 생성 157.23초(모델 최초 다운로드 포함), 캐시된 참조 수정 약 18~19초.
- 실제 UI에서 생성 → 중지 → 재생성, PNG 열기/내보내기, 입력 오류 표시, JSON 실패 결과 반환 확인.
- 참조 이미지의 파랑→초록 변경은 정확하지 않았습니다. 정밀한 명령 편집 모델이 아닙니다.
- 일부 생성에서는 배경 제거가 캐릭터를 많이 지웠습니다. 결과 폴더의 `sprite.source.png`에 제거 전 원본을 함께 저장하며 보이는 픽셀이 매우 적으면 경고합니다. 이 검사는 모든 품질 문제를 잡지는 못합니다.
- 수정 설명은 변경 명령만이 아니라 원하는 최종 모습 전체를 영문으로 적으세요. 변경 강도를 올리면 원본 형태도 바뀔 수 있습니다.
- 모델 약 6.78GiB, 가상환경 약 5.49GiB. Python 및 다운로드 캐시 등 추가 공간은 별도입니다.

## 화면 모드 / 백그라운드 모드

실행 전에 `--mode`를 선택합니다. 생략하면 `ui`이므로 기존 바탕화면 바로가기는 그대로 화면을 엽니다.

```powershell
# 화면 모드
.\..\bin\Release\PixelArtStudio.exe --mode ui

# 창 없는 모드: 요청을 자동 실행하고 종료합니다. --generate는 불필요합니다.
$p = Start-Process .\..\bin\Release\PixelArtStudio.exe -ArgumentList '--mode','background','--request','C:\Jobs\request.json','--result','C:\Jobs\result.json','--cancel-file','C:\Jobs\cancel.signal' -PassThru -Wait
$p.ExitCode
```

경로에 공백이 있으면 Start-Process 인수에 따옴표를 포함하세요. 요청 JSON은 위 형식이며 `outputPath`를 지정하면 그 경로로 저장합니다. 생략 시 앱 전용 outputs에 새 경로를 만듭니다. `--result`는 필수이고 요청/결과 경로는 서로 달라야 합니다. 요청별로 별도 결과·취소 경로를 사용하세요.

- 결과 JSON: `status`, `exitCode`, `error`, `outputPath`, `generation`(seed/elapsedSeconds/sourcePath/warning), `logPath`.
- 진행 로그: `result.json.log`. 로그와 PNG 생성은 로컬 동작이며 LLM 호출/토큰이 없습니다.
- 종료코드: 0 완료, 1 생성/파일 오류, 2 실행 인수/JSON 형식 오류, 124 제한 시간 초과, 130 취소.
- `--timeout-seconds 600` 기본값. 1~86400 지정 가능.
- 취소: 전달한 `--cancel-file` 위치에 빈 파일을 만들면 진행 중인 Python 프로세스 트리를 종료하고 cancelled 결과를 씁니다. 이미 존재하는 신호 파일은 실행 전 취소입니다. 다음 요청에는 새 신호 경로를 사용하세요.
- Windows 강제 프로세스 종료/전원 끄기는 결과 JSON 기록을 보장하지 않습니다. 정상 취소에는 취소 파일을 사용하세요.
- `--python <python.exe>`로 전용 Python 경로를 바꿀 수 있습니다.
- `Test-Background.ps1`은 재현용 테스트입니다. 이 PC의 PowerShell 정책으로 파일 실행은 차단되어 동일 테스트를 직접 명령으로 실행했습니다. 실행 정책은 변경하지 않았습니다.

검증: 실제 headless 20단계 GPU 생성 19.58초, 완료 exit0, 64×64 PNG와 source PNG 생성, 실행 중 주 창 핸들 0 확인. 실패 exit1·실행 중 취소 exit130·시간초과 exit124도 창 없이 결과 반환을 확인했습니다.

## 솔루션 빌드 및 공용 출력

Client/PixelTool과 같이 `PixelSolution\bin\<구성>`에 빌드됩니다.

- Debug: `PixelSolution\bin\Debug\PixelArtStudio.exe`
- Editor_d: `PixelSolution\bin\Editor_d\PixelArtStudio.exe`
- Release: `PixelSolution\bin\Release\PixelArtStudio.exe`

`PixelSolution.slnx`에서 x64 프로젝트로 빌드됩니다. PixelTool에는 런타임 DLL 참조가 아닌 빌드 전용 ProjectReference를 넣어 Studio가 먼저 빌드되도록 했습니다. 별도 중복 PostBuild 호출은 추가하지 않습니다. 네이티브 Client/엔진을 Studio에 참조하지 않습니다.

PixelTool Release 설치 패키지는 별도 payload를 만들기 때문에 설치 스크립트가 Studio를 self-contained로 `PixelArtStudio` 하위 폴더에 publish합니다. 설치된 앱 경로는 `<PixelTool 설치 폴더>\PixelArtStudio\PixelArtStudio.exe`입니다. Python/가상환경/모델은 패키지에 넣지 않으며 대상 PC에서 별도 설치가 필요합니다. 솔루션 빌드만으로 수 GB 모델을 내려받지 않습니다.
## 제작 보조 기능 (소스 추가, 빌드 미실행)

- 유형별 프롬프트: 캐릭터 / 배경 / 타일을 선택하면 해당 유형의 프롬프트를 코드에서 붙입니다. 배경·타일은 캐릭터의 흰 배경/단독 스프라이트 지시를 사용하지 않습니다. 타일의 완전한 이음새는 보장하지 않습니다.
- 생성 설정: 실제 seed와 요청 설정을 결과 옆 메타데이터에 보존합니다. 같은 seed라도 모델·라이브러리·GPU 환경이 달라지면 픽셀 단위 동일 결과를 보장하지 않습니다.
- 원본 비교/복구: 배경 제거 전 source PNG를 보관하고 결과와 비교합니다. source는 고해상도 생성 원본이므로 최종 픽셀 이미지와 크기가 다를 수 있습니다. 복구는 원본을 다시 선택하는 것이며 기존 파일을 지우지 않습니다.
- 색 변경/반전: AI 호출 없이 Pillow로 처리합니다. 색 변경은 지정한 RGB 값과 정확히 같은 픽셀만 바꾸고 알파는 보존합니다. 명암이 다른 색까지 자동으로 바꾸지는 않습니다.
- 프레임/시트: 기준 캐릭터를 참조해 개별 프레임을 생성하고 선택한 순서로 가로 시트를 만듭니다. 알파 영역의 바닥과 중앙을 맞추며, 이는 실제 발 관절을 인식하는 방식이 아닙니다. 투명 이미지가 아닌 프레임은 캔버스 바닥 기준이 됩니다. AI가 캐릭터 외형이나 동작의 연속성을 보장하지 않으므로 각 프레임을 확인해야 합니다.

이 개선 작업은 사용자 요청에 따라 빌드, publish, 설치 패키지 생성 없이 소스 및 비빌드 검사만 진행합니다. 기존 실행 파일에는 새 기능이 반영되지 않습니다.
### 저장 파일

생성 결과 폴더에는 `sprite.png`, `sprite.source.png`, `sprite.generation.json`(실제 seed, 요청, 적용 프롬프트, 모델), UI 세션의 `settings.json`이 남습니다. UI의 설정 불러오기는 `settings.json` 형식을 사용합니다. `sprite.generation.json`은 생성 백엔드 요청 형식이며 `--request`로 재사용할 수 있습니다. 재사용할 때에는 `outputPath`를 새 경로로 지정하세요.

비교는 두 이미지의 동시 나열이 아니라 원본/결과 버튼으로 같은 미리보기를 전환하는 방식입니다. 프레임은 자동 일괄 생성하지 않으며, 기준 고정 → 원하는 프레임 설명 → 프레임 생성 → 결과 확인 → 목록 추가 순서입니다. 목록 위/아래 이동으로 시트 순서를 정합니다. 이미지 처리와 시트 조립은 LLM·GPU 호출이 없습니다.

시트 완성 후 PNG 내보내기와 별도의 시트 좌표 JSON 내보내기 버튼을 사용하세요. 좌표 JSON의 imagePath/sourcePath는 생성 당시 로컬 경로이므로 다른 PC로 옮길 때에는 실제 PNG 위치에 맞게 사용해야 합니다.

이번 개선의 비빌드 검증: Python 단위/CLI 테스트 20개 통과, XAML XML 및 이벤트 핸들러 참조 정적 확인, git diff --check 통과. C# 컴파일 및 새 GUI 실제 조작 검증은 실행하지 않았습니다.
## 배경 제거 시 작은 내부 누락 자동 보호

수동 복구 브러시는 제거했습니다. 생성 후 배경 제거 단계에서 `protectInterior`(기본 true)를 적용합니다. UI의 '작은 내부 누락 자동 보호' 옵션 또는 요청 JSON의 `protectInterior: false`로 끌 수 있고 설정 저장/복원에도 반영됩니다.

최종 출력 격자에서 알파 128 미만 영역을 8방향으로 검사합니다. 이미지 외곽과 연결된 영역은 배경으로 남기고, 내부에 완전히 닫힌 작은 영역만 후보로 봅니다. 후보 크기는 전체 픽셀의 0.1% 이하(최대 16픽셀; 32px=1, 64px=4, 128px=16)로 제한하며, 그 안에서도 알파 8 이하의 거의 지워진 픽셀만 원본 RGB와 알파로 복구합니다. 반투명 경계는 그대로 둡니다.

캐릭터의 흰색도 원본 색으로 복원하지만, 의미 기반 캐릭터 인식은 아닙니다. 작은 의도적 구멍도 채울 수 있습니다. 큰 내부 누락, 외부 배경과 연결된 누락, 캐릭터가 거의 모두 지워진 경우에는 복구하지 못합니다. 원본 source PNG는 계속 보존하며 복원 수와 경고를 결과/메타데이터에 기록합니다. 정확한 캐릭터 마스크를 항상 보장하는 기능이 아닙니다.

비빌드 검증: Python 28개 테스트 통과. 인위적인 닫힌 흰색 누락은 복원되고 외부 연결/큰 구멍/반투명 경계는 유지됩니다. 기존 실제 생성물 8쌍은 조건에 해당하는 누락이 없어 복원 0픽셀로, 실제 샘플의 시각 품질 개선을 입증하지는 못했습니다. 원본 SHA256은 모두 동일했습니다. 비교 리포트는 %LOCALAPPDATA%\PixelArtStudio\smoke-tests\interior-protection\report.json 에 있습니다. C# 빌드 및 새 GUI 실행은 하지 않았습니다.
## Markdown 리소스 일감 입력

`Examples/orange-patrol-enemy.md`를 시작 서식으로 사용하세요. 게임 기능 구현 일감 전체를 이미지 프롬프트로 보내지 않습니다. Markdown의 리소스 종류, 이미지 설명, 출력 크기, 배경, 시드를 정해진 이름으로 읽고 검증하여 생성 옵션에 적용합니다. 이미지 설명은 영어로 작성하세요. 자동 번역 LLM은 연결하지 않았습니다.

제목/id/aliases/용도/완료 확인/주의사항/변경 이력은 참고 문서입니다. 코드나 API 구현 명령은 이미지 생성 요청이 아닙니다. 완료 확인은 사람이 결과를 검수하는 목록이며 앱이 외형·게임 기능 구현 여부를 자동 합격 판정하지 않습니다. 일감으로 생성 버튼으로 검증과 생성을 실행하고 PNG 내보내기로 원하는 경로에 저장합니다.

JSON 일감 모드는 문서화된 생성 옵션만 받으며 잘못된 자료형/지원하지 않는 필드는 오류로 알립니다. 생성 파일은 앱 outputs에 보존하고 GUI에서 일감의 outputPath로 임의 파일을 덮어쓰지 않습니다. 일반 그림 설명 텍스트와 Markdown 일감은 명시적으로 구분하여 선택하세요.

## 공통 아트 스타일

리소스 이미지 설명과 별도로 공통 스타일 및 스타일 제외 설명을 입력하고 JSON 파일로 저장/불러옵니다. 각 프로젝트의 `Editor/ArtStyle.json`처럼 서로 다른 파일을 선택하여 관리하세요. 이 독립 앱은 PixelTool의 현재 프로젝트를 자동 추정하지 않으며 스타일을 프로젝트 전체에 자동 전파하지 않습니다.

백그라운드 요청에는 `stylePrompt`, `styleNegativePrompt` 문자열을 전달할 수 있습니다(각 2,000자 이하). 실제 적용 프롬프트와 스타일은 생성 메타데이터에 기록됩니다. 같은 스타일을 재사용해도 캐릭터 비율·색상·선 모양이 완전히 같다고 보장하지 않습니다. seed는 난수 재현용이며 스타일을 고정하는 값이 아닙니다.
### 일감 사용 순서와 기본값
1. 일감 형식을 Markdown 리소스 일감으로 선택하고 예제 MD를 열거나 붙여넣습니다.
2. 공통 스타일을 선택한 뒤 일감으로 생성을 누릅니다. 검증/설정 적용/생성을 한 번에 진행합니다.
3. 직접 설정 탭에서는 옵션을 확인하고 변경할 수 있습니다. MD 일감은 공통 스타일을 지우지 않습니다.
4. 새 이미지 생성 → 결과 확인 → PNG 내보내기 순서로 진행합니다.

MD에서 인식하는 항목은 `## 내용` 아래의 `리소스 종류`, `이미지 설명`, `출력 크기`, `배경`, `시드`입니다. 코드 블록은 무시합니다. 그 외 항목은 원문 메모로만 보존됩니다. 이미지 설명 외 항목을 생략하면 기본값은 캐릭터/64x64/투명/seed -1이며, 생성 단계 20·변경 강도0.6·작은 내부 보호 사용입니다. 결과 폴더의 `workitem.json`은 입력 원문/검수 메모 기록이며 이미지 생성 백엔드 요청과는 다릅니다.

JSON 예제는 `Examples/orange-patrol-enemy.json`, 스타일 파일 예제는 `Examples/art-style.json`입니다. GUI JSON은 prompt/category/width/height/steps/seed/strength/negativePrompt/transparent/protectInterior/acceptanceCriteria/stylePrompt/styleNegativePrompt를 지원합니다. 이름의 대소문자를 정확히 사용하세요. JSON의 스타일 필드 생략 시 현재 공통 스타일을 유지합니다. 단순 텍스트 모드는 영문 그림 설명만 넣으며 기존 생성 옵션을 유지합니다.

이번 비빌드 확인: Python 32개 테스트 통과. C# 일감 파서는 소스 검토만 수행했으며 컴파일·실제 GUI 입력 테스트는 사용자 요청에 따라 하지 않았습니다.
### 두 가지 생성 탭과 설정 폭

- **일감 생성**: Markdown/텍스트/JSON 붙여넣기 → 일감으로 생성. 검증 성공시에만 적용 후 생성합니다. 붙여넣기/파일 열기만으로 GPU 작업이 시작되지 않습니다.
- **직접 설정**: 기존 종류·크기·배경·영문 설명·수정·색 변경·프레임 기능을 사용합니다. 직접 생성은 현재 화면 값을 사용하며 이전 일감 검수 메모를 자동으로 적용하지 않습니다.
- 공통 스타일은 두 방식에서 공유합니다. Markdown 또는 스타일 필드가 없는 JSON은 현재 스타일을 유지합니다.
- 일감으로 생성할 때에는 해당 일감 값과 기본값이 직접 설정 값을 교체합니다. 직접 설정을 바꾼 뒤 일감으로 생성을 다시 누르면 일감 기준으로 다시 적용됩니다. 텍스트 모드는 설명만 변경하고 옵션은 유지합니다.
- 미리보기와 오른쪽 설정 사이의 세로 구분선을 드래그하여 폭을 조절합니다. 양쪽 최소 폭과 설정 최대 폭을 제한하며 긴 설정은 스크롤합니다.

이 탭 변경은 소스 정적 확인만 했습니다. 사용자 요청대로 빌드·컴파일·실행하지 않았으므로 실제 창의 조작 확인은 남아 있습니다.
