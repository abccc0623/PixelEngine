- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
외부 텍스처나 데이터를 Asset 창으로 가져온 뒤 실행 중 엔진이 그 리소스를 언제 다시 읽는지 에디터 쪽 경로를 확인했다.

## 문제
`PixelSolution/PixelTool/Tool/AssetWindow/AssetWindow.xaml.cs`의 `ImportFiles()`와 `CopyExternalFile()`은 선택한 외부 파일을 Asset 폴더로 `File.Copy`한 뒤 `SelectFolder()`나 `Refresh()`로 목록만 갱신한다. 같은 파일 안에는 복사 뒤 `PixelEngineNative.Import(...)`나 그에 준하는 리소스 재로드 호출이 없다. 사용자는 Asset 창에서 파일이 보이기 시작했으니 엔진도 바로 읽었을 것처럼 느끼기 쉬운데, 실제 코드는 에디터 트리 갱신과 런타임 리소스 반영이 분리돼 있다.

## 제안
외부 파일을 가져온 직후 엔진 import를 명시적으로 호출하거나, 최소한 "파일은 복사됐지만 엔진 반영은 별도"라는 로그를 남기는 편이 낫다. Asset 창의 시각적 갱신과 런타임 리소스 상태가 어긋나면 텍스처나 JSON 테스트 때 원인 파악이 느려진다.
