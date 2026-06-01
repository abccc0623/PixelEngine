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

        private string _folderName;
        public string folderName
        {
            get => _folderName;
            set { _folderName = value; OnPropertyChanged(nameof(folderName)); }
        }

        private string _fullPath;
        public string fullPath
        {
            get => _fullPath;
            set { _fullPath = value; OnPropertyChanged(nameof(fullPath)); }
        }

        public ObservableCollection<FolderItem> FolderItems { get; set; } = new ObservableCollection<FolderItem>();

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

    public class FileItem : INotifyPropertyChanged
    {
        private string _fileName;
        public string FileName
        {
            get => _fileName;
            set { _fileName = value; OnPropertyChanged(nameof(FileName)); }
        }

        private string _fullPath;
        public string FullPath
        {
            get => _fullPath;
            set { _fullPath = value; OnPropertyChanged(nameof(FullPath)); }
        }

        public string FileIcon { get; set; }
        public string Extension { get; set; }
        public string FileSize { get; set; }

        public FileItem(string path)
        {
            var info = new FileInfo(path);
            FullPath = path.Replace('\\', '/');
            FileName = Path.GetFileNameWithoutExtension(path);
            Extension = info.Extension;
            FileSize = $"{(info.Length / 1024.0):F1} KB";

            switch (Extension.ToLower())
            {
                case ".lua":
                    FileIcon = (FileName == "main") ? "🏠" : "📜";
                    break;
                case ".scene":
                    FileIcon = "🎬";
                    break;
                case ".png":
                case ".jpg":
                case ".jpeg":
                case ".bmp":
                case ".tga":
                    FileIcon = "🖼️";
                    break;
                case ".pxm":
                    FileIcon = "🧩";
                    break;
                default:
                    FileIcon = "📄";
                    break;
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
