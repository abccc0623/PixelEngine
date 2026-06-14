using Microsoft.Win32;
using System.IO;
using System.Windows;

namespace PixelTool
{
    public partial class ProjectStartupWindow : Window
    {
        private readonly string _previousProjectPath;
        private bool _pathSelected;

        public ProjectStartupWindow()
        {
            InitializeComponent();

            _previousProjectPath = ProjectPathService.LoadPreviousProjectPath();
            bool hasPreviousPath = !string.IsNullOrEmpty(_previousProjectPath);
            PreviousPathButton.IsEnabled = hasPreviousPath;
            PreviousPathButton.Content = hasPreviousPath
                ? "Start With Previous Path"
                : "No Previous Project";
            PreviousPathButton.ToolTip = hasPreviousPath
                ? _previousProjectPath
                : "Create a new project first.";
            PreviousPathText.Text = hasPreviousPath
                ? _previousProjectPath
                : "No previous project is available.";

        }

        private void StartWithPreviousPath(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(_previousProjectPath) || !Directory.Exists(_previousProjectPath)) return;
            CompleteSelection(_previousProjectPath);
        }

        private void CreateNewAssetFolder(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFolderDialog
            {
                Title = "Select a parent folder for the new project",
                Multiselect = false
            };

            if (dialog.ShowDialog(this) != true) return;

            var nameWindow = new ProjectNameWindow(dialog.FolderName) { Owner = this };
            if (nameWindow.ShowDialog() != true) return;

            string projectPath = nameWindow.ProjectPath;
            if (Directory.Exists(projectPath))
            {
                PixelMessageBox.Show("A folder with this project name already exists.", "Create Project",
                    MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            ProjectPathService.CreateProject(projectPath);
            CompleteSelection(projectPath);
        }

        private void OpenExistingProject(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFolderDialog
            {
                Title = "Select an existing PixelTool project folder",
                Multiselect = false
            };

            if (dialog.ShowDialog(this) != true) return;

            if (!ProjectPathService.IsProjectRoot(dialog.FolderName))
            {
                PixelMessageBox.Show(
                    "The selected folder is not a valid PixelTool project.\nIt must contain Asset, Engine, and Editor folders.",
                    "Open Project",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning);
                return;
            }

            CompleteSelection(dialog.FolderName);
        }

        private void CompleteSelection(string projectRootPath)
        {
            ProjectPathService.UseProjectPath(projectRootPath);
            _pathSelected = true;
            DialogResult = true;
        }

    }
}
