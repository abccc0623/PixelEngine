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

        public ConsoleWindow()
        {
            InitializeComponent();

            // 3. 콜백 인스턴스 생성 및 등록
            _logCallback = new LogDelegate(OnNativeLogReceived);
            RegisterLogCallback(_logCallback);
        }

        // 4. C++에서 호출되는 실제 함수
        private void OnNativeLogReceived(string message, int level)
        {
            // 중요: C++ 로그는 별도 스레드에서 올 수 있으므로 UI 스레드로 보냄 (Dispatcher)
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() =>
            {
                string timeTag = DateTime.Now.ToString("HH:mm:ss");
                string levelTag = level == 0 ? "[INFO]" : (level == 1 ? "[WARN]" : "[ERR ]");

                // ListBox에 로그 추가
                string logEntry = $"[{timeTag}]{levelTag} {message}";
                EngineLogView.Items.Add(logEntry);

                // 자동 스크롤: 가장 최근 로그로 이동
                if (EngineLogView.Items.Count > 0)
                {
                    EngineLogView.ScrollIntoView(EngineLogView.Items[EngineLogView.Items.Count - 1]);
                }
            }));
        }
    }
}
