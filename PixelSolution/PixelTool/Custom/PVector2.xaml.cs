using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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
    /// PVector2.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class PVector2 : UserControl
    {
        // 1. LabelName: 외부에서 "Position", "Offset" 등으로 이름 지정
        public static readonly DependencyProperty LabelNameProperty =
            DependencyProperty.Register("LabelName", typeof(string), typeof(PVector2), new PropertyMetadata("Vector2"));

        public string LabelName
        {
            get { return (string)GetValue(LabelNameProperty); }
            set { SetValue(LabelNameProperty, value); }
        }

        // 2. XValue: 외부에서 접근할 X값 (String)
        public static readonly DependencyProperty XValueProperty =
            DependencyProperty.Register("XValue", typeof(string), typeof(PVector2), new PropertyMetadata("0", OnValueChanged));

        public string XValue
        {
            get { return (string)GetValue(XValueProperty); }
            set { SetValue(XValueProperty, value); }
        }

        // 3. YValue: 외부에서 접근할 Y값 (String)
        public static readonly DependencyProperty YValueProperty =
            DependencyProperty.Register("YValue", typeof(string), typeof(PVector2), new PropertyMetadata("0", OnValueChanged));

        public string YValue
        {
            get { return (string)GetValue(YValueProperty); }
            set { SetValue(YValueProperty, value); }
        }

        // 4. 값이 변경될 때 발생하는 커스텀 이벤트 (Hot-Reload 용도)
        public event RoutedEventHandler ValueChanged;

        public PVector2()
        {
            InitializeComponent();
        }

        // 값이 변경될 때마다 이벤트를 발생시켜 부모 창에 알림
        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is PVector2 control)
            {
                control.ValueChanged?.Invoke(control, new RoutedEventArgs());
            }
        }

        // [안전성 검증] 알파벳이나 특수문자 입력을 막고 숫자, 마이너스(-), 소수점(.)만 허용
        private void NumberValidationTextBox(object sender, TextCompositionEventArgs e)
        {
            Regex regex = new Regex("[^0-9.-]+");
            e.Handled = regex.IsMatch(e.Text);
        }
    }
}
