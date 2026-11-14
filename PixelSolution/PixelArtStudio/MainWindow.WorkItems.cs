using Microsoft.Win32;
using System.IO;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Windows;

namespace PixelArtStudio;

public partial class MainWindow
{
    private WorkItemNote? appliedWorkItem;
    private void OpenWorkItem_Click(object sender, RoutedEventArgs e)
    {
        if (busy) return;
        var dialog = new OpenFileDialog { Filter = "일감 문서|*.md;*.txt", CheckFileExists = true };
        if (dialog.ShowDialog(this) != true) return;
        try
        {
            if (new FileInfo(dialog.FileName).Length > 1024 * 1024) throw new InvalidDataException("문서는 1MB 이하만 지원합니다.");
            WorkItemBox.Text = File.ReadAllText(dialog.FileName); WorkItemMode.SelectedIndex = 0;
            Status.Text = "문서 열기 완료 · 일감으로 생성을 누르세요. 아직 GPU는 실행되지 않았습니다.";
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException) { Fail(ex.Message); }
    }
    private bool TryApplyWorkItem()
    {
        if (busy) return false;
        try
        {
            string input = WorkItemBox.Text;
            if (string.IsNullOrWhiteSpace(input) || input.Length > 1024 * 1024) throw new InvalidDataException("일감은 비어 있지 않은 1MB 이하 문서여야 합니다.");
            var item = WorkItemMode.SelectedIndex == 0 ? WorkItemParser.ParseMarkdown(input) : WorkItemParser.Parse(input, WorkItemMode.SelectedIndex == 2);
            var r = item.Request;
            PromptBox.Text = r.Prompt;
            if (item.StylePrompt is not null) StylePromptBox.Text = item.StylePrompt;
            if (item.StyleNegativePrompt is not null) StyleNegativeBox.Text = item.StyleNegativePrompt;
            if (!item.IsPlainText)
            {
                CategoryBox.SelectedIndex = r.Category switch { "background" => 1, "tile" => 2, _ => 0 };
                SizeBox.SelectedIndex = r.Width switch { 32 => 0, 64 => 1, _ => 2 };
                NegativeBox.Text = r.NegativePrompt; StepsBox.Text = r.Steps.ToString(); SeedBox.Text = r.Seed.ToString();
                TransparentBox.IsChecked = r.Transparent; ProtectInteriorBox.IsChecked = r.ProtectInterior; StrengthSlider.Value = r.Strength;
            }
            appliedWorkItem = new WorkItemNote(WorkItemMode.SelectedIndex == 0 ? "markdown" : item.IsPlainText ? "text" : "json", "이미지 일감", input, item.AcceptanceCriteria);
            WorkItemCriteria.Text = item.AcceptanceCriteria.Length == 0 ? "검수 메모 없음" : "검수 메모 (자동 검증 아님)\n" + string.Join("\n", item.AcceptanceCriteria.Select((text, i) => $"{i + 1}. {text}"));
            Status.Text = "일감 검증 및 적용 완료";
            return true;
        }
        catch (Exception ex) when (ex is InvalidDataException or JsonException) { Fail("일감 적용 실패: " + ex.Message); return false; }
    }
    private async void GenerateWorkItem_Click(object sender, RoutedEventArgs e)
    {
        if (busy || !TryApplyWorkItem()) return;
        await GenerateAsync(false);
    }
    private void ClearWorkItemForDirectGeneration()
    {
        appliedWorkItem = null;
        WorkItemCriteria.Text = "직접 설정으로 생성 · 이전 일감 메모는 연결하지 않습니다.";
    }
    private bool ValidateWorkItemPrompt(bool edit)
    {
        if (appliedWorkItem?.Mode != "markdown") return true;
        string prompt = edit && !string.IsNullOrWhiteSpace(EditBox.Text) ? EditBox.Text : PromptBox.Text;
        if (!WorkItemParser.IsImagePrompt(prompt))
        { Fail("이미지 설명 항목에는 코드나 URL 대신 영문 그림 설명만 사용하세요. 한국어 설명은 모델 지원이 제한됩니다."); return false; }
        return true;
    }
}
internal sealed record WorkItemNote(string Mode, string Title, string Original, string[] AcceptanceCriteria);
