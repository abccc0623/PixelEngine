using System;
using System.Collections.Generic;
using System.Linq;
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

namespace PixelTool
{
    /// <summary>
    /// ToolbarWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ToolbarWindow : UserControl
    {
        public ToolbarWindow()
        {
            InitializeComponent();
        }

        private void DefaultGameObject(object sender, RoutedEventArgs e)
        {
            var go = PixelEngine.CreateGameObject("Default");
        }

        private void QuadObject(object sender, RoutedEventArgs e)
        {
            PixelEngine.LoadTexture("./Asset/test.png");
            var go = PixelEngine.CreateGameObject("DefaultQuad");
            go.AddModule(MODULE_TYPE.Renderer2D);

            var r = Renderer2D.SafeCast(go.GetModule(MODULE_TYPE.Renderer2D));
            if (r!= null)
            {
                r.SetTexture("test");
            }
        }
        private void LuaObject(object sender, RoutedEventArgs e)
        {
            var go = PixelEngine.CreateGameObject("DefaultLua");
            go.AddModule(MODULE_TYPE.LuaScript);
        }
        private void CameraObject(object sender, RoutedEventArgs e)
        {
            var go = PixelEngine.CreateGameObject("DefaultLua");
            go.AddModule(MODULE_TYPE.DebugCamera);
        }
    }
}
