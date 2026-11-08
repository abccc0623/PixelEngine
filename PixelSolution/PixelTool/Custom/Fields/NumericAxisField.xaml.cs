using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PixelTool;

/// <summary>One editable numeric axis with drag-label interaction and editor focus highlighting.</summary>
public partial class NumericAxisField : UserControl
{
    public static readonly DependencyProperty AxisProperty = DependencyProperty.Register(nameof(Axis), typeof(string), typeof(NumericAxisField), new PropertyMetadata("X:"));
    public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(nameof(Value), typeof(double), typeof(NumericAxisField),
        new FrameworkPropertyMetadata(0d, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, ValueUpdated, CoerceNumber), Finite);
    public static readonly DependencyProperty MinimumProperty = DependencyProperty.Register(nameof(Minimum), typeof(double), typeof(NumericAxisField), new PropertyMetadata(double.MinValue, OptionsUpdated), Finite);
    public static readonly DependencyProperty MaximumProperty = DependencyProperty.Register(nameof(Maximum), typeof(double), typeof(NumericAxisField), new PropertyMetadata(double.MaxValue, OptionsUpdated), Finite);
    public static readonly DependencyProperty StepProperty = DependencyProperty.Register(nameof(Step), typeof(double), typeof(NumericAxisField), new PropertyMetadata(.1d), value => Finite(value) && (double)value > 0);
    public static readonly DependencyProperty DecimalPlacesProperty = DependencyProperty.Register(nameof(DecimalPlaces), typeof(int), typeof(NumericAxisField), new PropertyMetadata(3, OptionsUpdated), value => (int)value is >= 0 and <= 8);
    private static readonly DependencyPropertyKey IsDraggingPropertyKey = DependencyProperty.RegisterReadOnly(nameof(IsDragging), typeof(bool), typeof(NumericAxisField), new PropertyMetadata(false));
    public static readonly DependencyProperty IsDraggingProperty = IsDraggingPropertyKey.DependencyProperty;
    public string Axis { get => (string)GetValue(AxisProperty); set => SetValue(AxisProperty, value); }
    public double Value { get => (double)GetValue(ValueProperty); set => SetValue(ValueProperty, value); }
    public double Minimum { get => (double)GetValue(MinimumProperty); set => SetValue(MinimumProperty, value); }
    public double Maximum { get => (double)GetValue(MaximumProperty); set => SetValue(MaximumProperty, value); }
    public double Step { get => (double)GetValue(StepProperty); set => SetValue(StepProperty, value); }
    public int DecimalPlaces { get => (int)GetValue(DecimalPlacesProperty); set => SetValue(DecimalPlacesProperty, value); }
    public bool IsDragging => (bool)GetValue(IsDraggingProperty);
    public event RoutedEventHandler? ValueChanged;
    private bool _ready, _syncing, _typing;
    private double _dragX, _dragValue;

    public NumericAxisField()
    {
        InitializeComponent(); _ready = true; RefreshText();
        PreviewKeyDown += OnKeyDown;
        Unloaded += (_, _) => EndDrag();
        IsEnabledChanged += (_, _) => { if (!IsEnabled) EndDrag(); };
        IsVisibleChanged += (_, _) => { if (!IsVisible) EndDrag(); };
    }
    private static bool Finite(object value) => double.IsFinite((double)value);
    private static object CoerceNumber(DependencyObject d, object value)
    {
        var field = (NumericAxisField)d;
        return Math.Clamp((double)value, field.Minimum, Math.Max(field.Minimum, field.Maximum));
    }
    private static void ValueUpdated(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var field = (NumericAxisField)d;
        if (!field._typing) field.RefreshText();
        field.ValueChanged?.Invoke(field, new RoutedEventArgs());
    }
    private static void OptionsUpdated(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        d.CoerceValue(ValueProperty); ((NumericAxisField)d).RefreshText();
    }
    private void RefreshText()
    {
        if (!_ready) return;
        _syncing = true;
        InputBox.Text = Value.ToString(DecimalPlaces == 0 ? "0" : "0." + new string('#', DecimalPlaces), CultureInfo.InvariantCulture);
        _syncing = false;
    }
    private bool TryRead(out double value) => double.TryParse(InputBox.Text, NumberStyles.Float, CultureInfo.InvariantCulture, out value) && double.IsFinite(value);
    private void InputChanged(object sender, TextChangedEventArgs e)
    {
        if (!_ready || _syncing || !TryRead(out double value) || value < Minimum || value > Maximum ||
            (DecimalPlaces == 0 && value != Math.Truncate(value))) return;
        _typing = true;
        try { SetCurrentValue(ValueProperty, value); }
        finally { _typing = false; }
    }
    private void InputFinished(object sender, KeyboardFocusChangedEventArgs e) => CommitInput();
    private void CommitInput()
    {
        if (TryRead(out double value)) SetCurrentValue(ValueProperty, Math.Round(value, DecimalPlaces, MidpointRounding.AwayFromZero));
        RefreshText();
    }
    internal static double DragValue(double start, double delta, double step, int decimals, double minimum, double maximum)
        => Math.Clamp(Math.Round(start + Math.Truncate(delta / 4) * step, decimals, MidpointRounding.AwayFromZero), minimum, Math.Max(minimum, maximum));
    private void DragStart(object sender, MouseButtonEventArgs e)
    {
        if (!IsEnabled) return;
        CommitInput();
        if (!AxisLabel.CaptureMouse()) return;
        _dragValue = Value; _dragX = e.GetPosition(this).X;
        SetValue(IsDraggingPropertyKey, true); AxisLabel.Focus(); e.Handled = true;
    }
    private void DragMove(object sender, MouseEventArgs e)
    {
        if (!IsDragging) return;
        if (e.LeftButton != MouseButtonState.Pressed) { EndDrag(); return; }
        SetCurrentValue(ValueProperty, DragValue(_dragValue, e.GetPosition(this).X - _dragX, Step, DecimalPlaces, Minimum, Maximum));
        e.Handled = true;
    }
    private void DragEnd(object sender, MouseButtonEventArgs e) { if (IsDragging) { EndDrag(); e.Handled = true; } }
    private void DragLostCapture(object sender, MouseEventArgs e) => EndDrag();
    private void EndDrag()
    {
        SetValue(IsDraggingPropertyKey, false);
        if (AxisLabel.IsMouseCaptured) AxisLabel.ReleaseMouseCapture();
    }
    private void OnKeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter) { CommitInput(); e.Handled = true; }
        else if (e.Key == Key.Escape)
        {
            if (IsDragging) SetCurrentValue(ValueProperty, _dragValue);
            EndDrag(); RefreshText(); e.Handled = true;
        }
    }
}
