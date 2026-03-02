using System;
using System.Collections.ObjectModel;
using System.Windows.Controls;
using System.Windows.Media;


namespace PixelTool
{
    /// <summary>
    /// InspectorWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class InspectorWindow : UserControl
    {
        public static ObservableCollection<UserControl> ModuleUIList { get; set; } = new ObservableCollection<UserControl>();

        public InspectorWindow()
        {
            InitializeComponent();
            ModuleContainer.ItemsSource = ModuleUIList;
            CompositionTarget.Rendering += OnEditorUpdate;
        }

        private void OnEditorUpdate(object sender, EventArgs e)
        {
            // 현재 인스펙터에 표시 중인 모든 모듈 데이터를 순회
            foreach (var ui in ModuleUIList)
            {
                if (ui.DataContext is TransformData data)
                {
                    data.RefreshFromCore();
                }
            }
        }



        private readonly static Dictionary<int, Func<UserControl>> _moduleRegistry = new()
        {
            { 1, () => new TransformView() },
        };

        static public void RefreshInspector(GameObject selectObject)
        {
            ModuleUIList.Clear();
            if (selectObject.HasModule(MODULE_TYPE.Transform))
            {
                int typeId = (int)MODULE_TYPE.Transform;
                if (_moduleRegistry.ContainsKey(typeId))
                {
                    var transform = Transform.SafeCast(selectObject.GetModule(MODULE_TYPE.Transform));
                    var uiData = new TransformData(transform);
                    var uiView = _moduleRegistry[(int)MODULE_TYPE.Transform]();
                    
                    uiView.DataContext = uiData;
                    ModuleUIList.Add(uiView);
                }
            }
        }
    }
}
