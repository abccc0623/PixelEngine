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

namespace PixelTool
{
    /// <summary>
    /// ToolbarWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ToolbarWindow : UserControl
    {
        int createSceneindex = 0;
        public ToolbarWindow()
        {
            InitializeComponent();
        }

        public void CreateMainLua()
        {
            string luafilePath = "./Asset/main.lua";
            if (File.Exists(luafilePath))
            {
                var result = MessageBox.Show(
                 "main.lua 파일은 Asset폴더 하위로 한개만 존재할 수 있습니다. 기존 main파일을 삭제 하겠습니까?",
                 "이미 존재 하는 파일",
                  MessageBoxButton.YesNo,
                  MessageBoxImage.Warning);

                if (result == MessageBoxResult.No)
                {
                    return;
                }
            }
            string content = "--게임 로직에 진입점 입니다 \n";
            content += "function Main() \n";
            content += "\n";
            content += "end\n";
            File.WriteAllText(luafilePath, content, Encoding.UTF8);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if(luaWindow != null)
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
            string luafilePath = "./Asset/NewScene.scene";
            while (File.Exists(luafilePath))
            {
                luafilePath = $"./Asset/NewScene{createSceneindex}.scene";
                createSceneindex++;
            }
            string content = "--게임 씬 파일입니다. \n";
            content += "local Scene = {} \n";
            content += "\n";
            content += "--씬이 변경될 때 한번 실행됩니다. \n";
            content += "function Scene:Start() \n";
            content += "\n";
            content += "end\n";
            content += "\n";
            content += "--매 프래임 실행됩니다. \n";
            content += "function Scene:Update() \n";
            content += "\n";
            content += "end\n";
            content += "\n";
            content += "--씬이 변경되거나 삭제될 때 실행됩니다.\n";
            content += "function Scene:Release() \n";
            content += "\n";
            content += "end\n";
            content += "return Scene\n";
            File.WriteAllText(luafilePath, content, Encoding.UTF8);

            var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
            if(luaWindow != null)
            {
                luaWindow.OpenFile(luafilePath);
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


        private void QuadObject(object sender, RoutedEventArgs e)
        {
            // PixelEngine.LoadTexture("./Asset/test.png");
            // var go = PixelEngine.CreateGameObject("DefaultQuad");
            // go.AddModule(MODULE_TYPE.Renderer2D);
            //
            // var r = Renderer2D.SafeCast(go.GetModule(MODULE_TYPE.Renderer2D));
            // if (r!= null)
            // {
            //     r.SetTexture("test");
            // }
        }
        private void LuaObject(object sender, RoutedEventArgs e)
        {
            //var go = PixelEngine.CreateGameObject("DefaultLua");
            //go.AddModule(MODULE_TYPE.LuaScript);
        }
        private void CameraObject(object sender, RoutedEventArgs e)
        {
            //var go = PixelEngine.CreateGameObject("Camera");
            //go.AddModule(MODULE_TYPE.DebugCamera);
        }
        //-----------------Save----------------------
        private void SceneSave(object sender, RoutedEventArgs e)
        {
            //PixelEngine.SaveScene();
        }
        private void LoadScene(object sender, RoutedEventArgs e)
        {

            //OpenFileDialog openFileDialog = new OpenFileDialog();
            //openFileDialog.Filter = "Scene Files (*.scene)|*.scene;*.Scene";
            //
            //// 2. 초기 디렉토리 (프로젝트 경로가 있다면 지정)
            //openFileDialog.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory + "Asset\\";
            //
            //// 3. 창 띄우기 및 결과 확인
            //if (openFileDialog.ShowDialog() == true)
            //{
            //    string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            //    string fullPath = openFileDialog.FileName;
            //    Uri baseUri = new Uri(baseDir);
            //    Uri fullUri = new Uri(fullPath);
            //
            //    Uri relativeUri = baseUri.MakeRelativeUri(fullUri);
            //
            //    string relativePath = Uri.UnescapeDataString(relativeUri.ToString());
            //    relativePath =  "./" + relativePath.Replace('\\', '/');
            //
            //   PixelEngine.LoadScene(relativePath);
            //}


            //PixelEngine.SaveScene();
        }



        private void CreateModuleLuaFile(object sender, RoutedEventArgs e)
        {
            string newNameOnly = Microsoft.VisualBasic.Interaction.InputBox(
               "새 모듈 파일의 이름을 입력해주세요",
               "새로운 모듈 파일 만들기",
               "");

            if (string.IsNullOrWhiteSpace(newNameOnly)) return;
            string directoryPath = "./Asset/";
            string fullPath = System.IO.Path.Combine(directoryPath, newNameOnly);

            if (System.IO.File.Exists(fullPath))
            {
                MessageBox.Show("이미 같은 이름의 파일이 존재합니다!", "알림", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

           
            string content = "";
            content += "---@class LuaModule \n";
            content += "---@field gameObject GameObject\n";
            content += "---@field transform Transform\n";
            content += "---@class self : LuaModule \n";
            content += "self = self or {} \n";

            content += "function  self:Awake() \n";
            content += "\t\n";
            content += "end  \n\n";
            content += "function  self:Start()  \n";
            content += "\t\n";
            content += "end  \n\n";
            content += "function  self:Update(dTime)   \n";
            content += "\t\n";
            content += "end  \n\n";

            content = content.Replace("{{TYPE_NAME}}", newNameOnly);
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
    }
}
