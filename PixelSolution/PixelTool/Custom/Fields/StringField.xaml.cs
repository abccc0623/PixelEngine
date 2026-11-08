using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PixelTool;

/// <summary>Single-line editor string field, aligned with the shared vector fields.</summary>
public partial class StringField : UserControl
{
    public static readonly DependencyProperty LabelProperty = DependencyProperty.Register(nameof(Label), typeof(string), typeof(StringField), new PropertyMetadata(""));
    public static readonly DependencyProperty LabelWidthProperty = DependencyProperty.Register(nameof(LabelWidth), typeof(double), typeof(StringField),
        new PropertyMetadata(200d, LabelWidthChanged), value => double.IsFinite((double)value) && (double)value >= 0);
    public static readonly DependencyProperty TextProperty = DependencyProperty.Register(nameof(Text), typeof(string), typeof(StringField),
        new FrameworkPropertyMetadata("", FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
            (d, _) => ((StringField)d).ValueChanged?.Invoke(d, new RoutedEventArgs()), (_, value) => value ?? ""));
    public static readonly DependencyProperty MaxLengthProperty = DependencyProperty.Register(nameof(MaxLength), typeof(int), typeof(StringField), new PropertyMetadata(0), value => (int)value >= 0);
    public static readonly DependencyProperty InputWidthProperty = DependencyProperty.Register(nameof(InputWidth), typeof(double), typeof(StringField),
        new PropertyMetadata(128d), value => double.IsFinite((double)value) && (double)value > 0);

    public string Label { get => (string)GetValue(LabelProperty); set => SetValue(LabelProperty, value); }
    public double LabelWidth { get => (double)GetValue(LabelWidthProperty); set => SetValue(LabelWidthProperty, value); }
    public string Text { get => (string)GetValue(TextProperty); set => SetValue(TextProperty, value); }
    public int MaxLength { get => (int)GetValue(MaxLengthProperty); set => SetValue(MaxLengthProperty, value); }
    public double InputWidth { get => (double)GetValue(InputWidthProperty); set => SetValue(InputWidthProperty, value); }
    public event RoutedEventHandler? ValueChanged;
    public event RoutedEventHandler? TextCommitted;

    public StringField() => InitializeComponent();

    private static void LabelWidthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (((StringField)d).LabelColumn is { } column) column.Width = new GridLength((double)e.NewValue);
    }
    private void InputFinished(object sender, KeyboardFocusChangedEventArgs e)
        => TextCommitted?.Invoke(this, new RoutedEventArgs());
    private void InputKeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key != Key.Enter) return;
        TextCommitted?.Invoke(this, new RoutedEventArgs());
        e.Handled = true;
    }
}
