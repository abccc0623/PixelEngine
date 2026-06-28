- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
Asset 창에서 바로 새 Lua 스크립트나 `.pxm`, `.scene` 파일을 만들고 이어서 편집을 시작하려고 경로를 따라가 봤다.

## 문제
`PixelSolution/PixelTool/Tool/AssetWindow/AssetWindow.xaml.cs`의 `CreateNewLua`, `CreateNewPxm`, `CreateNewScene`은 모두 `CreateNewFile()`로 들어가고, 이 함수는 `File.WriteAllText(newPath, "")`로 빈 파일만 만든다. 반면 같은 프로젝트의 `ToolbarWindow.xaml.cs`는 `CreateMainLua`, `CreateSceneLua`, `CreateModule`에서 `LuaFileManager.GetFileContent(...)`를 통해 기본 템플릿을 채운다. 같은 에디터 안에서 생성 진입점만 다를 뿐인데 결과물이 달라서, Asset 창으로 만든 파일은 시작 코드 없이 비어 있고 툴바로 만든 파일만 기본 골격을 가진다.

## 제안
파일 종류별 기본 템플릿을 한 군데로 모으고, Asset 창 생성도 그 경로를 재사용하게 맞추는 편이 낫다. 최소한 `.lua`, `.pxm`, `.scene`은 같은 이름의 툴바 생성과 동일한 초기 내용을 넣어 줘야 사용자가 생성 위치에 따라 다른 파일 품질을 겪지 않는다.
