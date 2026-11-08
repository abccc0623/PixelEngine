using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace PixelTool;

/// <summary>Aligned label and focus border shared by editor property controls.</summary>
public abstract class EditorFieldBase : UserControl
{
    public static readonly DependencyProperty LabelProperty = DependencyProperty.Register(nameof(Label), typeof(string), typeof(EditorFieldBase), new PropertyMetadata(""));
    public static readonly DependencyProperty LabelWidthProperty = DependencyProperty.Register(nameof(LabelWidth), typeof(double), typeof(EditorFieldBase),
        new PropertyMetadata(200d, (d, e) => ((Grid)((EditorFieldBase)d).Content).ColumnDefinitions[0].Width = new GridLength((double)e.NewValue)), value => double.IsFinite((double)value) && (double)value >= 0);
    private static readonly DependencyPropertyKey IsActivePropertyKey = DependencyProperty.RegisterReadOnly(nameof(IsActive), typeof(bool), typeof(EditorFieldBase), new PropertyMetadata(false));
    public static readonly DependencyProperty IsActiveProperty = IsActivePropertyKey.DependencyProperty;
    public string Label { get => (string)GetValue(LabelProperty); set => SetValue(LabelProperty, value); }
    public double LabelWidth { get => (double)GetValue(LabelWidthProperty); set => SetValue(LabelWidthProperty, value); }
    public bool IsActive => (bool)GetValue(IsActiveProperty);
    public event RoutedEventHandler? ValueChanged;
    protected Border InputBorder { get; } = new() { Height = 24, CornerRadius = new CornerRadius(3), BorderThickness = new Thickness(1), VerticalAlignment = VerticalAlignment.Center };
    protected ContentControl EditorHost { get; } = new() { HorizontalContentAlignment = HorizontalAlignment.Stretch, VerticalContentAlignment = VerticalAlignment.Stretch };

    protected EditorFieldBase()
    {
        Resources.MergedDictionaries.Add(new ResourceDictionary { Source = new Uri("/PixelTool;component/Custom/Fields/FieldStyles.xaml", UriKind.Relative) });
        SetResourceReference(ForegroundProperty, "PixelTextBrush");
        var grid = new Grid(); grid.ColumnDefinitions.Add(new() { Width = new GridLength(200) }); grid.ColumnDefinitions.Add(new());
        var label = new TextBlock { VerticalAlignment = VerticalAlignment.Center, Margin = new Thickness(0, 0, 12, 0), TextTrimming = TextTrimming.CharacterEllipsis, TextWrapping = TextWrapping.NoWrap };
        label.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        label.SetBinding(TextBlock.TextProperty, new Binding(nameof(Label)) { Source = this }); label.SetBinding(ToolTipProperty, new Binding(nameof(Label)) { Source = this });
        grid.Children.Add(label);
        InputBorder.SetResourceReference(Border.BackgroundProperty, "PixelBackgroundBrush");
        var style = new Style(typeof(Border)); style.Setters.Add(new Setter(Border.BorderBrushProperty, new DynamicResourceExtension("PixelBorderBrush")));
        foreach (string property in new[] { nameof(IsKeyboardFocusWithin), nameof(IsActive) })
        {
            var trigger = new DataTrigger { Binding = new Binding(property) { Source = this }, Value = true };
            trigger.Setters.Add(new Setter(Border.BorderBrushProperty, new DynamicResourceExtension("PixelAccentBrush"))); style.Triggers.Add(trigger);
        }
        var disabled = new DataTrigger { Binding = new Binding(nameof(IsEnabled)) { Source = this }, Value = false };
        disabled.Setters.Add(new Setter(OpacityProperty, .5)); style.Triggers.Add(disabled);
        InputBorder.Style = style; InputBorder.Child = EditorHost; Grid.SetColumn(InputBorder, 1); grid.Children.Add(InputBorder); Content = grid;
    }
    protected void SetActive(bool value) => SetValue(IsActivePropertyKey, value);
    protected void NotifyValueChanged() => ValueChanged?.Invoke(this, new RoutedEventArgs());
    protected Style FieldStyle(string key) => (Style)FindResource(key);
}
