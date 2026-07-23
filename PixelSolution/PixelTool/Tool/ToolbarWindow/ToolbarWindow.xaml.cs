using AvalonDock.Layout;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Media.Animation;

namespace PixelTool
{
    /// <summary>
    /// ToolbarWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ToolbarWindow : UserControl
    {
        int createSceneindex = 0;
        private ToolbarButton _hoveredButton;
        public bool IsPlayMode { get; private set; }

        public ToolbarWindow()
        {
            InitializeComponent();
            Loaded += (_, _) => SelectionIndicator.Visibility = Visibility.Hidden;
        }

        private void EditorPlayToggle_Changed(object sender, RoutedEventArgs e)
        {
            IsPlayMode = EditorPlayToggle.IsChecked == true;

            string modeName = IsPlayMode ? "Play" : "Editor";
            EditorPlayToggle.ToolTip = $"Current Mode: {modeName}";

            if (IsPlayMode)
            {
                PixelEngineNative.EditorNotify(EditorEventType.PlayMode, "");
            }
            else
            {
                PixelEngineNative.EditorNotify(EditorEventType.EditorMode, "");
            }
        }

        private void ToolbarButton_Hovered(object sender, RoutedEventArgs e)
        {
            if (sender is ToolbarButton button)
            {
                _hoveredButton = button;
                MoveHoverIndicator(button);
            }
        }

        private void ToolbarButton_HoverEnded(object sender, RoutedEventArgs e)
        {
            if (sender == _hoveredButton)
            {
                _hoveredButton = null;
                SelectionIndicator.Visibility = Visibility.Hidden;
            }
        }

        private void MoveHoverIndicator(ToolbarButton hoveredButton)
        {
            SelectionIndicator.Visibility = Visibility.Visible;
            double targetY = hoveredButton.TranslatePoint(new Point(0, 0), ToolbarItems).Y + 3;

            var animation = new DoubleAnimation
            {
                To = targetY,
                Duration = TimeSpan.FromMilliseconds(220),
                EasingFunction = new CubicEase { EasingMode = EasingMode.EaseOut }
            };
            SelectionTransform.BeginAnimation(TranslateTransform.YProperty, animation);
        }

        public void CreateMainLua()
        {
            string luafilePath = ProjectPathService.GetAssetFilePath("main.lua");
            if (File.Exists(luafilePath))
            {
                var result = PixelMessageBox.Show(
                 "main.lua 파일은 Asset폴더 하위로 한개만 존재할 수 있습니다. 기존 main파일을 삭제 하겠습니까?",
                 "이미 존재 하는 파일",
                  MessageBoxButton.YesNo,
                  MessageBoxImage.Warning);

                if (result == MessageBoxResult.No)
                {
                    return;
                }
            }
            string content = LuaFileManager.GetFileContent("main.lua");
            var utf8WithBom = new System.Text.UTF8Encoding(true);
            File.WriteAllText(luafilePath, content, utf8WithBom);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if (luaWindow != null)
            {
                luaWindow.OpenFile(luafilePath);
            }
            var findWindow = GlobalFunction.GetDockedWindow<AssetWindow>();
            if (findWindow != null)
            {
                findWindow.Refresh();
            }
        }
        public void CreateSceneLua()
        {
            string luafilePath = ProjectPathService.GetAssetFilePath("NewScene.scene");
            while (File.Exists(luafilePath))
            {
                luafilePath = ProjectPathService.GetAssetFilePath($"NewScene{createSceneindex}.scene");
                createSceneindex++;
            }
            string content = LuaFileManager.GetFileContent("Scene");
            File.WriteAllText(luafilePath, content, Encoding.UTF8);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if (luaWindow != null)
            {
                luaWindow.OpenFile(luafilePath);
            }

            var findWindow = GlobalFunction.GetDockedWindow<AssetWindow>();
            if (findWindow != null)
            {
                findWindow.Refresh();
            }
        }

        public void CreateModule()
        {
            string newNameOnly = PixelPromptDialog.Show(
              "새 모듈 파일의 이름을 입력해주세요",
              "새로운 모듈 파일 만들기",
              "");

            if (string.IsNullOrWhiteSpace(newNameOnly)) return;
            string directoryPath = ProjectPathService.AssetPath;
            string fullPath = System.IO.Path.Combine(directoryPath, newNameOnly);

            if (System.IO.File.Exists(fullPath))
            {
                PixelMessageBox.Show("이미 같은 이름의 파일이 존재합니다!", "알림", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            string content = LuaFileManager.GetFileContent("Module");

            fullPath += ".pxm";
            File.WriteAllText(fullPath, content, Encoding.UTF8);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if (luaWindow != null)
            {
                luaWindow.OpenFile(fullPath);
            }

            var findWindow = GlobalFunction.GetDockedWindow<AssetWindow>();
            if (findWindow != null)
            {
                findWindow.Refresh();
            }
        }

        public void CreateDefaultLua()
        {
            string newNameOnly = PixelPromptDialog.Show(
              "새 Lua 파일의 이름을 입력해주세요",
              "새로운 Lua 파일 만들기",
              "");

            if (string.IsNullOrWhiteSpace(newNameOnly)) return;

            string fileName = System.IO.Path.GetFileNameWithoutExtension(newNameOnly.Trim());
            if (string.IsNullOrWhiteSpace(fileName)) return;

            string fullPath = System.IO.Path.Combine(ProjectPathService.AssetPath, fileName + ".lua");
            if (File.Exists(fullPath))
            {
                PixelMessageBox.Show("이미 같은 이름의 파일이 존재합니다!", "알림", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            string content = LuaFileManager.GetFileContent("Default.lua");
            File.WriteAllText(fullPath, content, Encoding.UTF8);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if (luaWindow != null)
            {
                luaWindow.OpenFile(fullPath);
            }

            var findWindow = GlobalFunction.GetDockedWindow<AssetWindow>();
            if (findWindow != null)
            {
                findWindow.Refresh();
            }
        }


        ///-----------------Create----------------------
        private void CreateMainLuaFile(object sender, RoutedEventArgs e)
        {
            CreateMainLua();
        }
        private void CreateSceneLuaFile(object sender, RoutedEventArgs e)
        {
            CreateSceneLua();
        }

        private void CreateModuleLuaFile(object sender, RoutedEventArgs e)
        {
            CreateModule();
        }

        private void CreateDefaultLuaFile(object sender, RoutedEventArgs e)
        {
            CreateDefaultLua();
        }

        private void SceneSave(object sender, RoutedEventArgs e)
        {

        }

        private void LoadScene(object sender, RoutedEventArgs e)
        {

        }

        private void CreateMaterial(object sender, RoutedEventArgs e)
        {
            var materialWindow = new MaterialWindow();
            var newToolWindow = new AvalonDock.Layout.LayoutAnchorable()
            {
                Title = "Create Material",
                Content = materialWindow, // 만들어둔 UserControl
                FloatingWidth = 500,
                FloatingHeight = 600,
            };
            MainWindow main = Application.Current.MainWindow as MainWindow;
            newToolWindow.AddToLayout(main.dockManager, AnchorableShowStrategy.Right);
            newToolWindow.IsActive = true;
            newToolWindow.Float();
            materialWindow.Close = () =>
            {
                newToolWindow.Close();
            };
        }

        private void CreateCollider(object sender, RoutedEventArgs e)
        {
            var colliderWindow = new ColliderWindow();
            var newToolWindow = new AvalonDock.Layout.LayoutAnchorable()
            {
                Title = "Create Collider",
                Content = colliderWindow, // 만들어둔 UserControl
                FloatingWidth = 500,
                FloatingHeight = 600,
            };
            MainWindow main = Application.Current.MainWindow as MainWindow;
            newToolWindow.AddToLayout(main.dockManager, AnchorableShowStrategy.Right);
            newToolWindow.IsActive = true;
            newToolWindow.Float();
            colliderWindow.Close = () =>
            {
                newToolWindow.Close();
            };
        }

        private void OpenLayerSettings(object sender, RoutedEventArgs e)
        {
            var layerSettingsWindow = new LayerSettingsWindow();
            var newToolWindow = new LayoutAnchorable()
            {
                Title = "Physics Layers",
                Content = layerSettingsWindow,
                FloatingWidth = 900,
                FloatingHeight = 700,
            };
            MainWindow main = Application.Current.MainWindow as MainWindow;
            newToolWindow.AddToLayout(main.dockManager, AnchorableShowStrategy.Right);
            newToolWindow.IsActive = true;
            newToolWindow.Float();
            layerSettingsWindow.Close = () =>
            {
                newToolWindow.Close();
            };
        }

        private void ResetEditorLayout(object sender, RoutedEventArgs e)
        {
            if (Application.Current.MainWindow is MainWindow mainWindow)
            {
                mainWindow.ResetDockLayout();
            }
        }

        private void UpdateEngineFiles(object sender, RoutedEventArgs e)
        {
            string EnginePath = ProjectPathService.EnginePath;
            if (string.IsNullOrWhiteSpace(EnginePath))
            {
                PixelMessageBox.Show("Engine path is not configured.");
                return;
            }

            string targetPath = EnginePath.Replace("\\", "/");
            PixelEngineNative.EditorNotify(EditorEventType.GenerateEngineFile, targetPath);
            PixelMessageBox.Show("Engine update completed. PixelTool will restart.", "Engine Update");

            string? executablePath = Environment.ProcessPath;
            if (string.IsNullOrWhiteSpace(executablePath))
            {
                PixelMessageBox.Show("PixelTool could not be restarted automatically.", "Engine Update");
                return;
            }

            System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
            {
                FileName = executablePath,
                WorkingDirectory = Environment.CurrentDirectory,
                UseShellExecute = true
            });
            Application.Current.Shutdown();
        }
    }
}
