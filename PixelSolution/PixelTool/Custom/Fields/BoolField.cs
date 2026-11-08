using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;

namespace PixelTool;

public sealed class BoolField : EditorFieldBase
{
    public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(nameof(Value), typeof(bool), typeof(BoolField),
        new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, (d, _) => ((BoolField)d).NotifyValueChanged()));
    public bool Value { get => (bool)GetValue(ValueProperty); set => SetValue(ValueProperty, value); }
    public BoolField()
    {
        InputBorder.Width = 64; InputBorder.HorizontalAlignment = HorizontalAlignment.Left;
        var toggle = new ToggleButton { Style = FieldStyle("FieldToggle"), IsThreeState = false };
        toggle.SetBinding(ToggleButton.IsCheckedProperty, new Binding(nameof(Value)) { Source = this, Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged });
        EditorHost.Content = toggle;
    }
}
