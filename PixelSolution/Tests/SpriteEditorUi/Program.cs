using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using PixelTool;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        var folder = Directory.CreateDirectory(Path.Combine(Directory.GetCurrentDirectory(), ".sprite-tests-" + Guid.NewGuid().ToString("N"))).FullName;
        var asset = Directory.CreateDirectory(Path.Combine(folder, "Asset")).FullName;
        string path = Path.Combine(asset, "sample.sprite"), texturePath = Path.Combine(asset, "sheet.png");
        var app = new App(); app.InitializeComponent(); // Resources only: no startup, native engine or AI.
        typeof(ProjectPathService).GetProperty("ProjectRootPath")!.SetValue(null, folder);
        void Require(bool condition, string message) { if (!condition) throw new Exception(message); }
        void Invalid(SpriteRegion region)
        {
            try { region.Validate(128, 64); throw new Exception("Invalid bounds accepted."); }
            catch (InvalidDataException) { }
        }
        try
        {
            Invalid(new() { X = -1 });
            Invalid(new() { Kind = SpriteRegionKind.Animation, Width = 64, Frames = int.MaxValue });
            Invalid(new() { Kind = SpriteRegionKind.Animation, Width = 64, Frames = 3 });
            Invalid(new() { Kind = SpriteRegionKind.Animation, FrameSeconds = double.NaN });
            var document = new SpriteSheetDocument { Texture = "Asset/sheet.png", Regions = new()
            {
                new() { Name = "Icon", Width = 32, Height = 32 },
                new() { Name = "Spark", Kind = SpriteRegionKind.Animation, X = 32, Width = 16, Height = 16, Frames = 4, FrameSeconds = .1 }
            } };
            document.Validate(128, 64); document.Save(path);
            try { document.Save(texturePath); throw new Exception("Image extension was accepted for metadata."); } catch (InvalidDataException) { }
            var loaded = SpriteSheetDocument.Read(path);
            Require(loaded.Regions.Count == 2 && loaded.Regions[1].Kind == SpriteRegionKind.Animation && loaded.Regions[1].Frames == 4, "Roundtrip failed.");
            loaded.Regions.Add(loaded.Regions[0] with { Name = "icon" });
            try { loaded.Validate(128, 64); throw new Exception("Duplicate name accepted."); } catch (InvalidDataException) { }
            // A tiny synthetic sheet keeps tests independent from game assets.
            var pixels = new byte[128 * 64 * 4];
            for (int y = 0; y < 64; y++) for (int x = 0; x < 128; x++)
            {
                int offset = (y * 128 + x) * 4;
                pixels[offset] = (byte)(80 + (x / 16) * 20); pixels[offset + 1] = (byte)(120 + y);
                pixels[offset + 2] = 230; pixels[offset + 3] = 255;
            }
            var bitmap = BitmapSource.Create(128, 64, 96, 96, PixelFormats.Bgra32, null, pixels, 128 * 4);
            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
            using (var output = File.Create(texturePath)) encoder.Save(output);
            var editor = new SpriteEditor();
            T Find<T>(string name) => (T)editor.FindName(name);
            void Call(string name) => typeof(SpriteEditor).GetMethod(name, BindingFlags.NonPublic | BindingFlags.Instance)!.Invoke(editor, new object[] { editor, new RoutedEventArgs() });
            Require(editor.OpenDocument(path), "Actual editor could not open .sprite.");
            Require(Find<ListBox>("RegionList").Items.Count == 2, "Region thumbnails missing.");
            Find<ListBox>("RegionList").SelectedIndex = 1;
            Require(Find<NumberField>("XInput").Value == 32 && Find<StackPanel>("AnimationFields").Visibility == Visibility.Visible, "Selection failed to restore frame data.");
            var range = ((double Min, double Max, double Step))typeof(SpriteEditor).GetMethod("NumberRange", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(editor, new object[] { Find<NumberField>("XInput") })!;
            Require(range.Max == 64, "Coordinate drag must account for all animation frames.");
            Find<BoolField>("ShowGrid").Value = true;
            Require(Find<System.Windows.Shapes.Path>("GridLines").Data != null, "Shared grid toggle must redraw the overlay.");
            Find<NumberField>("WidthInput").Value = 16.5;
            Require(Find<Image>("PreviewImage").Source == null, "Fractional pixel edits must be rejected without throwing.");
            Find<NumberField>("WidthInput").Value = 16;
            Find<BoolField>("LoopInput").Value = false;
            Find<StringField>("NameInput").Text = "SparkEdited";
            Require(editor.Tag.ToString()!.EndsWith("*"), "Dirty title missing.");
            Require(editor.SaveDocument(false), "Saving edited data failed.");
            Require(SpriteSheetDocument.Read(path).Regions[1].Name == "SparkEdited", "Saved draft was lost.");
            Require(!SpriteSheetDocument.Read(path).Regions[1].Loop, "Shared loop toggle must persist.");
            Call("DuplicateRegion");
            Require(Find<ListBox>("RegionList").Items.Count == 3, "Duplicate failed.");
            Call("DeleteRegion");
            Require(Find<ListBox>("RegionList").Items.Count == 2, "Delete failed.");
            Require(editor.SaveDocument(false), "Save after delete failed.");
            var reopened = new SpriteEditor(); Require(reopened.OpenDocument(path), "Reopening failed.");
            Require(Find<Image>("SheetImage").Width == 128, "Sheet must use pixel coordinates, independent of image DPI.");
            Find<ListBox>("RegionList").SelectedIndex = 1;
            foreach (double width in new[] { 900d, 1180d }) { editor.Measure(new Size(width, 760)); editor.Arrange(new Rect(0, 0, width, 760)); editor.UpdateLayout(); }
            foreach (string field in new[] { "XInput", "YInput", "WidthInput", "HeightInput", "CountInput", "SecondsInput" })
            {
                var input = Find<NumberField>(field);
                Require(input.LabelWidth == 200 && input.ActualHeight == 24, "Shared numeric fields must keep aligned labels and fixed height.");
            }
            if (args.Length > 0)
            {
                var screenshot = new RenderTargetBitmap(1180, 760, 96, 96, PixelFormats.Pbgra32); screenshot.Render(editor);
                var png = new PngBitmapEncoder(); png.Frames.Add(BitmapFrame.Create(screenshot));
                using var output = File.Create(args[0]); png.Save(output);
            }
            Console.WriteLine("PASS: sprite file/editor roundtrip, layout, shared numeric/string/toggle fields and image bounds. No native engine, game or AI started.");
        }
        finally
        {
            File.Delete(path); File.Delete(texturePath); Directory.Delete(asset); Directory.Delete(folder);
        }
    }
}
