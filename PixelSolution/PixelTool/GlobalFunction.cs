using AvalonDock;
using AvalonDock.Layout;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace PixelTool
{
    public static class GlobalFunction
    {
        public static T GetDockedWindow<T>() where T : class
        {
            // 1. 메인 윈도우 가져오기
            var mainWin = Application.Current.MainWindow as MainWindow;
            if (mainWin == null) return null;

            // 2. 메인 윈도우에 선언된 dockManager 가져오기 (이름 확인 필수!)
            var manager = mainWin.dockManager;
            if (manager == null) return null;

            // 3. 레이아웃의 모든 자식 요소를 뒤져서 T 타입이 들어있는 Anchorable 찾기
            var layoutAnchorable = manager.Layout.Descendents()
                                          .OfType<LayoutContent>()
                                          .FirstOrDefault(a => a.Content is T);
            // 4. 찾았다면 그 안의 실체(Content)를 T 타입으로 변환해서 반환
            return layoutAnchorable?.Content as T;
        }

     

    }
}
