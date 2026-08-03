using Microsoft.Win32;
using System.IO;
using System.Windows;

namespace PixelTool
{
    public partial class BuildPlayWindow : Window
    {
        private readonly string _previousPath;

        public string OutputRootPath { get; private set; } = string.Empty;

        public BuildPlayWindow()
        {
            InitializeComponent();
            _previousPath = ProjectPathService.LoadBuildPlayPath();
            PreviousPathButton.IsEnabled = !string.IsNullOrWhiteSpace(_previousPath);
            PreviousPathText.Text = PreviousPathButton.IsEnabled ? _previousPath : "저장된 경로가 없습니다.";
        }

        private void SaveToNewPath(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFolderDialog
            {
                Title = "Select a folder for Build Play",
                Multiselect = false
            };

            if (dialog.ShowDialog(this) != true) return;

            OutputRootPath = Path.GetFullPath(dialog.FolderName);
            ProjectPathService.SaveBuildPlayPath(OutputRootPath);
            DialogResult = true;
        }

        private void SaveToPreviousPath(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(_previousPath) || !Directory.Exists(_previousPath)) return;
            OutputRootPath = _previousPath;
            DialogResult = true;
        }
    }
}
