using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class MainWindow : Window
    {
        private readonly Brush _dockBorderBrush = new SolidColorBrush(Color.FromRgb(0x3A, 0x3C, 0x5E));
        private readonly Brush _dockBackgroundBrush = new SolidColorBrush(Color.FromRgb(0x1B, 0x1C, 0x2E));
        private readonly Brush _dockTextBrush = new SolidColorBrush(Color.FromRgb(0xD1, 0xD1, 0xE0));
        private readonly DispatcherTimer _dockThemeTimer;

        public MainWindow()
        {
            InitializeComponent();
            //LuaFileManager.CreateLuaByJIT("PixelTool.LuaCode.JIT.PVector3.lua", "./Asset/Scripts/PVector3.lua");
            IntPtr hWnd = new WindowInteropHelper(this).Handle;

            Loaded += (_, _) => ApplyAvalonDockRuntimeTheme();

            _dockThemeTimer = new DispatcherTimer(DispatcherPriority.Background)
            {
                Interval = TimeSpan.FromMilliseconds(1000)
            };
            _dockThemeTimer.Tick += (_, _) => ApplyAvalonDockRuntimeTheme();
            _dockThemeTimer.Start();
        }

        private void ApplyAvalonDockRuntimeTheme()
        {
            ApplyDockVisualTree(dockManager, false);

            foreach (Window window in Application.Current.Windows)
            {
                if (window == this) continue;
                if (window.GetType().FullName?.Contains("AvalonDock") != true) continue;

                window.Background = _dockBackgroundBrush;
                ApplyDockVisualTree(window, true);
            }
        }

        private void ApplyDockVisualTree(DependencyObject target, bool forceDockScope)
        {
            bool isDockScope = forceDockScope || IsAvalonDockVisual(target);

            if (isDockScope)
            {
                if (target is Border border)
                {
                    border.BorderBrush = _dockBorderBrush;
                    if (IsLightBrush(border.Background))
                        border.Background = _dockBackgroundBrush;
                }
                else if (target is Control control)
                {
                    if (IsLightBrush(control.BorderBrush))
                        control.BorderBrush = _dockBorderBrush;

                    if (IsLightBrush(control.Background))
                        control.Background = _dockBackgroundBrush;

                    if (IsAvalonDockTab(control))
                        control.Foreground = _dockTextBrush;
                }
                else if (target is Panel panel)
                {
                    if (IsLightBrush(panel.Background))
                        panel.Background = _dockBackgroundBrush;
                }
            }

            int count = VisualTreeHelper.GetChildrenCount(target);
            for (int i = 0; i < count; i++)
            {
                ApplyDockVisualTree(VisualTreeHelper.GetChild(target, i), isDockScope);
            }
        }

        private static bool IsAvalonDockVisual(DependencyObject target)
        {
            return target.GetType().FullName?.Contains("AvalonDock") == true;
        }

        private static bool IsAvalonDockTab(Control control)
        {
            string name = control.GetType().Name;
            return name.Contains("TabItem") || name.Contains("LayoutDocument");
        }

        private static bool IsLightBrush(Brush brush)
        {
            if (brush == null) return true;
            if (brush is not SolidColorBrush solid) return false;

            var color = solid.Color;
            return color.R > 180 && color.G > 180 && color.B > 180;
        }
    }
}
