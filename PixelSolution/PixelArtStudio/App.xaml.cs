using System.Windows;
namespace PixelArtStudio;
public partial class App : Application
{
    protected override async void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);
        string mode = BackgroundRunner.Argument(e.Args, "--mode") ?? (e.Args.Contains("--mode") ? "" : "ui");
        if (mode == "ui") { new MainWindow().Show(); return; }
        ShutdownMode = ShutdownMode.OnExplicitShutdown;
        int code = await BackgroundRunner.RunAsync(e.Args, mode);
        Shutdown(code);
    }
}
