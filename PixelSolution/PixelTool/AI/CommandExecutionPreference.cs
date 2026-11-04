namespace PixelTool;

// This is an assistant instruction, not an OS or app-server security boundary.
internal static class CommandExecutionPreference
{
    internal static string ForTurn(bool confirmCommands)
    {
        string mode = confirmCommands
            ? "작업 승인 모드: 항상 확인 요청. 읽기 전용 조회를 포함해 작업의 내용과 목적을 먼저 대화로 설명하고 명시적 승인을 받은 뒤 실행한다. 아직 승인받지 않은 조회는 제안만 하고 응답을 끝내 승인 답변을 기다린다. 파일 생성·수정 승인은 에디터 승인창으로 받으므로 대화로 같은 승인을 중복 질문하지 않는다."
            : "작업 승인 모드: 일반 작업 자동 승인. 사용자가 요청한 작업에 필요한 조회와 파일 생성·수정 및 Asset 내부 파일·폴더 생성·이동·이름 변경·휴지통 삭제는 허용된 권한 범위에서 별도 질문 없이 실행한다. 파일 수정 허용이 켜져 있다면 이 작업의 승인을 다시 요구하거나 승인을 기다리며 응답을 끝내지 않는다.";
        return "[PixelTool 이번 메시지의 실행 설정]\n" + mode +
            "\nAsset 내부 파일·폴더 작업은 delete_asset_file, delete_asset_folder, move_asset, create_asset_folder 등 제공된 도구와 작업 승인 설정을 따른다. 자동 승인에서는 추가 확인 없이 진행한다. Asset 자체·Engine·Editor·프로젝트 외부로 이동·삭제하거나 영구 삭제할 수 없다. 프로그램 설치, 커밋, 푸시는 정확한 대상과 영향을 알리고 이번 작업에 대한 별도 명시적 승인을 받아야 한다. " +
            "자동 실행은 사용자 요청 밖의 작업, 파일 수정 허용 해제, 운영 규칙, 파일 접근 범위 또는 서버의 승인 요구를 우회할 권한을 주지 않는다. " +
            "승인은 해당 명령에 한정되며 이전 대화의 승인을 다른 명령에 재사용하지 않는다.";
    }
}
