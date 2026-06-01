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
    public partial class AssetWindow : UserControl
    {
        public ObservableCollection<FolderItem> FolderItems { get; set; } = new ObservableCollection<FolderItem>();
        public ObservableCollection<FileItem> FileItems { get; set; } = new ObservableCollection<FileItem>();

        FolderItem nowFolder;
        FileItem nowFile;
        public FolderItem rootFolder;

        // 드래그 관련
        private Point _dragStartPoint;
        private FileItem _draggedFile;
        private bool _isDragging = false;

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

                foreach (var dir in dirInfo.GetDirectories())
                {
                    var newItem = new FolderItem(dir.FullName);
                    parentItem.FolderItems.Add(newItem);
                    ScanDirectories(newItem);
                }
            }
            catch (UnauthorizedAccessException) { }
            catch (Exception ex) { System.Diagnostics.Debug.WriteLine(ex.Message); }
        }

        private void FolderView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (FileView == null || FileItems == null) return;
            var folder = e.NewValue as FolderItem;
            if (folder == null) return;
            nowFolder = folder;
            SelectFolder(nowFolder.fullPath);
        }

        void SelectFolder(string path)
        {
            FileItems.Clear();
            try
            {
                string[] files = Directory.GetFiles(path);
                foreach (string filePath in files)
                    FileItems.Add(new FileItem(filePath));
            }
            catch (UnauthorizedAccessException) { }
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
                if (findWindow != null)
                    findWindow.OpenFile(selectedFile.FullPath);
                TexturePreview.Source = null;
            }
            else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            {
                try
                {
                    BitmapImage bitmap = new BitmapImage();
                    bitmap.BeginInit();
                    bitmap.CacheOption = BitmapCacheOption.OnLoad;
                    bitmap.UriSource = new Uri(Path.GetFullPath(selectedFile.FullPath));
                    bitmap.EndInit();
                    TexturePreview.Source = bitmap;
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine("이미지 로드 실패: " + ex.Message);
                    TexturePreview.Source = null;
                }
            }
            else
            {
                TexturePreview.Source = null;
            }
        }

        // ========================
        // 파일 컨텍스트 메뉴
        // ========================

        private void FileItem_Delete(object sender, RoutedEventArgs e)
        {
            var targetFile = GetFileItemFromSender(sender);
            if (targetFile == null) return;

            var result = MessageBox.Show($"'{targetFile.FileName}' 파일을 삭제하시겠습니까?", "삭제 확인", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result == MessageBoxResult.Yes)
            {
                try
                {
                    if (File.Exists(targetFile.FullPath))
                        File.Delete(targetFile.FullPath);
                    FileItems.Remove(targetFile);
                }
                catch (Exception ex) { MessageBox.Show($"삭제 중 오류 발생: {ex.Message}"); }
            }
        }

        private void FileItem_NameChange(object sender, RoutedEventArgs e)
        {
            var targetFile = GetFileItemFromSender(sender);
            if (targetFile == null) return;

            string oldPath = targetFile.FullPath;
            string directory = Path.GetDirectoryName(oldPath);
            string extension = Path.GetExtension(oldPath);
            string fileNameOnly = Path.GetFileNameWithoutExtension(oldPath);

            string newNameOnly = Microsoft.VisualBasic.Interaction.InputBox("새 이름을 입력하세요 (확장자 제외):", "이름 바꾸기", fileNameOnly);
            if (!string.IsNullOrWhiteSpace(newNameOnly) && newNameOnly != fileNameOnly)
            {
                try
                {
                    string newPath = Path.Combine(directory, newNameOnly + extension);
                    File.Move(oldPath, newPath);
                    targetFile.FileName = newNameOnly;
                    targetFile.FullPath = newPath.Replace('\\', '/');
                    SelectFolder(nowFolder.fullPath);
                }
                catch (Exception ex) { MessageBox.Show($"이름 변경 실패: {ex.Message}"); }
            }
        }

        private void FileItem_CopyPath(object sender, RoutedEventArgs e)
        {
            var targetFile = GetFileItemFromSender(sender);
            if (targetFile == null) return;
            try { Clipboard.SetText(targetFile.FullPath); }
            catch (Exception ex) { MessageBox.Show($"경로 복사 실패: {ex.Message}"); }
        }

        // ========================
        // 폴더 컨텍스트 메뉴
        // ========================

        private void FolderItem_Create(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null) return;

            string newName = Microsoft.VisualBasic.Interaction.InputBox("새 폴더 이름을 입력하세요:", "새 폴더", "새 폴더");
            if (string.IsNullOrWhiteSpace(newName)) return;

            try
            {
                string newPath = Path.Combine(nowFolder.fullPath, newName);
                Directory.CreateDirectory(newPath);
                var newItem = new FolderItem(newPath);
                nowFolder.FolderItems.Add(newItem);
                nowFolder.IsExpanded = true;
            }
            catch (Exception ex) { MessageBox.Show($"폴더 생성 실패: {ex.Message}"); }
        }

        private void FolderItem_Rename(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null || nowFolder == rootFolder) return;

            string oldPath = nowFolder.fullPath;
            string parentPath = Path.GetDirectoryName(oldPath);
            string oldName = Path.GetFileName(oldPath);

            string newName = Microsoft.VisualBasic.Interaction.InputBox("새 이름을 입력하세요:", "이름 바꾸기", oldName);
            if (string.IsNullOrWhiteSpace(newName) || newName == oldName) return;

            try
            {
                string newPath = Path.Combine(parentPath, newName);
                Directory.Move(oldPath, newPath);
                nowFolder.fullPath = newPath;
                nowFolder.folderName = newName;
                SelectFolder(newPath);
            }
            catch (Exception ex) { MessageBox.Show($"이름 변경 실패: {ex.Message}"); }
        }

        private void FolderItem_Delete(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null || nowFolder == rootFolder)
            {
                MessageBox.Show("루트 폴더는 삭제할 수 없습니다.");
                return;
            }

            var result = MessageBox.Show($"'{nowFolder.folderName}' 폴더와 내용을 모두 삭제하시겠습니까?", "삭제 확인", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result == MessageBoxResult.Yes)
            {
                try
                {
                    Directory.Delete(nowFolder.fullPath, true);
                    Refresh();
                }
                catch (Exception ex) { MessageBox.Show($"폴더 삭제 실패: {ex.Message}"); }
            }
        }

        // ========================
        // 새 파일 생성
        // ========================

        private void CreateNewLua(object sender, RoutedEventArgs e) => CreateNewFile(".lua", "NewScript");
        private void CreateNewPxm(object sender, RoutedEventArgs e) => CreateNewFile(".pxm", "NewObject");
        private void CreateNewScene(object sender, RoutedEventArgs e) => CreateNewFile(".scene", "NewScene");

        private void CreateNewFile(string extension, string defaultName)
        {
            if (nowFolder == null) return;

            string newName = Microsoft.VisualBasic.Interaction.InputBox($"파일 이름을 입력하세요 (확장자 제외):", "새 파일", defaultName);
            if (string.IsNullOrWhiteSpace(newName)) return;

            try
            {
                string newPath = Path.Combine(nowFolder.fullPath, newName + extension);
                if (File.Exists(newPath))
                {
                    MessageBox.Show("같은 이름의 파일이 이미 존재합니다.");
                    return;
                }
                File.WriteAllText(newPath, "");
                FileItems.Add(new FileItem(newPath));
            }
            catch (Exception ex) { MessageBox.Show($"파일 생성 실패: {ex.Message}"); }
        }

        private void RefreshFolder(object sender, RoutedEventArgs e)
        {
            if (nowFolder != null)
                SelectFolder(nowFolder.fullPath);
        }

        // ========================
        // 드래그 앤 드롭
        // ========================

        private void FileView_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _dragStartPoint = e.GetPosition(null);
            _draggedFile = GetFileItemAtPoint(e.GetPosition(FileView));
        }

        private void FileView_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed || _draggedFile == null || _isDragging) return;

            Point pos = e.GetPosition(null);
            if (Math.Abs(pos.X - _dragStartPoint.X) < SystemParameters.MinimumHorizontalDragDistance &&
                Math.Abs(pos.Y - _dragStartPoint.Y) < SystemParameters.MinimumVerticalDragDistance) return;

            _isDragging = true;
            DragDrop.DoDragDrop(FileView, new DataObject("FileItem", _draggedFile), DragDropEffects.Move);
            _isDragging = false;
            _draggedFile = null;
        }

        private void FileView_Drop(object sender, DragEventArgs e)
        {
            // 외부 파일 드롭 (탐색기에서)
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if (nowFolder == null) return;
                foreach (string file in files)
                {
                    try
                    {
                        string dest = Path.Combine(nowFolder.fullPath, Path.GetFileName(file));
                        if (file != dest)
                        {
                            File.Copy(file, dest, false);
                            FileItems.Add(new FileItem(dest));
                        }
                    }
                    catch (Exception ex) { MessageBox.Show($"파일 복사 실패: {ex.Message}"); }
                }
            }
        }

        private void FolderView_Drop(object sender, DragEventArgs e)
        {
            // 파일을 폴더로 드래그
            if (!e.Data.GetDataPresent("FileItem")) return;
            var file = e.Data.GetData("FileItem") as FileItem;
            if (file == null) return;

            var target = GetFolderItemAtPoint(e.GetPosition(FolderView));
            if (target == null || target.fullPath == nowFolder?.fullPath) return;

            try
            {
                string dest = Path.Combine(target.fullPath, Path.GetFileName(file.FullPath));
                File.Move(file.FullPath, dest);
                FileItems.Remove(file);
                if (nowFolder?.fullPath == target.fullPath)
                    FileItems.Add(new FileItem(dest));
            }
            catch (Exception ex) { MessageBox.Show($"파일 이동 실패: {ex.Message}"); }
        }

        // ========================
        // 헬퍼
        // ========================

        private void TreeViewItem_PreviewMouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            TreeViewItem item = sender as TreeViewItem;
            if (item != null)
            {
                item.IsSelected = true;
                item.Focus();
                e.Handled = true;
            }
        }

        private FileItem GetFileItemFromSender(object sender)
        {
            var menuItem = sender as MenuItem;
            return menuItem?.DataContext as FileItem;
        }

        private FileItem GetFileItemAtPoint(Point point)
        {
            var element = FileView.InputHitTest(point) as DependencyObject;
            while (element != null)
            {
                if (element is ListViewItem item)
                    return item.DataContext as FileItem;
                element = VisualTreeHelper.GetParent(element);
            }
            return null;
        }

        private FolderItem GetFolderItemAtPoint(Point point)
        {
            var element = FolderView.InputHitTest(point) as DependencyObject;
            while (element != null)
            {
                if (element is TreeViewItem item)
                    return item.DataContext as FolderItem;
                element = VisualTreeHelper.GetParent(element);
            }
            return null;
        }
    }
}
