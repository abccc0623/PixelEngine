using System.Reflection;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Data;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        try { Run(args); }
        catch (Exception ex) { Console.Error.WriteLine(ex); Environment.Exit(1); }
    }

    private static void Run(string[] args)
    {
        var assembly = Assembly.LoadFrom(Path.GetFullPath(args[0]));
        var app = (Application)Activator.CreateInstance(assembly.GetType("PixelTool.App")!)!;
        app.GetType().GetMethod("InitializeComponent")!.Invoke(app, null); // Resources only, no editor startup.
        object Make(string name) => Activator.CreateInstance(assembly.GetType("PixelTool." + name)!)!;
        void Check(bool condition, string name) { if (!condition) throw new Exception(name); Console.WriteLine("PASS " + name); }
        var toolbar = (UserControl)Make("ToolbarWindow");
        var changeProject = (Button)toolbar.FindName("ChangeProjectButton");
        Check(changeProject != null && changeProject.ToolTip.ToString()!.Contains("프로젝트 변경"), "toolbar exposes the project-change button");
        var restart = (System.Diagnostics.ProcessStartInfo)app.GetType().GetMethod("CreateProjectRestart", BindingFlags.Static | BindingFlags.NonPublic)!.Invoke(null, null)!;
        Check(restart.FileName == Environment.ProcessPath && restart.WorkingDirectory == AppContext.BaseDirectory && !restart.UseShellExecute, "project restart uses the current executable and installation directory");
        if (Path.GetFileNameWithoutExtension(Environment.ProcessPath)!.Equals("dotnet", StringComparison.OrdinalIgnoreCase))
            Check(restart.ArgumentList.Count == 1 && restart.ArgumentList[0] == Assembly.GetEntryAssembly()!.Location, "dotnet-hosted restart preserves the entry assembly");
        IEnumerable<T> Descendants<T>(DependencyObject root) where T : DependencyObject
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++)
            {
                var child = VisualTreeHelper.GetChild(root, i);
                if (child is T match) yield return match;
                foreach (var nested in Descendants<T>(child)) yield return nested;
            }
        }
        var genericScroll = new ScrollViewer { Width = 180, Height = 90, HorizontalScrollBarVisibility = ScrollBarVisibility.Auto, VerticalScrollBarVisibility = ScrollBarVisibility.Auto, Content = new Border { Width = 600, Height = 500 } };
        var textScroll = new TextBox { Width = 180, Height = 90, AcceptsReturn = true, VerticalScrollBarVisibility = ScrollBarVisibility.Auto, Text = string.Join("\n", Enumerable.Repeat(new string('W', 100), 50)) };
        var listScroll = new ListBox { Width = 180, Height = 90, ItemsSource = Enumerable.Range(0, 100).Select(i => "Item " + i) };
        var treeScroll = new TreeView { Width = 180, Height = 90 };
        foreach (int i in Enumerable.Range(0, 40)) treeScroll.Items.Add(new TreeViewItem { Header = "Folder " + i });
        var avalonAssembly = Assembly.LoadFrom(Path.Combine(Path.GetDirectoryName(Path.GetFullPath(args[0]))!, "ICSharpCode.AvalonEdit.dll"));
        var luaEditor = (FrameworkElement)Activator.CreateInstance(avalonAssembly.GetType("ICSharpCode.AvalonEdit.TextEditor")!)!;
        luaEditor.Width = 180; luaEditor.Height = 90;
        luaEditor.GetType().GetProperty("Text")!.SetValue(luaEditor, string.Join("\n", Enumerable.Repeat(new string('W', 100), 50)));
        foreach (var control in new FrameworkElement[] { genericScroll, textScroll, listScroll, treeScroll, luaEditor })
        {
            var host = new Border { Child = control };
            host.Measure(new Size(180, 90)); host.Arrange(new Rect(0, 0, 180, 90)); host.UpdateLayout();
            foreach (var nested in Descendants<Control>(control).ToArray()) nested.ApplyTemplate();
            control.UpdateLayout();
            var bars = Descendants<ScrollBar>(control).ToArray();
            Check(bars.Length > 0, control.GetType().Name + " has scrollbars");
            foreach (var bar in bars)
            {
                string key = bar.Orientation == Orientation.Vertical ? "PixelVerticalScrollBar" : "PixelHorizontalScrollBar";
                Check(ReferenceEquals(bar.Template, app.FindResource(key)), control.GetType().Name + " uses global " + bar.Orientation + " theme");
            }
        }
        genericScroll.ScrollToBottom(); genericScroll.UpdateLayout();
        Check(genericScroll.VerticalOffset > 0, "default ScrollViewer keeps scrolling behavior");
        textScroll.ScrollToEnd(); textScroll.UpdateLayout();
        Check(textScroll.VerticalOffset > 0, "TextBox keeps multiline scrolling behavior");
        var stack = new StackPanel { Width = 480 };
        var fields = new Dictionary<string, object>();
        foreach (string name in new[] { "NumberField", "BoolField", "DropdownField", "ColorField", "AssetField" })
        {
            object field = Make(name); fields[name] = field;
            stack.Children.Add((UIElement)field);
        }
        stack.Measure(new Size(480, 500)); stack.Arrange(new Rect(0, 0, 480, 500)); stack.UpdateLayout();
        foreach (var pair in fields)
        {
            var field = (UserControl)pair.Value;
            Check(((Grid)field.Content).ColumnDefinitions[0].Width.Value == 200, pair.Key + " label width");
            Check(field.ActualHeight == 24, pair.Key + " height");
        }
        var boolField = fields["BoolField"];
        var boolGrid = (Grid)((UserControl)boolField).Content;
        var toggle = (ToggleButton)((ContentControl)((Border)boolGrid.Children[1]).Child).Content;
        toggle.IsChecked = true;
        Check((bool)boolField.GetType().GetProperty("Value")!.GetValue(boolField)!, "toggle writes Value");
        var dropdown = fields["DropdownField"];
        dropdown.GetType().GetProperty("ItemsSource")!.SetValue(dropdown, new[] { "First", "Second" });
        dropdown.GetType().GetProperty("SelectedItem")!.SetValue(dropdown, "Second");
        Check((string)dropdown.GetType().GetProperty("SelectedItem")!.GetValue(dropdown)! == "Second", "dropdown selection");
        var color = fields["ColorField"];
        color.GetType().GetProperty("Value")!.SetValue(color, Color.FromArgb(128, 255, 20, 0));
        Check(((Color)color.GetType().GetProperty("Value")!.GetValue(color)!).A == 128, "RGBA alpha");
        var colorControl = (UserControl)color;
        var popup = ((Grid)colorControl.Content).Children.OfType<Popup>().Single();
        var colorPanel = (Border)popup.Child;
        var rows = (StackPanel)colorPanel.Child;
        var palette = rows.Children.OfType<UniformGrid>().Single();
        Check(palette.Children.Count == 40, "40 clickable palette colors");
        var colorProperty = (DependencyProperty)color.GetType().GetField("ValueProperty")!.GetValue(null)!;
        var source = new Border { Tag = Color.FromArgb(128, 1, 2, 3) };
        BindingOperations.SetBinding(colorControl, colorProperty, new Binding("Tag") { Source = source, Mode = BindingMode.TwoWay });
        var redButton = (Button)palette.Children[24];
        redButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        Check((Color)source.Tag == Color.FromArgb(128, 244, 67, 54), "palette preserves alpha and two-way binding");
        var selectedBorder = (Border)redButton.Content;
        Check(((SolidColorBrush)selectedBorder.BorderBrush).Color == ((SolidColorBrush)app.FindResource("PixelAccentBrush")).Color, "selected palette color is highlighted");
        var redInput = rows.Children.OfType<UserControl>().First();
        redInput.GetType().GetProperty("Value")!.SetValue(redInput, 10d);
        Check(((Color)source.Tag).R == 10 && ((Color)source.Tag).A == 128, "RGBA editing remains synchronized");
        Check(rows.Children.OfType<Button>().Any(b => b.Content is string label && label.Contains("스포이드")), "screen eyedropper entry exists");
        var pickerType = assembly.GetType("PixelTool.ScreenColorPicker", true)!;
        var sampleMethod = pickerType.GetMethod("Sample", BindingFlags.NonPublic | BindingFlags.Static)!;
        var pixels = BitmapSource.Create(2, 1, 96, 96, PixelFormats.Bgra32, null, new byte[] { 56, 34, 12, 255, 3, 2, 1, 255 }, 8);
        Check((Color)sampleMethod.Invoke(null, new object[] { pixels, 0, 0, (byte)64 })! == Color.FromArgb(64, 12, 34, 56), "screen sample reads RGB and retains caller alpha");
        Check(sampleMethod.Invoke(null, new object[] { pixels, -1, 0, (byte)255 }) == null && sampleMethod.Invoke(null, new object[] { pixels, 2, 0, (byte)255 }) == null, "out-of-screen sampling is rejected");
        if (args.Length > 1)
        {
            colorPanel.Measure(new Size(300, 500)); colorPanel.Arrange(new Rect(0, 0, colorPanel.DesiredSize.Width, colorPanel.DesiredSize.Height)); colorPanel.UpdateLayout();
            var image = new RenderTargetBitmap((int)Math.Ceiling(colorPanel.ActualWidth), (int)Math.Ceiling(colorPanel.ActualHeight), 96, 96, PixelFormats.Pbgra32);
            image.Render(colorPanel);
            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(image));
            using var stream = File.Create(Path.ChangeExtension(args[1], ".palette.png")); encoder.Save(stream);
        }
        var asset = fields["AssetField"];
        Check(!(bool)asset.GetType().GetMethod("TrySetAsset")!.Invoke(asset, new object[] { "../outside.png" })!, "asset rejects invalid path");
        string scratch = Path.Combine(Environment.CurrentDirectory, ".basic-fields-" + Guid.NewGuid().ToString("N"));
        var project = assembly.GetType("PixelTool.ProjectPathService")!.GetProperty("ProjectRootPath")!;
        object? oldRoot = project.GetValue(null);
        try
        {
            Directory.CreateDirectory(Path.Combine(scratch, "Asset"));
            string sample = Path.Combine(scratch, "Asset", "sample.txt");
            File.WriteAllText(sample, "test");
            project.GetSetMethod(true)!.Invoke(null, new object[] { scratch });
            Check((bool)asset.GetType().GetMethod("TrySetAsset")!.Invoke(asset, new object[] { sample })!, "asset accepts project file");
            Check((string)asset.GetType().GetProperty("AssetPath")!.GetValue(asset)! == "Asset/sample.txt", "asset relative path");
            asset.GetType().GetProperty("AllowedExtensions")!.SetValue(asset, ".png");
            Check(!(bool)asset.GetType().GetMethod("TrySetAsset")!.Invoke(asset, new object[] { sample })!, "asset extension filter");
            File.Delete(sample);
        }
        finally
        {
            project.GetSetMethod(true)!.Invoke(null, new[] { oldRoot });
            Directory.Delete(Path.Combine(scratch, "Asset")); Directory.Delete(scratch);
        }
        if (args.Length > 1)
        {
            stack.UpdateLayout();
            var bitmap = new RenderTargetBitmap(480, 120, 96, 96, PixelFormats.Pbgra32);
            bitmap.Render(stack);
            var png = new PngBitmapEncoder(); png.Frames.Add(BitmapFrame.Create(bitmap));
            using var stream = File.Create(Path.GetFullPath(args[1]));
            png.Save(stream);
        }
        Console.WriteLine("Basic field tests passed.");
    }
}
