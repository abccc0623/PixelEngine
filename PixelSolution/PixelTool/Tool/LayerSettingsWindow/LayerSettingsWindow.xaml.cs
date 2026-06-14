using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Newtonsoft.Json;

namespace PixelTool
{
    public partial class LayerSettingsWindow : UserControl
    {
        private const int MaxLayers = 32;
        private const string LayerFileType = "LayerMatrix";
        private readonly string _layerFilePath;

        public Action? Close;
        public ObservableCollection<string> Layers { get; } = new();
        public bool[,] CollisionMatrix { get; private set; } = new bool[0, 0];

        public LayerSettingsWindow()
        {
            InitializeComponent();
            _layerFilePath = ProjectPathService.GetEngineFilePath("LayerMatrix.json");
            LoadOrCreateLayerFile();
        }

        private void LoadOrCreateLayerFile()
        {
            if (!File.Exists(_layerFilePath))
            {
                CreateDefaultLayers();
                SaveLayerFile();
                return;
            }

            try
            {
                LayerMatrixFile? data = JsonConvert.DeserializeObject<LayerMatrixFile>(
                    File.ReadAllText(_layerFilePath));

                if (data == null || data.FileType != LayerFileType || data.Layers.Count == 0)
                {
                    throw new InvalidDataException("The file is not a Layer Matrix file.");
                }

                int size = Math.Min(data.Layers.Count, MaxLayers);
                for (int index = 0; index < size; index++)
                {
                    Layers.Add(data.Layers[index]);
                }

                CollisionMatrix = new bool[size, size];
                for (int row = 0; row < size; row++)
                {
                    for (int col = 0; col < size; col++)
                    {
                        bool value = row < data.CollisionMatrix.Count &&
                                     col < data.CollisionMatrix[row].Count &&
                                     data.CollisionMatrix[row][col];
                        CollisionMatrix[row, col] = value;
                    }
                }

                RenderMatrix();
            }
            catch (Exception ex)
            {
                PixelMessageBox.Show(
                    $"LayerMatrix.json could not be loaded. A default Layer Matrix will be created.\n\n{ex.Message}",
                    "Layer Settings",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning);
                CreateDefaultLayers();
                SaveLayerFile();
            }
        }

        private void CreateDefaultLayers()
        {
            Layers.Clear();
            CollisionMatrix = new bool[0, 0];
            AddLayerInternal("Default", false);
            AddLayerInternal("Player", false);
            AddLayerInternal("Ground", false);
        }

        private void SaveLayerFile()
        {
            Directory.CreateDirectory(ProjectPathService.EnginePath);

            var data = new LayerMatrixFile
            {
                Layers = Layers.ToList(),
                CollisionMatrix = Enumerable.Range(0, Layers.Count)
                    .Select(row => Enumerable.Range(0, Layers.Count)
                        .Select(col => CollisionMatrix[row, col])
                        .ToList())
                    .ToList()
            };

            File.WriteAllText(
                _layerFilePath,
                JsonConvert.SerializeObject(data, Formatting.Indented));
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
                PixelMessageBox.Show("Layer name is empty.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            if (Layers.Count >= MaxLayers)
            {
                PixelMessageBox.Show($"Only up to {MaxLayers} layers are supported.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (Layers.Any(name => string.Equals(name, layerName, StringComparison.OrdinalIgnoreCase)))
            {
                PixelMessageBox.Show("A layer with the same name already exists.", "Layer Settings", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            AddLayerInternal(layerName);
            LayerNameTextBox.Clear();
            LayerNameTextBox.Focus();
        }

        private void AddLayerInternal(string layerName, bool save = true)
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
            if (save)
            {
                SaveLayerFile();
            }
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
            SaveLayerFile();
        }

        private sealed class LayerMatrixFile
        {
            [JsonProperty("fileType", Order = 0)]
            public string FileType { get; set; } = LayerFileType;

            [JsonProperty("layers", Order = 1)]
            public List<string> Layers { get; set; } = new();

            [JsonProperty("collisionMatrix", Order = 2)]
            public List<List<bool>> CollisionMatrix { get; set; } = new();
        }
    }
}
