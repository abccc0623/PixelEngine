using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PixelTool
{
    /// <summary>
    /// AssetWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class AssetWindow : UserControl
    {
        public ObservableCollection<FolderItem> FolderItems { get; set; } = new ObservableCollection<FolderItem>();
        public ObservableCollection<FileItem> FileItems { get; set; }= new ObservableCollection<FileItem>();

        FolderItem nowFolder;
        FileItem nowFile;

        public FolderItem rootFolder;
        public AssetWindow()
        {
            InitializeComponent();
            this.DataContext = this;
            Refresh();
        }

        public void Refresh()
        {
            FolderItems.Clear();
            rootFolder = new FolderItem("./Asset");
            rootFolder.IsExpanded = true;
            ScanDirectories(rootFolder);
            FolderItems.Add(rootFolder);

            SelectFolder(rootFolder.fullPath);
        }

        private void ScanDirectories(FolderItem parentItem)
        {
            try
            {
                var dirInfo = new DirectoryInfo(parentItem.fullPath);
                if (!dirInfo.Exists) return;

                // 현재 폴더 안의 모든 서브 폴더를 가져옴
                foreach (var dir in dirInfo.GetDirectories())
                {
                    var newItem = new FolderItem(dir.FullName);
                    parentItem.FolderItems.Add(newItem);

                    ScanDirectories(newItem);
                }
            }
            catch (UnauthorizedAccessException) { /* 접근 권한 없는 폴더 패스 */ }
            catch (Exception ex) { System.Diagnostics.Debug.WriteLine(ex.Message); }
        }

        private void FolderView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (FileView == null || FileItems == null) return;
            
            var folder = e.NewValue as FolderItem;
            if (folder == null) return;
            nowFolder = folder;
            // UI 컨트롤(FileView.Items)이 아니라, 데이터 리스트(FileItems)를 건드립니다.
            SelectFolder(nowFolder.fullPath);
        }

        void SelectFolder(string path)
        {
            FileItems.Clear();
            try
            {
                // Directory.GetFiles는 절대 경로를 반환하므로 FileItem 생성에 딱 좋습니다.
                string[] files = System.IO.Directory.GetFiles(path);

                foreach (string filePath in files)
                {
                    // 리스트에 추가하면 ListView가 알아서 감지해서 화면을 그립니다.
                    FileItems.Add(new FileItem(filePath));
                }
            }
            catch (UnauthorizedAccessException) { /* 접근 권한 없음 스킵 */ }
            catch (Exception ex) { System.Diagnostics.Debug.WriteLine(ex.Message); }
        }


        private void FileView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            var selectedFile = FileView.SelectedItem as FileItem;
            if (selectedFile == null) return;
            nowFile = selectedFile;
            string ext = selectedFile.Extension.ToLower();
            if (ext == ".lua" || ext == ".scene" || ext == ".pxm")
            {
                var findWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                if(findWindow != null)
                {
                    findWindow.OpenFile(selectedFile.FullPath);
                }
                TexturePreview.Source = null;
            }
            else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            {
                try
                {
                    BitmapImage bitmap = new BitmapImage();
                    bitmap.BeginInit();
                    bitmap.CacheOption = BitmapCacheOption.OnLoad;
                    string absolutePath = System.IO.Path.GetFullPath(selectedFile.FullPath);
                    bitmap.UriSource = new Uri(absolutePath);
                    bitmap.EndInit();
                    TexturePreview.Source = bitmap;
                }
                catch (Exception ex)
                {
                    // 이미지 파일이 깨졌거나 읽을 수 없는 경우
                    System.Diagnostics.Debug.WriteLine("이미지 로드 실패: " + ex.Message);
                    TexturePreview.Source = null;
                }
            }
            else
            {
                TexturePreview.Source = null;
            }
        }

        private void FileItem_Delete(object sender, RoutedEventArgs e)
        {
            var menuItem = sender as MenuItem;
            var targetFile = menuItem?.DataContext as FileItem; // FileItem에 FullPath 속성이 있다고 가정

            if (targetFile == null) return;

            var result = MessageBox.Show($"'{targetFile.FileName}' 파일을 정말 삭제하시겠습니까?","삭제 확인", MessageBoxButton.YesNo, MessageBoxImage.Warning);

            if (result == MessageBoxResult.Yes)
            {
                try
                {
                    // 2. 실제 파일 삭제
                    if (System.IO.File.Exists(targetFile.FullPath))
                    {
                        System.IO.File.Delete(targetFile.FullPath);
                    }
                    FileItems.Remove(targetFile);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"삭제 중 오류 발생: {ex.Message}");
                }
            }
        }

        private void FileItem_NameChange(object sender, RoutedEventArgs e)
        {
            var menuItem = sender as MenuItem;
            var targetFile = menuItem?.DataContext as FileItem;

            if (targetFile == null) return;

            string oldPath = targetFile.FullPath;
            string directory = System.IO.Path.GetDirectoryName(oldPath);

            string extension = System.IO.Path.GetExtension(oldPath); // 예: ".png"
            string fileNameOnly = System.IO.Path.GetFileNameWithoutExtension(oldPath); // 예: "Character"

            string newNameOnly = Microsoft.VisualBasic.Interaction.InputBox(
                "새 이름을 입력하세요 (확장자 제외):",
                "이름 바꾸기",
                fileNameOnly);

            if (!string.IsNullOrWhiteSpace(newNameOnly) && newNameOnly != fileNameOnly)
            {
                try
                {
                    string newFullName = newNameOnly + extension;
                    string newPath = System.IO.Path.Combine(directory, newFullName);
                    System.IO.File.Move(oldPath, newPath);
                    targetFile.FileName = newFullName;
                    targetFile.FullPath = newPath;
                    SelectFolder(nowFolder.fullPath);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"이름 변경 실패: {ex.Message}");
                }
            }
        }

        private void FileItem_CopyPath(object sender, RoutedEventArgs e)
        {
            var menuItem = sender as MenuItem;
            var targetFile = menuItem?.DataContext as FileItem;

            if (targetFile == null || string.IsNullOrEmpty(targetFile.FullPath)) return;

            try
            {
                System.Windows.Clipboard.SetText(targetFile.FullPath);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"경로 복사 실패: {ex.Message}");
            }
        }

        private void FolderItem_Create(object sender, RoutedEventArgs e)
        {

        }

        private void FolderItem_Delete(object sender, RoutedEventArgs e)
        {

        }

        private void TreeViewItem_PreviewMouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            TreeViewItem item = sender as TreeViewItem;
            if (item != null)
            {
                item.IsSelected = true;
                item.Focus();
                e.Handled = true; // 이벤트가 부모로 튀지 않게 막음
            }
        }
    }
}
