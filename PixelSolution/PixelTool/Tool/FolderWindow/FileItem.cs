using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace PixelTool
{
    // 데이터 모델 클래스 (파일/폴더 정보)
    public class FileItem
    {
        public string Name { get; set; }
        public string FullPath { get; set; }
        public bool IsDirectory { get; set; }
        // 자식 노드 목록
        public ObservableCollection<FileItem> Children { get; set; } = new ObservableCollection<FileItem>();
    }

    public class FileDisplayItem
    {
        public string Name { get; set; }
        public string Icon { get; set; }
        public string IconColor { get; set; }
        public string FullPath { get; set; }      // 실제 이미지 경로
        public bool IsImage { get; set; }         // 이미지 파일인지 여부
    }
}
