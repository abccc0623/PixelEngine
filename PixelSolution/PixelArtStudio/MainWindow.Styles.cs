using Microsoft.Win32;
using System.IO;
using System.Text.Json;
using System.Windows;
namespace PixelArtStudio;
public partial class MainWindow
{
    private void SaveStyle_Click(object sender, RoutedEventArgs e)
    {
        var d = new SaveFileDialog { Filter = "스타일 JSON|*.json", FileName = "art-style.json", OverwritePrompt = true };
        if (d.ShowDialog(this) != true) return;
        try { if (StylePromptBox.Text.Length > 2000 || StyleNegativeBox.Text.Length > 2000) throw new InvalidDataException("스타일 설명은 각각 2000자 이하입니다."); File.WriteAllText(d.FileName, JsonSerializer.Serialize(new ArtStyle(StylePromptBox.Text, StyleNegativeBox.Text), JsonOptions)); Status.Text = "스타일 저장 완료"; }
        catch (Exception ex) { Fail(ex.Message); }
    }
    private void LoadStyle_Click(object sender, RoutedEventArgs e)
    {
        var d = new OpenFileDialog { Filter = "스타일 JSON|*.json" };
        if (d.ShowDialog(this) != true) return;
        try
        {
            if (new FileInfo(d.FileName).Length > 32768) throw new InvalidDataException("스타일 파일은 32KB 이하만 지원합니다.");
            var style = JsonSerializer.Deserialize<ArtStyle>(File.ReadAllText(d.FileName), new JsonSerializerOptions { PropertyNameCaseInsensitive = true }) ?? throw new InvalidDataException("빈 스타일 파일입니다.");
            if (style.StylePrompt is null || style.StyleNegativePrompt is null || style.StylePrompt.Length > 2000 || style.StyleNegativePrompt.Length > 2000) throw new InvalidDataException("스타일 설명은 각각 2000자 이하 문자열이어야 합니다.");
            StylePromptBox.Text = style.StylePrompt; StyleNegativeBox.Text = style.StyleNegativePrompt; Status.Text = "스타일 불러오기 완료";
        }
        catch (Exception ex) { Fail(ex.Message); }
    }
}
internal sealed record ArtStyle(string StylePrompt, string StyleNegativePrompt);
