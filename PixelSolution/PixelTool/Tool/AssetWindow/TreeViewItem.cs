using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace PixelTool
{
    public class FolderViewItem
    {
       public bool IsSelected { get; set; }
       public bool IsExpanded { get; set; }
       public string fullPath { get; set; }
    }

    public class FolderItem : INotifyPropertyChanged
    {
        public FolderItem(string path)
        {
            fullPath = path;
            folderName = Path.GetFileNameWithoutExtension(path);
        }

        private bool _isExpanded;
        public bool IsExpanded
        {
            get => _isExpanded;
            set { _isExpanded = value; OnPropertyChanged(nameof(IsExpanded)); }
        }

        private bool _isSelected;
        public bool IsSelected
        {
            get => _isSelected;
            set { _isSelected = value; OnPropertyChanged(nameof(IsSelected)); }
        }

        public string fullPath { get; set; }
        public string folderName { get; set; }
        public ObservableCollection<FolderItem> FolderItems { get; set; } = new ObservableCollection<FolderItem>();

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }


    public class FileItem
    {
        public string FileName { get; set; }     // 예: Player.lua
        public string FileIcon { get; set; }     // 예: Player.lua
        public string Extension { get; set; }    // 예: .lua
        public string FileSize { get; set; }     // 예: 15 KB
        public string FullPath { get; set; }     // 전체 경로

        public FileItem(string path)
        {
            var info = new System.IO.FileInfo(path);
            FullPath = path;
            FullPath = FullPath.Replace('\\', '/');
            FileName = System.IO.Path.GetFileNameWithoutExtension(path); ;
            Extension = info.Extension;
            // 바이트 단위를 KB로 변환해서 저장
            FileSize = $"{(info.Length / 1024.0):F1} KB";

            switch (Extension)
            {
                case ".lua":
                    FileIcon = (FileName == "main") ? "🏠" : "📜";
                    break;
                case ".scene":
                    FileIcon = "🎬";
                    break;
                case ".png":
                case ".jpg":
                    FileIcon = "🖼️";
                    break;
                case ".pxm":
                    FileIcon = "🧩";
                    break;
            }
        }
    }
}
