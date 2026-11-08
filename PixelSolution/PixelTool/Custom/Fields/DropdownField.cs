using System.Collections;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace PixelTool;

public sealed class DropdownField : EditorFieldBase
{
    public static readonly DependencyProperty ItemsSourceProperty = DependencyProperty.Register(nameof(ItemsSource), typeof(IEnumerable), typeof(DropdownField));
    public static readonly DependencyProperty SelectedItemProperty = DependencyProperty.Register(nameof(SelectedItem), typeof(object), typeof(DropdownField),
        new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, (d, _) => ((DropdownField)d).NotifyValueChanged()));
    public static readonly DependencyProperty DisplayMemberPathProperty = DependencyProperty.Register(nameof(DisplayMemberPath), typeof(string), typeof(DropdownField), new PropertyMetadata(""));
    public IEnumerable? ItemsSource { get => (IEnumerable?)GetValue(ItemsSourceProperty); set => SetValue(ItemsSourceProperty, value); }
    public object? SelectedItem { get => GetValue(SelectedItemProperty); set => SetValue(SelectedItemProperty, value); }
    public string DisplayMemberPath { get => (string)GetValue(DisplayMemberPathProperty); set => SetValue(DisplayMemberPathProperty, value); }
    public DropdownField()
    {
        InputBorder.MinWidth = 128;
        var combo = new ComboBox { Style = FieldStyle("FieldCombo") };
        combo.SetBinding(ItemsControl.ItemsSourceProperty, new Binding(nameof(ItemsSource)) { Source = this });
        combo.SetBinding(ItemsControl.DisplayMemberPathProperty, new Binding(nameof(DisplayMemberPath)) { Source = this });
        combo.SetBinding(ComboBox.SelectedItemProperty, new Binding(nameof(SelectedItem)) { Source = this, Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged });
        combo.DropDownOpened += (_, _) => SetActive(true); combo.DropDownClosed += (_, _) => SetActive(false);
        Unloaded += (_, _) => combo.IsDropDownOpen = false;
        IsVisibleChanged += (_, _) => { if (!IsVisible) combo.IsDropDownOpen = false; };
        IsEnabledChanged += (_, _) => { if (!IsEnabled) combo.IsDropDownOpen = false; };
        EditorHost.Content = combo;
    }
}
