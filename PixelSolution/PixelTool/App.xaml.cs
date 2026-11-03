using System.Windows;

namespace PixelTool
{
    public partial class App : Application
    {
        private PixelToolMcpServer? _mcpServer;
        internal bool IsMcpRunning => _mcpServer?.IsRunning == true;

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
            _mcpServer = new PixelToolMcpServer();
            try
            {
                _mcpServer.StartAsync().GetAwaiter().GetResult();
            }
            catch (Exception exception)
            {
                _mcpServer = null;
                PixelMessageBox.Show("MCP could not start. PixelTool will continue without MCP.\n\n" + exception.Message,
                    "PixelTool MCP", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        protected override void OnExit(ExitEventArgs e)
        {
            try
            {
                // Run cleanup off the dispatcher and bound the entire operation,
                // including DisposeAsync after the server's StopAsync timeout.
                Task.Run(async () =>
                {
                    if (_mcpServer != null) await _mcpServer.DisposeAsync().ConfigureAwait(false);
                }).WaitAsync(TimeSpan.FromSeconds(4)).GetAwaiter().GetResult();
            }
            catch (Exception exception)
            {
                System.Diagnostics.Debug.WriteLine("MCP shutdown: " + exception.Message);
            }
            base.OnExit(e);
        }
    }
}
