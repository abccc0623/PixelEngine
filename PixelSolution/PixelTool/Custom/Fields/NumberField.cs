using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace PixelTool;

/// <summary>Standalone scalar field. Use Step=1 and DecimalPlaces=0 for integer input.</summary>
public sealed class NumberField : NumericAxisField
{
    public static readonly DependencyProperty LabelProperty = DependencyProperty.Register(nameof(Label), typeof(string), typeof(NumberField),
        new PropertyMetadata("Number", (d, e) => d.SetCurrentValue(AxisProperty, e.NewValue)));
    public static readonly DependencyProperty LabelWidthProperty = DependencyProperty.Register(nameof(LabelWidth), typeof(double), typeof(NumberField),
        new PropertyMetadata(200d, (d, e) => ((Grid)((NumberField)d).Content).ColumnDefinitions[0].Width = new GridLength((double)e.NewValue)), value => double.IsFinite((double)value) && (double)value >= 0);
    public string Label { get => (string)GetValue(LabelProperty); set => SetValue(LabelProperty, value); }
    public double LabelWidth { get => (double)GetValue(LabelWidthProperty); set => SetValue(LabelWidthProperty, value); }
    public NumberField()
    {
        SetCurrentValue(AxisProperty, Label); ((Grid)Content).ColumnDefinitions[0].Width = new GridLength(LabelWidth);
        AxisLabel.TextTrimming = TextTrimming.CharacterEllipsis; AxisLabel.TextWrapping = TextWrapping.NoWrap;
        AxisLabel.SetBinding(ToolTipProperty, new Binding(nameof(Label)) { Source = this });
    }
}
