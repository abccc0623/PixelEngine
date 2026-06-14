using System.IO;
using System.Windows;
using System.Windows.Input;

namespace PixelTool
{
    public partial class ProjectNameWindow : Window
    {
        private readonly string _parentDirectory;

        public string ProjectPath { get; private set; } = string.Empty;

        public ProjectNameWindow(string parentDirectory)
        {
            InitializeComponent();
            _parentDirectory = parentDirectory;
            Loaded += (_, _) =>
            {
                ProjectNameTextBox.Focus();
                ProjectNameTextBox.SelectAll();
                ValidateProjectName();
            };
        }

        private void ProjectNameTextBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {
            ValidateProjectName();
        }

        private void ProjectNameTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && CreateButton.IsEnabled)
            {
                CreateProject();
                e.Handled = true;
            }
        }

        private void Create_Click(object sender, RoutedEventArgs e)
        {
            CreateProject();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void CreateProject()
        {
            if (!ValidateProjectName()) return;
            ProjectPath = Path.Combine(_parentDirectory, ProjectNameTextBox.Text.Trim());
            DialogResult = true;
        }

        private bool ValidateProjectName()
        {
            if (ProjectNameTextBox == null || CreateButton == null || ValidationText == null) return false;

            string projectName = ProjectNameTextBox.Text.Trim();
            string error = string.Empty;

            if (string.IsNullOrWhiteSpace(projectName))
            {
                error = "Enter a project name.";
            }
            else if (projectName.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0)
            {
                error = "The project name contains invalid characters.";
            }
            else if (Directory.Exists(Path.Combine(_parentDirectory, projectName)))
            {
                error = "A folder with this project name already exists.";
            }

            ValidationText.Text = error;
            CreateButton.IsEnabled = string.IsNullOrEmpty(error);
            return CreateButton.IsEnabled;
        }
    }
}
