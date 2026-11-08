using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace PixelTool;

/// <summary>Shared Vector2/3 layout. Keeps the consumer's DataContext and supports two-way axis bindings.</summary>
public abstract class VectorField : UserControl
{
    public static readonly DependencyProperty LabelProperty = DependencyProperty.Register(nameof(Label), typeof(string), typeof(VectorField), new PropertyMetadata(""));
    public static readonly DependencyProperty LabelWidthProperty = DependencyProperty.Register(nameof(LabelWidth), typeof(double), typeof(VectorField),
        new PropertyMetadata(200d, LabelWidthChanged), value => Finite(value) && (double)value >= 0);
    public static readonly DependencyProperty XProperty = Coordinate(nameof(X));
    public static readonly DependencyProperty YProperty = Coordinate(nameof(Y));
    public static readonly DependencyProperty ZProperty = Coordinate(nameof(Z));
    public static readonly DependencyProperty MinimumProperty = DependencyProperty.Register(nameof(Minimum), typeof(double), typeof(VectorField), new PropertyMetadata(double.MinValue, RangeChanged), Finite);
    public static readonly DependencyProperty MaximumProperty = DependencyProperty.Register(nameof(Maximum), typeof(double), typeof(VectorField), new PropertyMetadata(double.MaxValue, RangeChanged), Finite);
    public static readonly DependencyProperty StepProperty = DependencyProperty.Register(nameof(Step), typeof(double), typeof(VectorField), new PropertyMetadata(.1d), value => Finite(value) && (double)value > 0);
    public static readonly DependencyProperty DecimalPlacesProperty = DependencyProperty.Register(nameof(DecimalPlaces), typeof(int), typeof(VectorField), new PropertyMetadata(3), value => (int)value is >= 0 and <= 8);
    public string Label { get => (string)GetValue(LabelProperty); set => SetValue(LabelProperty, value); }
    public double LabelWidth { get => (double)GetValue(LabelWidthProperty); set => SetValue(LabelWidthProperty, value); }
    public double X { get => (double)GetValue(XProperty); set => SetValue(XProperty, value); }
    public double Y { get => (double)GetValue(YProperty); set => SetValue(YProperty, value); }
    public double Z { get => (double)GetValue(ZProperty); set => SetValue(ZProperty, value); }
    public double Minimum { get => (double)GetValue(MinimumProperty); set => SetValue(MinimumProperty, value); }
    public double Maximum { get => (double)GetValue(MaximumProperty); set => SetValue(MaximumProperty, value); }
    public double Step { get => (double)GetValue(StepProperty); set => SetValue(StepProperty, value); }
    public int DecimalPlaces { get => (int)GetValue(DecimalPlacesProperty); set => SetValue(DecimalPlacesProperty, value); }
    public event RoutedEventHandler? ValueChanged;

    protected VectorField(int dimensions)
    {
        SetResourceReference(ForegroundProperty, "PixelTextBrush");
        var grid = new Grid();
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(LabelWidth) });
        var label = new TextBlock { VerticalAlignment = VerticalAlignment.Center, Margin = new Thickness(0, 0, 12, 0),
            TextWrapping = TextWrapping.NoWrap, TextTrimming = TextTrimming.CharacterEllipsis };
        label.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        label.SetBinding(TextBlock.TextProperty, new Binding(nameof(Label)) { Source = this });
        label.SetBinding(TextBlock.ToolTipProperty, new Binding(nameof(Label)) { Source = this });
        grid.Children.Add(label);
        string[] axes = { nameof(X), nameof(Y), nameof(Z) };
        for (int index = 0; index < dimensions; index++)
        {
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
            var field = new NumericAxisField { Axis = axes[index] + ":", Margin = new Thickness(index == 0 ? 0 : 8, 0, 0, 0) };
            field.SetBinding(NumericAxisField.MinimumProperty, new Binding(nameof(Minimum)) { Source = this });
            field.SetBinding(NumericAxisField.MaximumProperty, new Binding(nameof(Maximum)) { Source = this });
            field.SetBinding(NumericAxisField.StepProperty, new Binding(nameof(Step)) { Source = this });
            field.SetBinding(NumericAxisField.DecimalPlacesProperty, new Binding(nameof(DecimalPlaces)) { Source = this });
            field.SetBinding(NumericAxisField.ValueProperty, new Binding(axes[index]) { Source = this, Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged });
            Grid.SetColumn(field, index + 1); grid.Children.Add(field);
        }
        Content = grid;
    }
    private static bool Finite(object value) => double.IsFinite((double)value);
    private static void LabelWidthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (((VectorField)d).Content is Grid grid) grid.ColumnDefinitions[0].Width = new GridLength((double)e.NewValue);
    }
    private static DependencyProperty Coordinate(string name) => DependencyProperty.Register(name, typeof(double), typeof(VectorField),
        new FrameworkPropertyMetadata(0d, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
            (d, _) => ((VectorField)d).ValueChanged?.Invoke(d, new RoutedEventArgs()),
            (d, value) => Math.Clamp((double)value, ((VectorField)d).Minimum, Math.Max(((VectorField)d).Minimum, ((VectorField)d).Maximum))), Finite);
    private static void RangeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    { d.CoerceValue(XProperty); d.CoerceValue(YProperty); d.CoerceValue(ZProperty); }
}
