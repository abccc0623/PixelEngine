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
    /// ToolbarButton.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ToolbarButton : UserControl
    {
        public static readonly DependencyProperty ButtonIconProperty =
            DependencyProperty.Register("ButtonIcon", typeof(string), typeof(ToolbarButton), new PropertyMetadata("⚙️"));

        public string ButtonIcon
        {
            get { return (string)GetValue(ButtonIconProperty); }
            set { SetValue(ButtonIconProperty, value); }
        }

        // 2. 팝업 창 안의 내용물을 밖에서 마음대로 넣을 수 있게 해주는 속성
        public static readonly DependencyProperty PopupContentProperty =
            DependencyProperty.Register("PopupContent", typeof(object), typeof(ToolbarButton), new PropertyMetadata(null));

        public object PopupContent
        {
            get { return GetValue(PopupContentProperty); }
            set { SetValue(PopupContentProperty, value); }
        }

        public ToolbarButton()
        {
            InitializeComponent();
        }

        // 1. 메인 버튼에 마우스가 들어오면 팝업을 엽니다.
        private void MainHoverButton_MouseEnter(object sender, MouseEventArgs e)
        {
            SubMenuPopup.IsOpen = true;
        }

        // 2. 메인 버튼에서 마우스가 나갈 때
        private async void MainHoverButton_MouseLeave(object sender, MouseEventArgs e)
        {
            // 마우스가 팝업 창으로 넘어갈 수 있도록 0.1초(100ms) 대기합니다.
            await Task.Delay(100);

            // 대기 후에도 마우스가 팝업 창 안에 없다면 팝업을 닫습니다.
            if (!SubMenuPopup.IsMouseOver)
            {
                SubMenuPopup.IsOpen = false;
            }
        }

        // 3. 팝업 창 안으로 마우스가 들어오면 계속 열어둡니다.
        private void SubMenuPopup_MouseEnter(object sender, MouseEventArgs e)
        {
            SubMenuPopup.IsOpen = true;
        }

        // 4. 팝업 창 밖으로 마우스가 완전히 나가면 닫습니다.
        private void SubMenuPopup_MouseLeave(object sender, MouseEventArgs e)
        {
            SubMenuPopup.IsOpen = false;
        }

        // 5. 팝업 창 안의 서브 버튼을 클릭했을 때의 기능
        private void ActionSubButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("서브 버튼이 클릭되었습니다! 엔진 명령을 실행합니다.", "Pixel Engine");

            // 클릭 후 팝업을 깔끔하게 닫고 싶다면 아래 코드를 유지하세요.
            SubMenuPopup.IsOpen = false;
        }
    }
}
