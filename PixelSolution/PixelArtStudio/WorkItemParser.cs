using System.IO;
using System.Text.Json;

namespace PixelArtStudio;

internal sealed record ParsedWorkItem(GenerationRequest Request, string[] AcceptanceCriteria, bool IsPlainText, string? StylePrompt = null, string? StyleNegativePrompt = null);

// Local parsing only: no model call, file access, translation, or execution.
internal static class WorkItemParser
{
    internal static ParsedWorkItem Parse(string input, bool jsonMode)
    {
        if (string.IsNullOrWhiteSpace(input)) throw new InvalidDataException("일감을 입력하세요.");
        if (input.Length > 65536) throw new InvalidDataException("일감은 65,536자 이하만 지원합니다.");
        if (!jsonMode && input.Length > 4000) throw new InvalidDataException("이미지 설명은 4000자 이하만 지원합니다.");
        if (!jsonMode) return new(new GenerationRequest { Prompt = input }, Array.Empty<string>(), true);
        using var doc = JsonDocument.Parse(input, new JsonDocumentOptions { MaxDepth = 8 });
        if (doc.RootElement.ValueKind != JsonValueKind.Object) throw new InvalidDataException("JSON 일감은 객체여야 합니다.");
        var request = new GenerationRequest(); string? stylePrompt = null, styleNegative = null;
        var criteria = new List<string>();
        var seen = new HashSet<string>(StringComparer.Ordinal);
        foreach (var property in doc.RootElement.EnumerateObject())
        {
            if (!seen.Add(property.Name)) throw new InvalidDataException("중복 필드: " + property.Name);
            var value = property.Value;
            switch (property.Name)
            {
                case "prompt": request.Prompt = String(value, property.Name); break;
                case "category": request.Category = String(value, property.Name); break;
                case "stylePrompt": stylePrompt = String(value, property.Name); if (stylePrompt.Length > 2000) throw new InvalidDataException("stylePrompt는 2000자 이하입니다."); break;
                case "styleNegativePrompt": styleNegative = String(value, property.Name); if (styleNegative.Length > 2000) throw new InvalidDataException("styleNegativePrompt는 2000자 이하입니다."); break;
                case "negativePrompt": request.NegativePrompt = String(value, property.Name); break;
                case "width": request.Width = Integer(value, property.Name); break;
                case "height": request.Height = Integer(value, property.Name); break;
                case "steps": request.Steps = Integer(value, property.Name); break;
                case "seed":
                    if (value.ValueKind != JsonValueKind.Number || !value.TryGetInt64(out long seed)) throw new InvalidDataException("seed는 정수여야 합니다.");
                    request.Seed = seed; break;
                case "strength":
                    if (value.ValueKind != JsonValueKind.Number || !value.TryGetDouble(out double strength) || !double.IsFinite(strength)) throw new InvalidDataException("strength는 유한한 숫자여야 합니다.");
                    request.Strength = strength; break;
                case "transparent": request.Transparent = Boolean(value, property.Name); break;
                case "protectInterior": request.ProtectInterior = Boolean(value, property.Name); break;
                case "acceptanceCriteria":
                    if (value.ValueKind != JsonValueKind.Array) throw new InvalidDataException("acceptanceCriteria는 문자열 배열이어야 합니다.");
                    foreach (var item in value.EnumerateArray()) criteria.Add(String(item, property.Name));
                    break;
                case "outputPath": throw new InvalidDataException("일감의 outputPath는 지원하지 않습니다. 생성 후 PNG 내보내기로 저장 위치를 선택하세요.");
                default: throw new InvalidDataException("지원하지 않는 필드: " + property.Name);
            }
        }
        if (!seen.Contains("prompt") || string.IsNullOrWhiteSpace(request.Prompt)) throw new InvalidDataException("비어 있지 않은 prompt가 필요합니다.");
        if (request.Prompt.Length > 4000) throw new InvalidDataException("prompt는 4000자 이하만 지원합니다.");
        if (request.Category is not ("character" or "background" or "tile")) throw new InvalidDataException("category는 character, background, tile 중 하나여야 합니다.");
        if (request.Width != request.Height || request.Width is not (32 or 64 or 128)) throw new InvalidDataException("width와 height는 같은 32, 64, 128 값이어야 합니다. 크기를 바꾸려면 둘 다 지정하세요.");
        if (request.Steps is < 4 or > 50) throw new InvalidDataException("steps 범위는 4~50입니다.");
        if (request.Seed < -1 || request.Seed > uint.MaxValue) throw new InvalidDataException("seed 범위는 -1~4294967295입니다.");
        if (request.Strength < .1 || request.Strength > 1) throw new InvalidDataException("strength 범위는 0.1~1입니다.");
        return new(request, criteria.ToArray(), false, stylePrompt, styleNegative);
    }
    internal static ParsedWorkItem ParseMarkdown(string input)
    {
        if (string.IsNullOrWhiteSpace(input) || input.Length > 1024 * 1024) throw new InvalidDataException("일감 문서 크기가 올바르지 않습니다.");
        var r = new GenerationRequest(); var seen = new HashSet<string>(); var criteria = new List<string>();
        string section = ""; char fence = (char)0; int fenceLength = 0;
        foreach (string raw in input.Split('\n'))
        {
            string line = raw.Trim();
            int run = 0;
            if (line.Length > 0 && line[0] is '`' or '~') while (run < line.Length && line[run] == line[0]) run++;
            if (fence != (char)0)
            {
                if (run >= fenceLength && line[0] == fence && string.IsNullOrWhiteSpace(line[run..])) { fence = (char)0; fenceLength = 0; }
                continue;
            }
            if (run >= 3) { fence = line[0]; fenceLength = run; continue; }
            if (line.StartsWith("## ")) { section = line[3..].Trim(); continue; }
            if (line.StartsWith("#")) { section = ""; continue; }
            if (section == "완료 확인" && line.StartsWith("- ")) { criteria.Add(line[2..]); continue; }
            if (section != "내용" || !line.StartsWith("- ")) continue;
            int colon = line.IndexOf(':'); if (colon < 0) continue;
            string key = line[2..colon].Trim(), value = line[(colon + 1)..].Trim();
            if (key is not ("리소스 종류" or "이미지 설명" or "출력 크기" or "배경" or "시드")) continue;
            if (!seen.Add(key)) throw new InvalidDataException("중복 일감 항목: " + key);
            switch (key)
            {
                case "리소스 종류": r.Category = value switch { "캐릭터" => "character", "배경" => "background", "타일" => "tile", _ => throw new InvalidDataException("리소스 종류는 캐릭터/배경/타일이어야 합니다.") }; break;
                case "이미지 설명": r.Prompt = value; break;
                case "출력 크기": r.Width = r.Height = value switch { "32x32" => 32, "64x64" => 64, "128x128" => 128, _ => throw new InvalidDataException("출력 크기는 32x32/64x64/128x128이어야 합니다.") }; break;
                case "배경": r.Transparent = value switch { "투명" => true, "불투명" => false, _ => throw new InvalidDataException("배경은 투명/불투명이어야 합니다.") }; break;
                case "시드": if (!long.TryParse(value, out long seed) || seed < -1 || seed > uint.MaxValue) throw new InvalidDataException("시드 범위는 -1~4294967295입니다."); r.Seed = seed; break;
            }
        }
        if (!IsImagePrompt(r.Prompt)) throw new InvalidDataException("## 내용 아래 '- 이미지 설명: 영문 이미지 설명'을 1~4000자로 입력하세요. 코드/URL/한국어 설명은 지원하지 않습니다.");
        return new(r, criteria.ToArray(), false);
    }
    internal static bool IsImagePrompt(string prompt) => !string.IsNullOrWhiteSpace(prompt) && prompt.Length <= 4000 && System.Text.RegularExpressions.Regex.IsMatch(prompt, "[A-Za-z]") && !System.Text.RegularExpressions.Regex.IsMatch(prompt, @"[가-힣ㄱ-ㅎㅏ-ㅣ]|```|(?i:https?://|www\.)|\b(function|class|local|return)\s+\w+|\b\w+\s*\([^)]*\)\s*[{;]");
    private static string String(JsonElement value, string name) => value.ValueKind == JsonValueKind.String ? value.GetString()! : throw new InvalidDataException(name + "은 문자열이어야 합니다.");
    private static int Integer(JsonElement value, string name) => value.ValueKind == JsonValueKind.Number && value.TryGetInt32(out int result) ? result : throw new InvalidDataException(name + "은 정수여야 합니다.");
    private static bool Boolean(JsonElement value, string name) => value.ValueKind switch { JsonValueKind.True => true, JsonValueKind.False => false, _ => throw new InvalidDataException(name + "은 true 또는 false여야 합니다.") };
}
