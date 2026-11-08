using System.IO;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace PixelTool;

public enum SpriteRegionKind { Sprite, Animation }

public sealed record SpriteRegion
{
    public string Name { get; set; } = "Sprite";
    public SpriteRegionKind Kind { get; set; }
    public int X { get; set; }
    public int Y { get; set; }
    public int Width { get; set; } = 32;
    public int Height { get; set; } = 32;
    public int Frames { get; set; } = 1;
    public double FrameSeconds { get; set; } = .1;
    public bool Loop { get; set; } = true;

    public void Validate(int imageWidth, int imageHeight)
    {
        if (string.IsNullOrWhiteSpace(Name) || Name.Length > 100)
            throw new InvalidDataException("이름은 1~100자로 입력하세요.");
        if (!Enum.IsDefined(Kind) || X < 0 || Y < 0 || Width < 1 || Height < 1 || Frames is < 1 or > 1024 ||
            (Kind == SpriteRegionKind.Sprite && Frames != 1) ||
            !double.IsFinite(FrameSeconds) || FrameSeconds < .001 || FrameSeconds > 60)
            throw new InvalidDataException("좌표·크기·프레임 수 또는 프레임 시간을 확인하세요.");
        if ((long)X + (long)Width * Frames > imageWidth || (long)Y + Height > imageHeight)
            throw new InvalidDataException("선택 영역이 원본 이미지 범위를 벗어납니다. 애니메이션은 가로 한 줄로 지정하세요.");
    }
}

public sealed class SpriteSheetDocument
{
    public int Version { get; set; } = 1;
    public string Texture { get; set; } = "";
    public List<SpriteRegion> Regions { get; set; } = new();
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true, PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        Converters = { new JsonStringEnumConverter() }
    };

    public static SpriteSheetDocument Read(string path)
    {
        if (new FileInfo(path).Length > 1024 * 1024) throw new InvalidDataException("스프라이트 파일은 1MB 이하여야 합니다.");
        var document = JsonSerializer.Deserialize<SpriteSheetDocument>(File.ReadAllText(path), JsonOptions)
            ?? throw new InvalidDataException("스프라이트 파일을 읽을 수 없습니다.");
        if (document.Version != 1 || string.IsNullOrWhiteSpace(document.Texture) || document.Regions == null ||
            document.Regions.Count > 1024 || document.Regions.Any(r => r == null))
            throw new InvalidDataException("지원하지 않거나 올바르지 않은 스프라이트 파일입니다.");
        return document;
    }

    public void Validate(int width, int height)
    {
        var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        if (Regions.Count > 1024) throw new InvalidDataException("파일당 최대 1024개 항목을 저장할 수 있습니다.");
        foreach (var region in Regions)
        {
            region.Validate(width, height);
            if (!names.Add(region.Name)) throw new InvalidDataException("항목 이름이 중복됩니다: " + region.Name);
        }
    }

    public void Save(string path)
    {
        if (!Path.GetExtension(path).Equals(".sprite", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("스프라이트 데이터는 .sprite 확장자로 저장하세요. 원본 이미지에는 저장할 수 없습니다.");
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            File.WriteAllText(temporary, JsonSerializer.Serialize(this, JsonOptions));
            File.Move(temporary, path, true);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
    }
}
