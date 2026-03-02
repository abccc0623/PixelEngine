using Microsoft.Win32;
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


        ///-----------------Create----------------------
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
            var go = PixelEngine.CreateGameObject("Camera");
            go.AddModule(MODULE_TYPE.DebugCamera);
        }
        //-----------------Save----------------------
        private void SceneSave(object sender, RoutedEventArgs e)
        {
            PixelEngine.SaveScene();
        }
        private void LoadScene(object sender, RoutedEventArgs e)
        {

            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Scene Files (*.scene)|*.scene;*.Scene";

            // 2. 초기 디렉토리 (프로젝트 경로가 있다면 지정)
            openFileDialog.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory + "Asset\\";

            // 3. 창 띄우기 및 결과 확인
            if (openFileDialog.ShowDialog() == true)
            {
                string baseDir = AppDomain.CurrentDomain.BaseDirectory;
                string fullPath = openFileDialog.FileName;
                Uri baseUri = new Uri(baseDir);
                Uri fullUri = new Uri(fullPath);

                Uri relativeUri = baseUri.MakeRelativeUri(fullUri);

                string relativePath = Uri.UnescapeDataString(relativeUri.ToString());
                relativePath =  "./" + relativePath.Replace('\\', '/');

               PixelEngine.LoadScene(relativePath);
            }


            //PixelEngine.SaveScene();
        }
    }
}
