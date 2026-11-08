using System.ComponentModel;
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

internal sealed class Source : INotifyPropertyChanged
{
    private double _coordinate;
    private string _text = "Initial";
    public double Coordinate { get => _coordinate; set { _coordinate = value; PropertyChanged?.Invoke(this, new(nameof(Coordinate))); } }
    public string Text { get => _text; set { _text = value; PropertyChanged?.Invoke(this, new(nameof(Text))); } }
    public event PropertyChangedEventHandler? PropertyChanged;
}

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        var assembly = Assembly.LoadFrom(Path.GetFullPath(args[0]));
        var vectorBase = assembly.GetType("PixelTool.VectorField", true)!;
        var axisType = assembly.GetType("PixelTool.NumericAxisField", true)!;
        void Require(bool condition, string message) { if (!condition) throw new Exception(message); }
        DependencyProperty Property(Type type, string name) => (DependencyProperty)type.GetField(name + "Property", BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy)!.GetValue(null)!;
        double Number(UserControl field, string name) => (double)field.GetValue(Property(field.GetType(), name));
        foreach (int dimensions in new[] { 2, 3 })
        {
            var vector = (UserControl)Activator.CreateInstance(assembly.GetType("PixelTool.Vector" + dimensions + "Field", true)!)!;
            vector.Resources.MergedDictionaries.Add((ResourceDictionary)Activator.CreateInstance(assembly.GetType("PixelTool.EditorColors", true)!)!);
            var grid = (Grid)vector.Content;
            Require(grid.Children.Count == dimensions + 1, "Incorrect axis count.");
            var source = new Source { Coordinate = 1.25 }; vector.DataContext = source;
            var xProperty = Property(vectorBase, "X");
            BindingOperations.SetBinding(vector, xProperty, new Binding(nameof(Source.Coordinate)) { Source = source, Mode = BindingMode.TwoWay });
            vector.Measure(new Size(420, 40)); vector.Arrange(new Rect(0, 0, 420, 40)); vector.UpdateLayout();
            var axis = (UserControl)grid.Children[1];
            var input = (TextBox)axis.FindName("InputBox");
            Require(ReferenceEquals(vector.DataContext, source), "Vector fields must not replace consumer DataContext.");
            Require(input.Text == "1.25", "Initial binding value missing.");
            input.Text = "-3.25";
            Require(Number(vector, "X") == -3.25 && source.Coordinate == -3.25, "Typed numbers must propagate through both two-way bindings.");
            input.Text = "-";
            Require(source.Coordinate == -3.25, "Incomplete numeric input must retain the valid value.");
            source.Coordinate = 7.5;
            vector.Dispatcher.Invoke(() => { }, System.Windows.Threading.DispatcherPriority.DataBind);
            Require(input.Text == "7.5", "Source changes must refresh numeric text.");
            var dragMethod = axisType.GetMethod("DragValue", BindingFlags.Static | BindingFlags.NonPublic)!;
            double Drag(double delta) => (double)dragMethod.Invoke(null, new object[] { 7.5d, delta, .1d, 3, -100d, 100d })!;
            Require(Drag(12) == 7.8 && Drag(-12) == 7.2 && Drag(3) == 7.5, "Axis drag must increment, decrement and ignore small motion.");
            axis.SetCurrentValue(Property(axisType, "Value"), Drag(12));
            Require(Number(vector, "X") == 7.8 && source.Coordinate == 7.8 && BindingOperations.IsDataBound(axis, Property(axisType, "Value")), "Dragging must preserve bindings.");
            foreach (int index in Enumerable.Range(1, dimensions))
            {
                var currentAxis = (UserControl)grid.Children[index];
                ((TextBox)currentAxis.FindName("InputBox")).Text = (-index - .5).ToString(System.Globalization.CultureInfo.InvariantCulture);
                string coordinate = index == 1 ? "X" : index == 2 ? "Y" : "Z";
                Require(Number(vector, coordinate) == -index - .5, "Each axis must update its matching coordinate.");
                var key = (DependencyPropertyKey)axisType.GetField("IsDraggingPropertyKey", BindingFlags.Static | BindingFlags.NonPublic)!.GetValue(null)!;
                currentAxis.SetValue(key, true); vector.UpdateLayout();
                var border = (Border)currentAxis.FindName("InputBorder");
                Require(((SolidColorBrush)border.BorderBrush).Color == ((SolidColorBrush)vector.FindResource("PixelAccentBrush")).Color, "Dragging must show a yellow field border.");
                currentAxis.SetValue(key, false); vector.UpdateLayout();
                Require(((SolidColorBrush)border.BorderBrush).Color == ((SolidColorBrush)vector.FindResource("PixelBorderBrush")).Color, "Inactive fields must restore the normal border.");
            }
            void NaturalLayout()
            {
                vector.Measure(new Size(double.PositiveInfinity, 40));
                vector.Arrange(new Rect(0, 0, vector.DesiredSize.Width, 40)); vector.UpdateLayout();
            }
            input.Text = "1"; NaturalLayout();
            Require(Number(vector, "LabelWidth") == 200 && grid.ColumnDefinitions[0].ActualWidth == 200, "Name columns must default to 200px.");
            double defaultAxisLeft = axis.TranslatePoint(new Point(), grid).X;
            Require(defaultAxisLeft == 200, "Both Vector2 and Vector3 must start X at the same 200px position.");
            vector.SetCurrentValue(Property(vectorBase, "Label"), "A much longer property name for alignment"); NaturalLayout();
            Require(axis.TranslatePoint(new Point(), grid).X == defaultAxisLeft, "Changing label text must not move the axes.");
            vector.SetCurrentValue(Property(vectorBase, "LabelWidth"), 120d); NaturalLayout();
            Require(axis.TranslatePoint(new Point(), grid).X == 120, "Custom label widths must update layout.");
            vector.SetCurrentValue(Property(vectorBase, "LabelWidth"), 200d); NaturalLayout();
            double initialWidth = vector.DesiredSize.Width;
            var axisPositions = grid.Children.OfType<UserControl>().Select(child => child.TranslatePoint(new Point(), grid).X).ToArray();
            foreach (string text in new[] { "10000", "-10000", "99999" })
            {
                input.Text = text; NaturalLayout();
                Require(vector.DesiredSize.Width == initialWidth, "Five-digit values must not resize the vector field.");
                Require(grid.Children.OfType<UserControl>().Select(child => child.TranslatePoint(new Point(), grid).X).SequenceEqual(axisPositions), "Larger numbers must not shift other axes.");
                foreach (var child in grid.Children.OfType<UserControl>())
                {
                    var border = (Border)child.FindName("InputBorder");
                    Require(border.ActualWidth == 64 && border.ActualHeight == 24, "Numeric fields must have a compact fixed 64x24 size.");
                }
            }
            vector.SetCurrentValue(Property(vectorBase, "Minimum"), 1d);
            vector.SetCurrentValue(Property(vectorBase, "Maximum"), 10d);
            axis.SetCurrentValue(Property(axisType, "Value"), 99d);
            Require(Number(vector, "X") == 10, "Bounds must apply to drag and input values.");
            input.Text = "invalid";
            axisType.GetMethod("CommitInput", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(axis, null);
            Require(input.Text == "10", "Invalid text must restore the last valid value.");
            vector.IsEnabled = false;
            Require(!input.IsEnabled, "Disabling vectors must disable every numeric field.");
            if (dimensions == 3) Require(((TextBlock)((UserControl)grid.Children[3]).FindName("AxisLabel")).Text == "Z:", "Vector3 must show the Z axis.");
        }
        var stringType = assembly.GetType("PixelTool.StringField", true)!;
        var stringField = (UserControl)Activator.CreateInstance(stringType)!;
        stringField.Resources.MergedDictionaries.Add((ResourceDictionary)Activator.CreateInstance(assembly.GetType("PixelTool.EditorColors", true)!)!);
        var stringSource = new Source(); stringField.DataContext = stringSource;
        var textProperty = Property(stringType, "Text");
        BindingOperations.SetBinding(stringField, textProperty, new Binding(nameof(Source.Text)) { Source = stringSource, Mode = BindingMode.TwoWay });
        stringField.Measure(new Size(500, 32)); stringField.Arrange(new Rect(0, 0, 500, 32)); stringField.UpdateLayout();
        var stringInput = (TextBox)stringField.FindName("InputBox");
        var stringGrid = (Grid)stringField.Content;
        Require(stringInput.Text == "Initial" && ReferenceEquals(stringField.DataContext, stringSource), "String fields must preserve DataContext and initial bindings.");
        stringInput.Text = "Fire Rune";
        Require(stringSource.Text == "Fire Rune", "Typed strings must update the source.");
        stringField.SetCurrentValue(textProperty, "Normalized.png");
        Require(stringSource.Text == "Normalized.png" && stringInput.Text == "Normalized.png" && BindingOperations.IsDataBound(stringField, textProperty), "Programmatic normalization must preserve two-way binding.");
        Require(stringGrid.ColumnDefinitions[0].ActualWidth == 200 && ((Border)stringField.FindName("InputBorder")).ActualHeight == 24,
            "String fields must align with the 200px name column and 24px numeric row height.");
        stringField.SetCurrentValue(Property(stringType, "LabelWidth"), 120d);
        var stringBorder = (Border)stringField.FindName("InputBorder");
        Require(stringBorder.ActualWidth == 128, "String input must default to a fixed 128px width.");
        stringInput.Text = new string('W', 100);
        stringField.Measure(new Size(800, 40)); stringField.Arrange(new Rect(0, 0, 800, 40)); stringField.UpdateLayout();
        Require(stringBorder.ActualWidth == 128, "Long text and a wide parent must not stretch the string input.");
        stringField.SetCurrentValue(Property(stringType, "InputWidth"), 160d);
        stringField.Measure(new Size(800, 40)); stringField.Arrange(new Rect(0, 0, 800, 40)); stringField.UpdateLayout();
        Require(stringBorder.ActualWidth == 160, "String input-width overrides must work.");
        stringField.Measure(new Size(500, 32)); stringField.Arrange(new Rect(0, 0, 500, 32)); stringField.UpdateLayout();
        Require(stringGrid.ColumnDefinitions[0].ActualWidth == 120, "String label-width overrides must work.");
        int commits = 0;
        stringType.GetEvent("TextCommitted")!.AddEventHandler(stringField, new RoutedEventHandler((_, _) => commits++));
        stringType.GetMethod("InputFinished", BindingFlags.NonPublic | BindingFlags.Instance)!.Invoke(stringField,
            new object[] { stringInput, new System.Windows.Input.KeyboardFocusChangedEventArgs(System.Windows.Input.Keyboard.PrimaryDevice, 0, stringInput, null) });
        Require(commits == 1, "String commit events must be available to callers.");
        stringField.IsEnabled = false; Require(!stringInput.IsEnabled, "Disabled string controls must disable typing.");
        Console.WriteLine("PASS: shared Vector2/3 and string controls, two-way bindings, input/drag, sizing/alignment, numeric limits, highlight state and string commit events. No editor/game launched.");
    }
}
