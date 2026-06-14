using AvalonDock.Layout.Serialization;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class MainWindow : Window
    {
        private readonly Brush _dockBorderBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0x3A, 0x3C, 0x5E));
        private readonly Brush _dockBackgroundBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0x1B, 0x1C, 0x2E));
        private readonly Brush _dockTextBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0xD1, 0xD1, 0xE0));
        private readonly DispatcherTimer _dockThemeTimer;
        private readonly Dictionary<string, object> _layoutContents;
        private readonly string _layoutFilePath;
        private readonly string _defaultLayout;

        public MainWindow()
        {
            InitializeComponent();

            _layoutContents = new Dictionary<string, object>
            {
                ["SceneView"] = SceneViewContent,
                ["LuaEditor"] = LuaEditorContent,
                ["Asset"] = AssetContent,
                ["Log"] = LogContent
            };
            _layoutFilePath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "PixelTool",
                "Layout.xml");
            _defaultLayout = SerializeCurrentLayout();

            IntPtr hWnd = new WindowInteropHelper(this).Handle;
            Loaded += (_, _) =>
            {
                LoadSavedLayout();
                ApplyAvalonDockRuntimeTheme();
            };
            Closing += MainWindow_Closing;

            _dockThemeTimer = new DispatcherTimer(DispatcherPriority.Background)
            {
                Interval = TimeSpan.FromMilliseconds(1000)
            };
            _dockThemeTimer.Tick += (_, _) => ApplyAvalonDockRuntimeTheme();
            _dockThemeTimer.Start();
        }

        private void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            SaveCurrentLayout();
        }

        private string SerializeCurrentLayout()
        {
            var serializer = new XmlLayoutSerializer(dockManager);
            using var writer = new StringWriter();
            serializer.Serialize(writer);
            return writer.ToString();
        }

        private void DeserializeLayout(string layoutXml)
        {
            var serializer = new XmlLayoutSerializer(dockManager);
            serializer.LayoutSerializationCallback += (_, e) =>
            {
                if (e.Model?.ContentId != null && _layoutContents.TryGetValue(e.Model.ContentId, out object content))
                {
                    e.Content = content;
                }
                else
                {
                    e.Cancel = true;
                }
            };

            using var reader = new StringReader(layoutXml);
            serializer.Deserialize(reader);
        }

        private void LoadSavedLayout()
        {
            if (!File.Exists(_layoutFilePath)) return;

            try
            {
                DeserializeLayout(File.ReadAllText(_layoutFilePath));
            }
            catch
            {
                DeserializeLayout(_defaultLayout);
            }
        }

        private void SaveCurrentLayout()
        {
            try
            {
                string directory = Path.GetDirectoryName(_layoutFilePath);
                if (!string.IsNullOrEmpty(directory)) Directory.CreateDirectory(directory);
                File.WriteAllText(_layoutFilePath, SerializeCurrentLayout());
            }
            catch
            {
                // Layout persistence must not prevent the editor from closing.
            }
        }

        public void ResetDockLayout()
        {
            DeserializeLayout(_defaultLayout);
            ApplyAvalonDockRuntimeTheme();

            try
            {
                if (File.Exists(_layoutFilePath)) File.Delete(_layoutFilePath);
            }
            catch
            {
            }
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
                    if (IsLightBrush(border.Background)) border.Background = _dockBackgroundBrush;
                }
                else if (target is Control control)
                {
                    if (IsLightBrush(control.BorderBrush)) control.BorderBrush = _dockBorderBrush;
                    if (IsLightBrush(control.Background)) control.Background = _dockBackgroundBrush;
                    if (IsAvalonDockTab(control)) control.Foreground = _dockTextBrush;
                }
                else if (target is Panel panel)
                {
                    if (IsLightBrush(panel.Background)) panel.Background = _dockBackgroundBrush;
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
