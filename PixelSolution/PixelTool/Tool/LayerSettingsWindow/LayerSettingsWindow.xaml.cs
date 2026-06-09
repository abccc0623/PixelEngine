using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace PixelTool
{
    public partial class LayerSettingsWindow : UserControl
    {
        private const int MaxLayers = 32;

        public Action? Close;
        public ObservableCollection<string> Layers { get; } = new();
        public bool[,] CollisionMatrix { get; private set; } = new bool[0, 0];

        public LayerSettingsWindow()
        {
            InitializeComponent();

            AddLayerInternal("Default");
            AddLayerInternal("Player");
            AddLayerInternal("Ground");
        }

        private void AddLayerButton_Click(object sender, RoutedEventArgs e)
        {
            TryAddLayerFromInput();
        }

        private void LayerNameTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Enter)
            {
                return;
            }

            TryAddLayerFromInput();
            e.Handled = true;
        }

        private void TryAddLayerFromInput()
        {
            string layerName = LayerNameTextBox.Text.Trim();

            if (string.IsNullOrWhiteSpace(layerName))
            {
                MessageBox.Show("Layer name is empty.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            if (Layers.Count >= MaxLayers)
            {
                MessageBox.Show($"Only up to {MaxLayers} layers are supported.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (Layers.Any(name => string.Equals(name, layerName, StringComparison.OrdinalIgnoreCase)))
            {
                MessageBox.Show("A layer with the same name already exists.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            AddLayerInternal(layerName);
            LayerNameTextBox.Clear();
            LayerNameTextBox.Focus();
        }

        private void AddLayerInternal(string layerName)
        {
            int oldSize = Layers.Count;
            int newSize = oldSize + 1;
            bool[,] nextMatrix = new bool[newSize, newSize];

            for (int row = 0; row < oldSize; row++)
            {
                for (int col = 0; col < oldSize; col++)
                {
                    nextMatrix[row, col] = CollisionMatrix[row, col];
                }
            }

            for (int index = 0; index < newSize; index++)
            {
                nextMatrix[oldSize, index] = true;
                nextMatrix[index, oldSize] = true;
            }

            Layers.Add(layerName);
            CollisionMatrix = nextMatrix;

            RenderMatrix();
        }

        private void RenderMatrix()
        {
            MatrixGrid.Children.Clear();
            MatrixGrid.RowDefinitions.Clear();
            MatrixGrid.ColumnDefinitions.Clear();

            int size = Layers.Count;
            if (size == 0)
            {
                return;
            }

            MatrixGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(180) });
            for (int col = 0; col < size; col++)
            {
                MatrixGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
            }

            MatrixGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            for (int row = 0; row < size; row++)
            {
                MatrixGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            }

            AddHeaderCell(0, 0, "Layer");

            for (int index = 0; index < size; index++)
            {
                AddHeaderCell(0, index + 1, Layers[index]);
                AddHeaderCell(index + 1, 0, Layers[index]);
            }

            for (int row = 0; row < size; row++)
            {
                for (int col = 0; col < size; col++)
                {
                    MatrixGrid.Children.Add(CreateCheckBox(row, col));
                }
            }
        }

        private void AddHeaderCell(int row, int col, string text)
        {
            Border border = new()
            {
                BorderBrush = Application.Current.Resources["PixelBorderBrush"] as Brush,
                BorderThickness = new Thickness(1),
                Background = Application.Current.Resources["PixelBackgroundBrush"] as Brush,
                Padding = new Thickness(10, 8, 10, 8),
                Child = new TextBlock
                {
                    Text = text,
                    Foreground = Application.Current.Resources["PixelTextBrush"] as Brush,
                    FontWeight = FontWeights.SemiBold,
                    VerticalAlignment = VerticalAlignment.Center
                }
            };

            Grid.SetRow(border, row);
            Grid.SetColumn(border, col);
            MatrixGrid.Children.Add(border);
        }

        private FrameworkElement CreateCheckBox(int row, int col)
        {
            CheckBox checkBox = new()
            {
                IsChecked = CollisionMatrix[row, col],
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                Tag = Tuple.Create(row, col),
                Margin = new Thickness(18, 10, 18, 10),
                Foreground = Application.Current.Resources["PixelTextBrush"] as Brush
            };
            checkBox.Click += CollisionCheckBox_Click;

            Border border = new()
            {
                BorderBrush = Application.Current.Resources["PixelBorderBrush"] as Brush,
                BorderThickness = new Thickness(1),
                Background = Application.Current.Resources["PixelBackgroundBrush"] as Brush,
                Child = checkBox
            };

            Grid.SetRow(border, row + 1);
            Grid.SetColumn(border, col + 1);
            return border;
        }

        private void CollisionCheckBox_Click(object sender, RoutedEventArgs e)
        {
            if (sender is not CheckBox checkBox || checkBox.Tag is not Tuple<int, int> position)
            {
                return;
            }

            bool value = checkBox.IsChecked == true;
            int row = position.Item1;
            int col = position.Item2;

            CollisionMatrix[row, col] = value;
            CollisionMatrix[col, row] = value;

            RenderMatrix();
        }
    }
}
