using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using System.Windows.Automation;

namespace PixelTool;

/// <summary>RGBA byte color editor with a transparent swatch preview.</summary>
public sealed class ColorField : EditorFieldBase
{
    public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(nameof(Value), typeof(Color), typeof(ColorField),
        new FrameworkPropertyMetadata(Colors.White, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, (d, _) => ((ColorField)d).Refresh()));
    public Color Value { get => (Color)GetValue(ValueProperty); set => SetValue(ValueProperty, value); }
    private readonly Border swatch = new();
    private readonly NumericAxisField[] channels = new NumericAxisField[4];
    private readonly List<(Color Color, Border Border)> paletteSwatches = new();
    private readonly TextBlock colorCode = new() { Margin = new Thickness(2, 4, 2, 8) };
    private ScreenColorPicker? screenPicker;
    private bool syncing;

    public ColorField()
    {
        InputBorder.Width = 64;
        InputBorder.HorizontalAlignment = HorizontalAlignment.Left;
        var button = new Button { Style = FieldStyle("FieldButton"), Padding = new Thickness(3) };
        var checker = new Border { Background = (Brush)FindResource("FieldChecker"), Child = swatch, Width = 52, Height = 16 };
        button.Content = checker;
        EditorHost.Content = button;
        var rows = new StackPanel { Width = 224 };
        colorCode.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        rows.Children.Add(colorCode);
        var palette = new UniformGrid { Columns = 8, Margin = new Thickness(0, 0, 0, 8) };
        foreach (string hex in new[]
        {
            "FFFFFF", "E0E0E0", "BDBDBD", "9E9E9E", "757575", "505050", "303030", "000000",
            "FFCDD2", "FFE0B2", "FFF9C4", "C8E6C9", "B2EBF2", "BBDEFB", "D1C4E9", "F8BBD0",
            "EF9A9A", "FFCC80", "FFF176", "A5D6A7", "80DEEA", "90CAF9", "B39DDB", "F48FB1",
            "F44336", "FF9800", "FFEB3B", "4CAF50", "00BCD4", "2196F3", "673AB7", "E91E63",
            "B71C1C", "E65100", "F57F17", "1B5E20", "006064", "0D47A1", "311B92", "880E4F"
        })
        {
            var color = (Color)ColorConverter.ConvertFromString("#" + hex);
            var face = new Border { Width = 22, Height = 22, Background = new SolidColorBrush(color), BorderThickness = new Thickness(2), CornerRadius = new CornerRadius(2) };
            var choice = new Button { Content = face, Padding = new Thickness(2), Margin = new Thickness(1), Style = FieldStyle("FieldButton"), ToolTip = "#" + hex };
            AutomationProperties.SetName(choice, "색상 #" + hex);
            choice.Click += (_, _) => SetCurrentValue(ValueProperty, Color.FromArgb(Value.A, color.R, color.G, color.B));
            paletteSwatches.Add((color, face)); palette.Children.Add(choice);
        }
        rows.Children.Add(palette);
        for (int i = 0; i < channels.Length; i++)
        {
            channels[i] = new NumericAxisField { Axis = new[] { "R:", "G:", "B:", "A:" }[i], Minimum = 0, Maximum = 255, Step = 1, DecimalPlaces = 0, Margin = new Thickness(2) };
            channels[i].ValueChanged += (_, _) =>
            {
                if (!syncing)
                    SetCurrentValue(ValueProperty, Color.FromArgb((byte)channels[3].Value, (byte)channels[0].Value, (byte)channels[1].Value, (byte)channels[2].Value));
            };
            rows.Children.Add(channels[i]);
        }
        var panel = new Border { Child = rows, Padding = new Thickness(6), BorderThickness = new Thickness(1) };
        panel.SetResourceReference(Border.BackgroundProperty, "PixelPanelBrush");
        panel.SetResourceReference(Border.BorderBrushProperty, "PixelBorderBrush");
        var popup = new Popup { Child = panel, PlacementTarget = button, Placement = PlacementMode.Bottom, StaysOpen = false, AllowsTransparency = true };
        ((Grid)Content).Children.Add(popup);
        var eyedropper = new Button { Content = "스포이드 · 화면에서 선택", Style = FieldStyle("FieldButton"), Padding = new Thickness(8), Margin = new Thickness(0, 8, 0, 0), ToolTip = "화면의 색상을 클릭하여 선택합니다. Esc 또는 우클릭으로 취소합니다. 알파값은 유지합니다." };
        AutomationProperties.SetName(eyedropper, "화면 색상 스포이드");
        eyedropper.Click += async (_, _) =>
        {
            popup.IsOpen = false;
            // Let the popup disappear before capturing the desktop.
            await Dispatcher.InvokeAsync(() => { }, DispatcherPriority.ContextIdle);
            if (!IsLoaded || !IsVisible || !IsEnabled || screenPicker != null) return;
            try
            {
                SetActive(true);
                screenPicker = new ScreenColorPicker(Value.A);
                if (Window.GetWindow(this) is { } owner) screenPicker.Owner = owner;
                if (screenPicker.ShowDialog() == true && screenPicker.SelectedColor is { } color)
                    SetCurrentValue(ValueProperty, color);
            }
            catch (Exception ex) when (ex is System.ComponentModel.Win32Exception or InvalidOperationException)
            { MessageBox.Show("화면 색상을 선택하지 못했습니다. " + ex.Message, "ColorField", MessageBoxButton.OK, MessageBoxImage.Warning); }
            finally
            {
                screenPicker?.Close(); screenPicker = null; SetActive(false);
                if (IsLoaded && IsVisible && IsEnabled) popup.IsOpen = true;
            }
        };
        rows.Children.Add(eyedropper);
        button.Click += (_, _) => popup.IsOpen = !popup.IsOpen;
        popup.Opened += (_, _) => SetActive(true);
        popup.Closed += (_, _) => SetActive(false);
        panel.PreviewKeyDown += (_, e) => { if (e.Key == Key.Escape) { popup.IsOpen = false; e.Handled = true; } };
        void ClosePicker() { popup.IsOpen = false; screenPicker?.Close(); }
        Unloaded += (_, _) => ClosePicker();
        IsEnabledChanged += (_, _) => { if (!IsEnabled) ClosePicker(); };
        IsVisibleChanged += (_, _) => { if (!IsVisible) ClosePicker(); };
        Refresh();
    }

    private void Refresh()
    {
        if (channels[0] == null) return;
        syncing = true;
        try
        {
            byte[] values = { Value.R, Value.G, Value.B, Value.A };
            for (int i = 0; i < channels.Length; i++) channels[i].SetCurrentValue(NumericAxisField.ValueProperty, (double)values[i]);
            swatch.Background = new SolidColorBrush(Value);
            colorCode.Text = $"#{Value.R:X2}{Value.G:X2}{Value.B:X2} · A {Value.A}";
            foreach (var entry in paletteSwatches)
            {
                bool selected = entry.Color.R == Value.R && entry.Color.G == Value.G && entry.Color.B == Value.B;
                entry.Border.SetResourceReference(Border.BorderBrushProperty, selected ? "PixelAccentBrush" : "PixelBorderBrush");
            }
            ToolTip = $"R {Value.R} · G {Value.G} · B {Value.B} · A {Value.A}";
        }
        finally { syncing = false; }
        NotifyValueChanged();
    }
}
