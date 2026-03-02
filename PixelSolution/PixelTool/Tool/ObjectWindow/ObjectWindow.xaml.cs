using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.InteropServices;
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
using static PixelTool.ConsoleWindow;
using static PixelTool.ObjectWindow;

namespace PixelTool
{
    /// <summary>
    /// ObjectWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ObjectWindow : UserControl
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void SceneChangeDelegate();

        private static SceneChangeDelegate? _sceneChangeDelegate;

        // C++ DLL 함수 가져오기 (DLL 이름이 PixelEngine.dll 이라고 가정)
        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void RegisterSceneObjectChange(SceneChangeDelegate callback);

        public static ObservableCollection<GameObject> SceneObjects { get; set; } = new ObservableCollection<GameObject>();
        public ObjectWindow()
        {
            InitializeComponent();
            this.DataContext = this;

            _sceneChangeDelegate = new SceneChangeDelegate(SceneChangeCallBack);
            RegisterSceneObjectChange(_sceneChangeDelegate);
        }

        void SceneChangeCallBack()
        {
            UpdateScene();
        }

        public static void UpdateScene()
        {
            SceneObjects.Clear();
            int maxCount = 0;
            var swigPtrPtr = PixelEngine.GetAllSceneObjects(out maxCount);
        
            if (maxCount <= 0 || swigPtrPtr == null) return;
        
            IntPtr baseAddr = SWIGTYPE_p_p_GameObject.getCPtr(swigPtrPtr).Handle;
        
            for (int i = 0; i < maxCount; i++)
            {
                IntPtr objAddr = Marshal.ReadIntPtr(baseAddr, i * IntPtr.Size);
                GameObject obj = new GameObject(objAddr, false);
                SceneObjects.Add(obj);
            }
        }
        private void OnHierarchySelectionChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            // e.NewValue가 사용자가 방금 클릭한(선택한) 객체야
            var selectedObject = e.NewValue as GameObject;

            if (selectedObject != null)
            {
                // 아까 만든 인스펙터 갱신 함수 호출!
                InspectorWindow.RefreshInspector(selectedObject);
            }
        }
    }
}
