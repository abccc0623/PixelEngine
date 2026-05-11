using System;
using System.Collections.Concurrent;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;

namespace PixelTool
{
    // 로그 데이터를 담는 가벼운 POCO 객체
    public class LogEntry
    {
        public string Timestamp { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
        public int Level { get; set; }
        public Brush DisplayColor => Level switch
        {
            0 => Brushes.White,   // INFO
            1 => Brushes.Yellow,  // WARN
            2 => Brushes.Red,     // ERR
            _ => Brushes.Gray
        };
        public string Tag => Level == 0 ? "[INFO]" : (Level == 1 ? "[WARN]" : "[ERR ]");
        public override string ToString() => $"[{Timestamp}]{Tag} {Message}";
    }

    public partial class ConsoleWindow : UserControl
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void LogDelegate(string message, int level);

        private static LogDelegate? _logCallback;

        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void RegisterLogCallback(LogDelegate callback);

        // 고성능 처리를 위한 스레드 안전 큐 및 바인딩 컬렉션
        private static readonly ConcurrentQueue<LogEntry> _pendingLogs = new ConcurrentQueue<LogEntry>();
        private readonly ObservableCollection<LogEntry> _logItems = new ObservableCollection<LogEntry>();
        private readonly DispatcherTimer _uiUpdateTimer;

        private const int MAX_LOG_COUNT = 2000; // 최대 로그 보관 개수 (메모리 방어)

        public ConsoleWindow()
        {
            InitializeComponent();

            // 데이터 바인딩 설정 (XAML에서 ItemsSource={Binding _logItems} 형태로 연결되어 있어야 함)
            EngineLogView.ItemsSource = _logItems;

            // 콜백 등록
            _logCallback = OnNativeLogReceived;
            RegisterLogCallback(_logCallback);

            // UI 업데이트를 위한 전용 타이머 (Batch Processing)
            _uiUpdateTimer = new DispatcherTimer(DispatcherPriority.Background)
            {
                Interval = TimeSpan.FromMilliseconds(100) // 0.1초마다 UI 갱신
            };
            _uiUpdateTimer.Tick += ProcessLogQueue;
            _uiUpdateTimer.Start();
        }

        // 1. C# 툴 내부의 다른 시스템에서 호출하기 위한 정적 인터페이스
        public static void LogMessage(string message, int level)
        {
            // 어느 스레드에서 호출하든 안전하게 큐에 적재
            _pendingLogs.Enqueue(new LogEntry
            {
                Timestamp = DateTime.Now.ToString("HH:mm:ss"),
                Message = message,
                Level = level
            });
        }

        // 2. C++ 엔진 네이티브 스레드에서 콜백으로 들어오는 지점
        private void OnNativeLogReceived(string message, int level)
        {
            // 내부적으로 C# 호출용 LogMessage를 재사용하여 로직 일원화
            LogMessage(message, level);
        }

        // 3. UI 스레드에서 주기적으로 실행되는 로그 배치 처리 로직
        private void ProcessLogQueue(object? sender, EventArgs e)
        {
            if (_pendingLogs.IsEmpty) return;

            // 큐 적체 현상(Queue Buildup)을 막기 위해 1회 최대 500개까지 신속하게 처리
            int processCount = 0;
            while (_pendingLogs.TryDequeue(out var log) && processCount < 500)
            {
                if (_logItems.Count >= MAX_LOG_COUNT)
                {
                    _logItems.RemoveAt(0); // 가장 오래된 로그 삭제
                }
                _logItems.Add(log);
                processCount++;
            }

            // 자동 스크롤: 갱신된 내역이 있을 때만 맨 아래로 포커스 이동
            if (processCount > 0 && EngineLogView.Items.Count > 0)
            {
                EngineLogView.ScrollIntoView(_logItems[_logItems.Count - 1]);
            }
        }

        private void Clear(object sender, RoutedEventArgs e)
        {
            _logItems.Clear();
            while (_pendingLogs.TryDequeue(out _)) { } // 잔여 큐 완전히 비우기
        }

        void Copy()
        {
            if (EngineLogView.SelectedItem is LogEntry log)
            {
                Clipboard.SetText(log.ToString());
                // 필요시 EngineLogView.SelectedIndex = -1; 추가하여 선택 해제 가능
            }
        }

        private void CopyMenuItem_Click(object sender, RoutedEventArgs e) => Copy();
        private void EngineLogView_SelectionChanged(object sender, SelectionChangedEventArgs e) => Copy();
    }
}