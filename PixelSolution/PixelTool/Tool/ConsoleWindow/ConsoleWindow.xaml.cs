using System;
using System.Collections.Generic;
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
using System.Windows.Threading;

namespace PixelTool
{
    /// <summary>
    /// ConsoleWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ConsoleWindow : UserControl
    {
        // 1. C++ DLL의 함수 포인터와 일치하는 델리게이트 선언
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void LogDelegate(string message, int level);

        // 2. GC(가비지 컬렉터)가 콜백을 삭제하지 못하도록 static 변수로 들고 있음
        private static LogDelegate? _logCallback;

        // C++ DLL 함수 가져오기 (DLL 이름이 PixelEngine.dll 이라고 가정)
        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void RegisterLogCallback(LogDelegate callback);

        static ConsoleWindow console;

        public ConsoleWindow()
        {
            InitializeComponent();

            // 3. 콜백 인스턴스 생성 및 등록
            _logCallback = new LogDelegate(OnNativeLogReceived);
            RegisterLogCallback(_logCallback);
            EngineLogView.SelectionChanged += EngineLogView_SelectionChanged;
            console = GlobalFunction.GetDockedWindow<ConsoleWindow>();
        }

        // 4. C++에서 호출되는 실제 함수
        private void OnNativeLogReceived(string message, int level)
        {
            LogMessage(message, level);
        }

        public static void LogMessage(string message, int level)
        {
            if(console == null)
            {
                console = GlobalFunction.GetDockedWindow<ConsoleWindow>();
            }
            if (console == null) return;

            console.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() =>
            {
                string timeTag = DateTime.Now.ToString("HH:mm:ss");
                string levelTag = level == 0 ? "[INFO]" : (level == 1 ? "[WARN]" : "[ERR ]");
            
                // ListBox에 로그 추가
                ListBoxItem item = new ListBoxItem();
                string logEntry = $"[{timeTag}]{levelTag} {message}";
                item.FontFamily = new System.Windows.Media.FontFamily("Consolas, Malgun Gothic");
                switch (level)
                {
                    case 0: // INFO
                        item.Foreground = Brushes.White;
                        break;
                    case 1: // WARN
                        item.Foreground = Brushes.Yellow;
                        break;
                    case 2: // ERR
                        item.Foreground = Brushes.Red;
                        break;
                    default:
                        item.Foreground = Brushes.Gray;
                        break;
                }
                var logView = console.EngineLogView;
                item.Content = logEntry;
                logView.Items.Add(item);
            
                // 자동 스크롤: 가장 최근 로그로 이동
                if (console.EngineLogView.Items.Count > 0)
                {
                    logView.ScrollIntoView(logView.Items[logView.Items.Count - 1]);
                }
            }));
        }


        private void Clear(object sender, RoutedEventArgs e)
        {
            EngineLogView.Items.Clear();
        }

        private void CopyMenuItem_Click(object sender, RoutedEventArgs e)
        {
            Copy();
        }

        private void EngineLogView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            Copy();
        }

        void Copy()
        {
            // 1. 선택된 아이템이 있는지 확인
            if (EngineLogView.SelectedItem == null) return;

            string logText = "";

            // 2. 아이템 타입에 따라 텍스트 추출
            if (EngineLogView.SelectedItem is ListBoxItem item)
            {
                // ListBoxItem으로 넣었을 경우
                logText = item.Content.ToString();
            }
            else
            {
                // 문자열로 넣었을 경우
                logText = EngineLogView.SelectedItem.ToString();
            }

            // 3. 클립보드에 복사
            if (!string.IsNullOrEmpty(logText))
            {
                Clipboard.SetText(logText);
                EngineLogView.SelectedIndex = -1;
            }
        }
    }
}
