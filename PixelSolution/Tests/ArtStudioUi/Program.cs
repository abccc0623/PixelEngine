using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        string folder = Path.Combine(Directory.GetCurrentDirectory(), ".art-fields-tests-" + Guid.NewGuid().ToString("N"));
        string asset = Directory.CreateDirectory(Path.Combine(folder, "Asset")).FullName;
        string nested = Directory.CreateDirectory(Path.Combine(asset, "Nested")).FullName;
        string referencePath = Path.Combine(asset, "reference.png"), resultPath = Path.Combine(asset, "result.png");
        string[] fixtures = { Path.Combine(asset, "default.png"), Path.Combine(asset, "default_1.png"), Path.Combine(nested, "Default_2.PNG"), referencePath, resultPath };
        foreach (string fixture in fixtures.Take(3)) File.WriteAllBytes(fixture, new byte[] { 0 });
        try
        {
        var assembly = Assembly.LoadFrom(Path.GetFullPath(args[0]));
        var artType = assembly.GetType("PixelTool.ArtStudio", true)!;
        assembly.GetType("PixelTool.ProjectPathService", true)!.GetProperty("ProjectRootPath")!.SetValue(null, folder);
        var view = (UserControl)Activator.CreateInstance(artType)!;
        view.Resources.MergedDictionaries.Add((ResourceDictionary)Activator.CreateInstance(assembly.GetType("PixelTool.EditorColors", true)!)!);
        T Find<T>(string name) => (T)view.FindName(name);
        var sizeInput = Find<UserControl>("ImageSizeInput");
        var sizeGrid = (Grid)sizeInput.Content;
        var xAxis = (UserControl)sizeGrid.Children[1];
        var yAxis = (UserControl)sizeGrid.Children[2];
        var widthInput = (TextBox)xAxis.FindName("InputBox");
        var heightInput = (TextBox)yAxis.FindName("InputBox");
        var nameField = Find<UserControl>("ImageNameInput");
        var nameInput = (TextBox)nameField.FindName("InputBox");
        void Require(bool condition, string message) { if (!condition) throw new Exception(message); }
        Require(Find<RadioButton>("Mode2D").IsChecked == true && Find<Border>("CategoryPanel").Visibility == Visibility.Visible, "Art Studio must open in 2D mode.");
        Find<RadioButton>("Mode2D").IsChecked = true;
        Require(Find<Border>("CategoryPanel").Visibility == Visibility.Visible && Find<Border>("DetailsPanel").Visibility == Visibility.Visible, "2D must show categories and the settings region.");
        var list = Find<ListBox>("ResourceList");
        Require(list.Items.Count == 5 && list.SelectedIndex == 0, "Item icons must be selected initially.");
        Require(Find<StackPanel>("CreationFields").Visibility == Visibility.Visible, "Item creation fields must be visible initially.");
        Require(nameInput.Text == "default_3.png", "Initial filename must avoid existing files.");
        Require(view.FindName("ItemSubject") != null && view.FindName("CustomRequest") != null && view.FindName("ReferencePreview") != null, "Item questions, custom request and reference preview must exist.");
        Require(!Find<Button>("GenerateButton").IsEnabled, "Generation must require a reference and subject.");
        Require(widthInput.Text == "64" && heightInput.Text == "64" && !(bool)Find<UserControl>("AutoSize").GetType().GetProperty("Value")!.GetValue(Find<UserControl>("AutoSize"))!, "Size defaults must be 64 by 64 with Auto off.");
        var unique = artType.GetMethod("UniqueImageName", BindingFlags.NonPublic | BindingFlags.Static)!;
        string Name(string input) => (string)unique.Invoke(null, new object[] { input, asset })!;
        Require(Name("default.png") == "default_3.png", "Numbering must check Asset subfolders and filenames case-insensitively.");
        Require(Name("default_1.png") == "default_3.png" && Name("flame") == "flame.png", "Numbering must avoid nested suffixes and append .png when omitted.");
        try { Name("../outside.png"); throw new Exception("Paths were accepted as filenames."); }
        catch (TargetInvocationException ex) when (ex.InnerException is ArgumentException) { }
        foreach (var item in list.Items)
        {
            list.SelectedItem = item;
            Require(view.FindName("ResourceTitle") == null && view.FindName("QuestionsNotice") == null, "Selected categories must not be repeated as headings or instructions.");
            Require(Find<StackPanel>("CreationFields").Visibility == Visibility.Visible, "Every resource type must show common fields.");
            Require(Find<Grid>("ImageNameRow").Visibility == (list.SelectedIndex == 0 ? Visibility.Visible : Visibility.Collapsed), "Only the icon tab should show its name field.");
            Require(Find<ContentControl>("QuestionsHost").Content == null, "No unrequested category-specific questions may be added.");
        }
        Require(nameInput.Text == "default_3.png" && Find<TextBlock>("FieldError").Visibility == Visibility.Collapsed, "Icon selection must choose an unused name automatically.");
        nameInput.Text = "default";
        nameField.GetType().GetMethod("InputFinished", BindingFlags.NonPublic | BindingFlags.Instance)!.Invoke(nameField,
            new object[] { nameInput, new System.Windows.Input.KeyboardFocusChangedEventArgs(System.Windows.Input.Keyboard.PrimaryDevice, 0, nameInput, null) });
        Require(nameInput.Text == "default_3.png", "String-field commit must retain image filename numbering.");
        // Build nested UserControl templates before checking inherited enabled state.
        view.Measure(new Size(1060, 680)); view.Arrange(new Rect(0, 0, 1060, 680)); view.UpdateLayout();
        widthInput.Text = "128";
        Find<UserControl>("AutoSize").GetType().GetProperty("Value")!.SetValue(Find<UserControl>("AutoSize"), true);
        Require(!widthInput.IsEnabled && !heightInput.IsEnabled, "Auto must disable both dimensions.");
        Find<UserControl>("AutoSize").GetType().GetProperty("Value")!.SetValue(Find<UserControl>("AutoSize"), false);
        Require(widthInput.IsEnabled && heightInput.IsEnabled && widthInput.Text == "128", "Disabling Auto must restore editing without losing size values.");
        Find<RadioButton>("Mode3D").IsChecked = true;
        Require(Find<RadioButton>("Mode2D").IsChecked == false, "Dimension selection must be exclusive.");
        Require(Find<Border>("CategoryPanel").Visibility == Visibility.Collapsed && Find<Border>("DetailsPanel").Visibility == Visibility.Collapsed, "3D must hide category and editing controls.");
        Require(Find<Border>("ModeNoticePanel").Visibility == Visibility.Visible && Find<TextBlock>("ModeNoticeTitle").Text == "준비 중입니다" && view.FindName("ModeNoticeHint") == null, "3D must show only the coming-soon notice.");
        Find<RadioButton>("Mode2D").IsChecked = true;
        Require(list.SelectedIndex == 4 && Find<Border>("DetailsPanel").Visibility == Visibility.Visible, "Returning to 2D must restore the selection.");
        foreach (double width in new[] { 400d, 1060d })
        {
            view.Measure(new Size(width, 680)); view.Arrange(new Rect(0, 0, width, 680)); view.UpdateLayout();
            Require(view.DesiredSize.Width <= width, "Narrow docks must scroll rather than grow outside the pane.");
        }
        var scroll = Find<ScrollViewer>("FieldsScroll");
        var mode2D = Find<RadioButton>("Mode2D");
        var mode3D = Find<RadioButton>("Mode3D");
        Require(mode2D.ActualHeight == 36 && mode3D.ActualHeight == 36, "Dimension tabs must match resource tab heights.");
        Require(((FrameworkElement)mode2D.Template.FindName("SelectedDot", mode2D)).Visibility == Visibility.Visible &&
            ((FrameworkElement)mode3D.Template.FindName("SelectedDot", mode3D)).Visibility == Visibility.Hidden, "Only the selected dimension tab must show a dot.");
        for (int i = 0; i < list.Items.Count; i++)
        {
            var container = (ListBoxItem)list.ItemContainerGenerator.ContainerFromIndex(i);
            Require(container.ActualHeight == 36, "Resource tabs must be half-height compact rows.");
            var dot = (System.Windows.Shapes.Ellipse)container.Template.FindName("SelectedDot", container);
            Require(dot.Visibility == (i == list.SelectedIndex ? Visibility.Visible : Visibility.Hidden), "Only the selected resource tab must show a dot.");
            Require(((SolidColorBrush)dot.Fill).Color == ((SolidColorBrush)view.FindResource("PixelAccentBrush")).Color, "Selection dots must use the editor yellow accent.");
        }
        Require(scroll.ExtentWidth <= scroll.ViewportWidth + 1, "The three panels must fit a normal-width window without horizontal scrolling.");
        Require(view.FindName("LayoutScroll") == null && ReferenceEquals(scroll.Parent, Find<Border>("DetailsPanel")), "Only the right fields panel may own the page scroll.");
        list.SelectedIndex = 0;
        view.Measure(new Size(1060, 480)); view.Arrange(new Rect(0, 0, 1060, 480)); view.UpdateLayout();
        Point modePosition = mode2D.TranslatePoint(new Point(), view);
        Point categoryPosition = list.TranslatePoint(new Point(), view);
        Point fieldPosition = nameField.TranslatePoint(new Point(), view);
        Require(scroll.ScrollableHeight > 0, "Long fields must scroll within the fixed panel.");
        scroll.ScrollToEnd(); view.UpdateLayout();
        Require(scroll.VerticalOffset > 0 && mode2D.TranslatePoint(new Point(), view) == modePosition && list.TranslatePoint(new Point(), view) == categoryPosition,
            "Scrolling the fields must never move either tab column.");
        Require(nameField.TranslatePoint(new Point(), view).Y < fieldPosition.Y, "The fields themselves must move.");
        var verticalBar = (ScrollBar)scroll.Template.FindName("PART_VerticalScrollBar", scroll);
        var track = (Track)verticalBar.Template.FindName("PART_Track", verticalBar);
        Require(((SolidColorBrush)track.Thumb.Background).Color == ((SolidColorBrush)view.FindResource("PixelAccentBrush")).Color, "The scroll thumb must use the engine yellow.");
        var barGrid = (Grid)VisualTreeHelper.GetChild(verticalBar, 0);
        Require(((SolidColorBrush)barGrid.Background).Color == ((SolidColorBrush)view.FindResource("PixelBackgroundBrush")).Color, "The scroll track must use the engine background.");
        scroll.ScrollToTop(); view.UpdateLayout();
        var downArrow = (RepeatButton)barGrid.Children[2];
        ((RoutedCommand)downArrow.Command).Execute(null, downArrow); view.UpdateLayout();
        Require(scroll.VerticalOffset > 0, "The down arrow must scroll the field content.");
        var arrowFace = (Border)VisualTreeHelper.GetChild(downArrow, 0);
        Require(((SolidColorBrush)((System.Windows.Shapes.Path)arrowFace.Child).Fill).Color == ((SolidColorBrush)view.FindResource("PixelAccentBrush")).Color, "Scroll arrows must use the engine yellow.");
        scroll.ScrollToTop(); view.UpdateLayout();
        track.Thumb.RaiseEvent(new DragStartedEventArgs(0, 0) { RoutedEvent = Thumb.DragStartedEvent });
        track.Thumb.RaiseEvent(new DragDeltaEventArgs(0, 30) { RoutedEvent = Thumb.DragDeltaEvent });
        track.Thumb.RaiseEvent(new DragCompletedEventArgs(0, 30, false) { RoutedEvent = Thumb.DragCompletedEvent });
        view.UpdateLayout();
        Require(scroll.VerticalOffset > 0, "Thumb drag events must move the field content.");
        scroll.ScrollToTop();
        view.Measure(new Size(400, 480)); view.Arrange(new Rect(0, 0, 400, 480)); view.UpdateLayout();
        Require(scroll.ScrollableWidth > 0 && view.DesiredSize.Width <= 400, "Narrow panes must scroll the fields horizontally without expanding the page.");
        var horizontalBar = (ScrollBar)scroll.Template.FindName("PART_HorizontalScrollBar", scroll);
        var horizontalGrid = (Grid)VisualTreeHelper.GetChild(horizontalBar, 0);
        var rightArrow = (RepeatButton)horizontalGrid.Children[2];
        ((RoutedCommand)rightArrow.Command).Execute(null, rightArrow); view.UpdateLayout();
        Require(scroll.HorizontalOffset > 0, "Horizontal arrows must scroll only the field area.");
        scroll.ScrollToLeftEnd();
        view.Measure(new Size(1060, 680)); view.Arrange(new Rect(0, 0, 1060, 680)); view.UpdateLayout();
        list.IsEnabled = false; mode2D.IsEnabled = false; mode3D.IsEnabled = false;
        view.UpdateLayout();
        var disabledList = new RenderTargetBitmap((int)Math.Ceiling(list.ActualWidth), (int)Math.Ceiling(list.ActualHeight), 96, 96, PixelFormats.Pbgra32);
        disabledList.Render(list);
        byte[] disabledPixel = new byte[4];
        disabledList.CopyPixels(new Int32Rect((int)list.ActualWidth / 2, (int)list.ActualHeight - 3, 1, 1), disabledPixel, 4, 0);
        Require(disabledPixel[3] == 0 || disabledPixel.Take(3).Max() < 200, "Disabled resource tabs must not acquire the system white background.");
        list.IsEnabled = true; mode2D.IsEnabled = true; mode3D.IsEnabled = true;
        if (args.Length > 1)
        {
            list.SelectedIndex = 0; view.UpdateLayout();
            var bitmap = new RenderTargetBitmap(1060, 680, 96, 96, PixelFormats.Pbgra32); bitmap.Render(view);
            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
            using var output = File.Create(args[1]); encoder.Save(output);
        }
        list.SelectedIndex = 0; view.UpdateLayout();
        var sizeRow = Find<Grid>("ImageSizeRow");
        var nameGrid = (Grid)nameField.Content;
        Require(nameGrid.ColumnDefinitions[0].ActualWidth == 200 && sizeGrid.ColumnDefinitions[0].ActualWidth == 200,
            "String and vector fields must share their 200px name columns.");
        var xTop = widthInput.TranslatePoint(new Point(), sizeRow).Y;
        var yTop = heightInput.TranslatePoint(new Point(), sizeRow).Y;
        Require(xTop == yTop && sizeRow.RowDefinitions.Count <= 1, "X, Y and Auto must share one row.");
        var iconQuestions = Find<StackPanel>("IconQuestions");
        Require(iconQuestions.Visibility == Visibility.Visible, "Item-specific questions must be visible on the icon tab.");
        var subject = Find<UserControl>("ItemSubject");
        subject.GetType().GetProperty("Text")!.SetValue(subject, "Ice rune");
        Find<TextBox>("CustomRequest").Text = "Only add a blue border; preserve the artwork.";
        var promptMethod = artType.GetMethod("BuildPrompt", BindingFlags.Instance | BindingFlags.NonPublic)!;
        try { promptMethod.Invoke(view, new object[] { 64, 64 }); throw new Exception("Reference must be required."); }
        catch (TargetInvocationException ex) when (ex.InnerException is InvalidOperationException) { }
        byte[] pixels = new byte[32 * 16 * 4];
        for (int i = 0; i < pixels.Length; i += 4) { pixels[i] = 200; pixels[i + 3] = 128; }
        var referenceBitmap = BitmapSource.Create(32, 16, 96, 96, PixelFormats.Bgra32, null, pixels, 32 * 4);
        var referenceEncoder = new PngBitmapEncoder(); referenceEncoder.Frames.Add(BitmapFrame.Create(referenceBitmap));
        using (var output = File.Create(referencePath)) referenceEncoder.Save(output);
        artType.GetMethod("LoadReference", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(view, new object[] { referencePath });
        Require(Find<Button>("GenerateButton").IsEnabled && Find<Image>("ReferencePreview").Source != null, "Subject and reference must enable creation and display the reference.");
        string prompt = (string)promptMethod.Invoke(view, new object[] { 64, 64 })!;
        Require(prompt.Contains("Ice rune") && prompt.Contains("Only add a blue border") && prompt.Contains("64 x 64") && prompt.Contains("transparent alpha"), "The art brief must include subject, custom edits, size and alpha.");
        var colorField = Find<UserControl>("PaletteInput");
        var paletteMode = Find<UserControl>("UseReferencePalette");
        Require(colorField.GetType().Name == "ColorField" && !colorField.IsEnabled && prompt.Contains("Palette: match the reference"), "The shared color field must preserve reference colors by default.");
        paletteMode.GetType().GetProperty("Value")!.SetValue(paletteMode, false);
        colorField.GetType().GetProperty("Value")!.SetValue(colorField, Color.FromArgb(128, 12, 34, 56));
        string coloredPrompt = (string)promptMethod.Invoke(view, new object[] { 64, 64 })!;
        Require(colorField.IsEnabled && coloredPrompt.Contains("#0C2238") && coloredPrompt.Contains("A=128"), "Custom RGBA color must reach the generation brief.");
        Require((string)subject.GetType().GetProperty("Label")!.GetValue(subject)! == "그릴 대상 *", "The subject field must clearly name the drawing target.");
        var resultType = assembly.GetType("PixelTool.ArtImageResultWindow", true)!;
        byte[] fitted = (byte[])resultType.GetMethod("FitImage", BindingFlags.Static | BindingFlags.NonPublic)!.Invoke(null, new object[] { File.ReadAllBytes(referencePath), 64, 64 })!;
        using (var input = new MemoryStream(fitted))
        {
            var frame = BitmapDecoder.Create(input, BitmapCreateOptions.None, BitmapCacheOption.OnLoad).Frames[0];
            Require(frame.PixelWidth == 64 && frame.PixelHeight == 64, "The output must use the requested pixel dimensions.");
            var rgba = new FormatConvertedBitmap(frame, PixelFormats.Bgra32, null, 0);
            byte[] data = new byte[64 * 64 * 4]; rgba.CopyPixels(data, 64 * 4, 0);
            Require(data[3] == 0 && data[(32 * 64 + 32) * 4 + 3] == 128, "Letterboxing and source transparency must be preserved.");
        }
        Require(!File.Exists(resultPath), "The preview test must start without a result file.");
        var preview = (Window)Activator.CreateInstance(resultType, fitted, asset, "result.png")!;
        Require(!File.Exists(resultPath), "Opening a preview must not save automatically.");
        if (args.Length > 1)
        {
            preview.Resources.MergedDictionaries.Add((ResourceDictionary)Activator.CreateInstance(assembly.GetType("PixelTool.EditorColors", true)!)!);
            var content = (FrameworkElement)preview.Content;
            content.Measure(new Size(600, 480)); content.Arrange(new Rect(0, 0, 600, 480)); content.UpdateLayout();
            var image = new RenderTargetBitmap(600, 480, 96, 96, PixelFormats.Pbgra32); image.Render(content);
            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(image));
            using var output = File.Create(Path.ChangeExtension(args[1], ".result.png")); encoder.Save(output);
        }
        preview.Close();
        var saveMethod = resultType.GetMethod("SaveImage", BindingFlags.Static | BindingFlags.NonPublic)!;
        saveMethod.Invoke(null, new object[] { resultPath, asset, fitted });
        Require(File.ReadAllBytes(resultPath).SequenceEqual(fitted), "Explicit save must preserve the PNG.");
        foreach (string denied in new[] { resultPath, Path.Combine(folder, "outside.png") })
        {
            try { saveMethod.Invoke(null, new object[] { denied, asset, fitted }); throw new Exception("Overwrite or out-of-Asset save was accepted."); }
            catch (TargetInvocationException ex) when (ex.InnerException is IOException) { }
        }
        artType.GetMethod("ClearReference", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(view, new object[] { view, new RoutedEventArgs() });
        Require(!Find<Button>("GenerateButton").IsEnabled && Find<Image>("ReferencePreview").Source == null, "Clearing the reference must disable generation.");
        Console.WriteLine("PASS: default 2D, item questions, custom/reference brief, preview-only results, alpha/size normalization, Asset-only non-overwrite save and existing layout/Auto behavior. No CLI, AI or engine started.");
        }
        finally
        {
            foreach (string fixture in fixtures) File.Delete(fixture);
            Directory.Delete(nested); Directory.Delete(asset); Directory.Delete(folder);
        }
    }
}
