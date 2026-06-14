using System.Windows;

namespace PixelTool
{
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            ShutdownMode = ShutdownMode.OnExplicitShutdown;

            var startupWindow = new ProjectStartupWindow();
            bool? result = startupWindow.ShowDialog();
            if (result != true || string.IsNullOrWhiteSpace(ProjectPathService.ProjectRootPath))
            {
                Shutdown();
                return;
            }

            var mainWindow = new MainWindow();
            MainWindow = mainWindow;
            mainWindow.Show();
            ShutdownMode = ShutdownMode.OnMainWindowClose;
        }
    }
}
