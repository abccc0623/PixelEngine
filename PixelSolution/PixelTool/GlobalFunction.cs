using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace PixelTool
{
    public static class GlobalFunction
    {
        public static T? GetDockedWindow<T>() where T : class
        {
            // 1. 메인 윈도우 가져오기
            var mainWin = Application.Current.MainWindow as MainWindow;
            if (mainWin == null) return null;

            // Registered instances remain discoverable when hidden or floating.
            foreach (var content in mainWin.DockContents)
            {
                if (content is T target) return target;
                var child = FindChild<T>(content);
                if (child != null) return child;
            }
            return null;
        }

        private static T? FindChild<T>(DependencyObject? parent) where T : class
        {
            if (parent == null) return null;

            if (parent is T target)
                return target;

            int childCount = VisualTreeHelper.GetChildrenCount(parent);
            for (int i = 0; i < childCount; i++)
            {
                var result = FindChild<T>(VisualTreeHelper.GetChild(parent, i));
                if (result != null)
                    return result;
            }

            return null;
        }

        public static void CreateMainLuaFile(string path)
        {
            string mainpath = path + "/main.lua";
            if (!File.Exists(mainpath))
            {
                string content = LuaFileManager.GetFileContent("main.lua");
                var utf8WithBom = new System.Text.UTF8Encoding(true);
                File.WriteAllText(mainpath, content, utf8WithBom);

                var luaWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                if (luaWindow != null)
                {
                    luaWindow.OpenFile(mainpath);
                }
                var findWindow = GlobalFunction.GetDockedWindow<AssetWindow>();
                if (findWindow != null)
                {
                    findWindow.Refresh();
                }
            }
        }
    }
}
