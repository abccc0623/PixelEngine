- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
Asset 창에서 파일 하나만 골라 지울 때와 여러 파일을 함께 지울 때의 동작이 같은지 코드 흐름을 확인했다.

## 문제
`PixelSolution/PixelTool/Tool/AssetWindow/AssetWindow.xaml.cs`에서 다중 삭제 경로 `DeleteSelectedFiles()`는 `FileSystem.DeleteFile(..., RecycleOption.SendToRecycleBin)`을 쓰지만, 단건 삭제 경로 `DeleteFile()`는 바로 `File.Delete()`를 호출한다. 같은 에디터 안에서도 어떤 메뉴를 탔는지에 따라 복구 가능 여부가 달라져 실수 복구가 어렵다.

## 제안
단건 삭제도 다중 삭제와 같은 휴지통 경로로 통일하고, 확인 문구도 "휴지통으로 이동" 기준으로 맞추는 편이 안전하다. 삭제 정책이 하나로 정리돼야 사용자가 자산 정리를 덜 불안하게 할 수 있다.
