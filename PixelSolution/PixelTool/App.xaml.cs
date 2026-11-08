using System.Windows;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace PixelTool
{
    public partial class App : Application
    {
        private PixelToolMcpServer? _mcpServer;
        internal bool IsMcpRunning => _mcpServer?.IsRunning == true;
        internal ProcessStartInfo? ProjectRestart { get; set; }

        internal static ProcessStartInfo CreateProjectRestart()
        {
            string executable = Environment.ProcessPath ?? throw new IOException("실행 중인 에디터 경로를 찾을 수 없습니다.");
            if (!File.Exists(executable)) throw new FileNotFoundException("에디터 실행 파일을 찾을 수 없습니다.", executable);
            var start = new ProcessStartInfo(executable) { WorkingDirectory = AppContext.BaseDirectory, UseShellExecute = false, CreateNoWindow = true };
            if (Path.GetFileNameWithoutExtension(executable).Equals("dotnet", StringComparison.OrdinalIgnoreCase))
            {
                string entry = Assembly.GetEntryAssembly()?.Location ?? "";
                if (!File.Exists(entry)) throw new IOException("에디터 시작 어셈블리를 찾을 수 없습니다.");
                start.ArgumentList.Add(entry);
            }
            return start;
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            ShutdownMode = ShutdownMode.OnExplicitShutdown;

            var startupWindow = new ProjectStartupWindow();
            EditorColors.AttachTitleBar(startupWindow);
            bool? result = startupWindow.ShowDialog();
            if (result != true || string.IsNullOrWhiteSpace(ProjectPathService.ProjectRootPath))
            {
                Shutdown();
                return;
            }

            var mainWindow = new MainWindow();
            EditorColors.AttachTitleBar(mainWindow);
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
            if (ProjectRestart is { } restart)
            {
                ProjectRestart = null;
                try { Process.Start(restart)?.Dispose(); }
                catch (Exception exception)
                {
                    MessageBox.Show("프로젝트 선택창을 자동으로 열지 못했습니다. PixelTool을 다시 실행해 주세요.\n" + exception.Message,
                        "프로젝트 변경", MessageBoxButton.OK, MessageBoxImage.Warning);
                }
            }
        }
    }
}
