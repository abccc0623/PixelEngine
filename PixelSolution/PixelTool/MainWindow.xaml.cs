using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace PixelTool
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            IntPtr hWnd = new WindowInteropHelper(this).Handle;
            int darkMode = 1; // 1이면 다크, 0이면 라이트
            DwmSetWindowAttribute(hWnd, 35, ref darkMode, sizeof(int)); // 35: DWMWA_USE_IMMERSIVE_DARK_MODE
        }

        [DllImport("dwmapi.dll")]
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);
    }
}