using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Microsoft.VisualBasic.FileIO;

namespace PixelTool
{
    public partial class AssetWindow : UserControl
    {
        public ObservableCollection<FolderItem> FolderItems { get; set; } = new ObservableCollection<FolderItem>();
        public ObservableCollection<FileItem> FileItems { get; set; } = new ObservableCollection<FileItem>();

        private readonly List<FileItem> _allFiles = new List<FileItem>();
        private Point _dragStartPoint;
        private FileItem? _draggedFile;
        private bool _isDragging;
        private bool _preserveSelectionForDrag;

        private FolderItem nowFolder;
        private FileItem nowFile;
        public FolderItem rootFolder;

        public AssetWindow()
        {
            InitializeComponent();
            DataContext = this;
            Refresh();
        }

        public void Refresh()
        {
            var selectedPath = nowFolder?.fullPath;
            FolderItems.Clear();
            rootFolder = new FolderItem(ProjectPathService.AssetPath);
            rootFolder.IsExpanded = true;
            ScanDirectories(rootFolder);
            FolderItems.Add(rootFolder);

            var targetPath = !string.IsNullOrWhiteSpace(selectedPath) && Directory.Exists(selectedPath)
                ? selectedPath
                : rootFolder.fullPath;
            SelectFolderItem(targetPath);
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
            catch (Exception ex) { Debug.WriteLine(ex.Message); }
        }

        private void FolderView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (FileView == null || FileItems == null) return;
            if (e.NewValue is not FolderItem folder) return;

            nowFolder = folder;
            SelectFolder(nowFolder.fullPath);
        }

        private void SelectFolder(string path)
        {
            _allFiles.Clear();
            FileItems.Clear();
            ClearPreview();

            try
            {
                foreach (string filePath in Directory.GetFiles(path))
                    _allFiles.Add(new FileItem(filePath));
            }
            catch (UnauthorizedAccessException) { }
            catch (Exception ex) { Debug.WriteLine(ex.Message); }

            ApplySearchFilter();
        }

        private void SelectFolderItem(string path)
        {
            var folder = FindFolder(rootFolder, path);
            if (folder == null)
            {
                SelectFolder(path);
                return;
            }

            if (nowFolder != null)
                nowFolder.IsSelected = false;

            nowFolder = folder;
            nowFolder.IsSelected = true;
            nowFolder.IsExpanded = true;
            SelectFolder(nowFolder.fullPath);
        }

        private FolderItem FindFolder(FolderItem folder, string path)
        {
            if (Path.GetFullPath(folder.fullPath).Equals(Path.GetFullPath(path), StringComparison.OrdinalIgnoreCase))
                return folder;

            foreach (var child in folder.FolderItems)
            {
                var found = FindFolder(child, path);
                if (found != null)
                    return found;
            }

            return null;
        }

        private void ApplySearchFilter()
        {
            FileItems.Clear();
            var keyword = SearchBox?.Text?.Trim();
            IEnumerable<FileItem> source = _allFiles;

            if (!string.IsNullOrWhiteSpace(keyword))
            {
                source = source.Where(file =>
                    file.FileName.Contains(keyword, StringComparison.OrdinalIgnoreCase) ||
                    file.Extension.Contains(keyword, StringComparison.OrdinalIgnoreCase));
            }

            foreach (var file in source)
                FileItems.Add(file);
        }

        private void FileView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (FileView.SelectedItem is not FileItem selectedFile) return;
            nowFile = selectedFile;
            string ext = selectedFile.Extension.ToLower();

            if (ext == ".lua" || ext == ".scene" || ext == ".pxm")
            {
                var findWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                findWindow?.OpenFile(selectedFile.FullPath);
                TexturePreview.Source = null;
            }
            else if (IsImageFile(ext))
            {
                LoadImagePreview(selectedFile.FullPath);
            }
            else
            {
                TexturePreview.Source = null;
            }

            UpdatePreviewInfo(selectedFile);
        }

        private void FileView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (FileView.SelectedItem is FileItem selectedFile)
            {
                nowFile = selectedFile;
                UpdatePreviewInfo(selectedFile);
                if (IsImageFile(selectedFile.Extension.ToLower()))
                    LoadImagePreview(selectedFile.FullPath);
                else
                    TexturePreview.Source = null;
            }
            else
            {
                ClearPreview();
            }
        }

        private void LoadImagePreview(string path)
        {
            try
            {
                BitmapImage bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.UriSource = new Uri(Path.GetFullPath(path));
                bitmap.EndInit();
                TexturePreview.Source = bitmap;
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Image load failed: " + ex.Message);
                TexturePreview.Source = null;
            }
        }

        private static bool IsImageFile(string extension)
        {
            return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga";
        }

        private void UpdatePreviewInfo(FileItem file)
        {
            PreviewFileName.Text = file.FileName + file.Extension;
            PreviewFileInfo.Text = $"{file.Extension} / {file.FileSize}";
            PreviewFilePath.Text = file.FullPath;
        }

        private void ClearPreview()
        {
            TexturePreview.Source = null;
            if (PreviewFileName == null) return;
            PreviewFileName.Text = "";
            PreviewFileInfo.Text = "";
            PreviewFilePath.Text = "";
        }

        private void FileItem_Delete(object sender, RoutedEventArgs e)
        {
            DeleteSelectedFiles(GetFilesFromSender(sender));
        }

        private IReadOnlyList<FileItem> GetSelectedFiles()
        {
            return FileView.SelectedItems.Cast<FileItem>().ToList();
        }

        private IReadOnlyList<FileItem> GetFilesFromSender(object sender)
        {
            var target = GetFileItemFromSender(sender);
            var selected = GetSelectedFiles();
            return target != null && selected.Contains(target) ? selected : target != null ? new[] { target } : selected;
        }

        private void DeleteSelectedFiles(IReadOnlyList<FileItem> files)
        {
            if (files.Count == 0) return;

            string message = files.Count == 1
                ? $"Move '{files[0].FileName}{files[0].Extension}' to the Recycle Bin?"
                : $"Move {files.Count} selected files to the Recycle Bin?";
            var result = PixelMessageBox.Show(message, "Delete", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result != MessageBoxResult.Yes) return;

            try
            {
                foreach (var file in files)
                {
                    if (File.Exists(file.FullPath))
                        FileSystem.DeleteFile(file.FullPath, UIOption.OnlyErrorDialogs, RecycleOption.SendToRecycleBin);
                }
                SelectFolder(nowFolder.fullPath);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Delete failed: {ex.Message}"); }
        }

        private void DeleteFile(FileItem targetFile)
        {
            if (targetFile == null) return;

            var result = PixelMessageBox.Show($"Delete '{targetFile.FileName}{targetFile.Extension}'?", "Delete", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result != MessageBoxResult.Yes) return;

            try
            {
                if (File.Exists(targetFile.FullPath))
                    File.Delete(targetFile.FullPath);
                _allFiles.RemoveAll(file => file.FullPath == targetFile.FullPath);
                FileItems.Remove(targetFile);
                ClearPreview();
            }
            catch (Exception ex) { PixelMessageBox.Show($"Delete failed: {ex.Message}"); }
        }

        private void FileItem_NameChange(object sender, RoutedEventArgs e)
        {
            RenameFile(GetFileItemFromSender(sender));
        }

        private void RenameFile(FileItem targetFile)
        {
            if (targetFile == null) return;

            string oldPath = targetFile.FullPath;
            string directory = Path.GetDirectoryName(oldPath);
            string extension = Path.GetExtension(oldPath);
            string fileNameOnly = Path.GetFileNameWithoutExtension(oldPath);

            string newNameOnly = PixelPromptDialog.Show("Input file name without extension:", "Rename", fileNameOnly);
            if (string.IsNullOrWhiteSpace(newNameOnly) || newNameOnly == fileNameOnly) return;

            try
            {
                string newPath = Path.Combine(directory, newNameOnly + extension);
                if (File.Exists(newPath))
                {
                    PixelMessageBox.Show("A file with the same name already exists.");
                    return;
                }

                File.Move(oldPath, newPath);
                SelectFolder(nowFolder.fullPath);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Rename failed: {ex.Message}"); }
        }

        private void FileItem_CopyPath(object sender, RoutedEventArgs e)
        {
            var targetFile = GetFileItemFromSender(sender);
            if (targetFile == null) return;
            CopyText(targetFile.FullPath);
        }

        private void FileItem_OpenExplorer(object sender, RoutedEventArgs e)
        {
            var targetFile = GetFileItemFromSender(sender);
            if (targetFile == null) return;
            OpenExplorer($"/select,\"{Path.GetFullPath(targetFile.FullPath)}\"");
        }

        private void FileItem_Duplicate(object sender, RoutedEventArgs e)
        {
            foreach (var file in GetFilesFromSender(sender))
                DuplicateFile(file, false);
            SelectFolder(nowFolder.fullPath);
        }

        private void DuplicateFile(FileItem targetFile, bool refresh = true)
        {
            if (targetFile == null) return;

            try
            {
                string directory = Path.GetDirectoryName(targetFile.FullPath);
                string name = Path.GetFileNameWithoutExtension(targetFile.FullPath);
                string extension = Path.GetExtension(targetFile.FullPath);
                string copyPath = GetUniquePath(directory, $"{name}_Copy", extension);
                File.Copy(targetFile.FullPath, copyPath);
                if (refresh) SelectFolder(nowFolder.fullPath);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Duplicate failed: {ex.Message}"); }
        }

        private void FolderItem_Create(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null) return;

            string newName = PixelPromptDialog.Show("Input folder name:", "New Folder", "NewFolder");
            if (string.IsNullOrWhiteSpace(newName)) return;

            try
            {
                string newPath = Path.Combine(nowFolder.fullPath, newName);
                if (Directory.Exists(newPath))
                {
                    PixelMessageBox.Show("A folder with the same name already exists.");
                    return;
                }

                Directory.CreateDirectory(newPath);
                var newItem = new FolderItem(newPath);
                nowFolder.FolderItems.Add(newItem);
                nowFolder.IsExpanded = true;
            }
            catch (Exception ex) { PixelMessageBox.Show($"Create folder failed: {ex.Message}"); }
        }

        private void FolderItem_Rename(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null || nowFolder == rootFolder) return;

            string oldPath = nowFolder.fullPath;
            string parentPath = Path.GetDirectoryName(oldPath);
            string oldName = Path.GetFileName(oldPath);

            string newName = PixelPromptDialog.Show("Input folder name:", "Rename", oldName);
            if (string.IsNullOrWhiteSpace(newName) || newName == oldName) return;

            try
            {
                string newPath = Path.Combine(parentPath, newName);
                if (Directory.Exists(newPath))
                {
                    PixelMessageBox.Show("A folder with the same name already exists.");
                    return;
                }

                Directory.Move(oldPath, newPath);
                nowFolder.fullPath = newPath;
                nowFolder.folderName = newName;
                Refresh();
            }
            catch (Exception ex) { PixelMessageBox.Show($"Rename folder failed: {ex.Message}"); }
        }

        private void FolderItem_Delete(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null || nowFolder == rootFolder)
            {
                PixelMessageBox.Show("The root folder cannot be deleted.");
                return;
            }

            var result = PixelMessageBox.Show($"Delete '{nowFolder.folderName}' and all contents?", "Delete", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result != MessageBoxResult.Yes) return;

            try
            {
                Directory.Delete(nowFolder.fullPath, true);
                nowFolder = rootFolder;
                Refresh();
            }
            catch (Exception ex) { PixelMessageBox.Show($"Delete folder failed: {ex.Message}"); }
        }

        private void FolderItem_CopyPath(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null) return;
            CopyText(nowFolder.fullPath);
        }

        private void FolderItem_Paste(object sender, RoutedEventArgs e) => PasteClipboard(nowFolder?.fullPath);
        private void FileView_Paste(object sender, RoutedEventArgs e) => PasteClipboard(nowFolder?.fullPath);

        private void FileItem_Copy(object sender, RoutedEventArgs e) => CopyFilesToClipboard(GetFilesFromSender(sender), false);
        private void FileItem_Cut(object sender, RoutedEventArgs e) => CopyFilesToClipboard(GetFilesFromSender(sender), true);

        private void FolderItem_OpenExplorer(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null) return;
            OpenExplorer($"\"{Path.GetFullPath(nowFolder.fullPath)}\"");
        }

        private void CreateNewLua(object sender, RoutedEventArgs e) => CreateNewFile(".lua", "NewScript");
        private void CreateNewPxm(object sender, RoutedEventArgs e) => CreateNewFile(".pxm", "NewObject");
        private void CreateNewScene(object sender, RoutedEventArgs e) => CreateNewFile(".scene", "NewScene");

        private void CreateNewFile(string extension, string defaultName)
        {
            if (nowFolder == null) return;

            string newName = PixelPromptDialog.Show("Input file name without extension:", "New File", defaultName);
            if (string.IsNullOrWhiteSpace(newName)) return;

            try
            {
                string newPath = Path.Combine(nowFolder.fullPath, newName + extension);
                if (File.Exists(newPath))
                {
                    PixelMessageBox.Show("A file with the same name already exists.");
                    return;
                }

                File.WriteAllText(newPath, "");
                SelectFolder(nowFolder.fullPath);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Create file failed: {ex.Message}"); }
        }

        private void RefreshFolder(object sender, RoutedEventArgs e)
        {
            if (nowFolder != null)
                SelectFolder(nowFolder.fullPath);
        }

        private void RefreshAll(object sender, RoutedEventArgs e)
        {
            Refresh();
        }

        private void GoParentFolder(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null || nowFolder == rootFolder) return;
            var parent = Directory.GetParent(nowFolder.fullPath);
            if (parent == null) return;
            SelectFolderItem(parent.FullName);
        }

        private void OpenProjectFolder(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(ProjectPathService.ProjectRootPath)) return;
            OpenExplorer($"\"{Path.GetFullPath(ProjectPathService.ProjectRootPath)}\"");
        }

        private void ImportFiles(object sender, RoutedEventArgs e)
        {
            if (nowFolder == null) return;

            var dialog = new OpenFileDialog
            {
                Multiselect = true,
                Title = "Import Files"
            };

            if (dialog.ShowDialog() != true) return;

            foreach (var source in dialog.FileNames)
                CopyExternalFile(source);

            SelectFolder(nowFolder.fullPath);
        }

        private void SearchBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            ApplySearchFilter();
        }

        private void FileView_KeyDown(object sender, KeyEventArgs e)
        {
            var selectedFile = FileView.SelectedItem as FileItem;
            var selectedFiles = GetSelectedFiles();

            if (e.Key == Key.Delete)
            {
                DeleteSelectedFiles(selectedFiles);
                e.Handled = true;
            }
            else if (e.Key == Key.F2)
            {
                RenameFile(selectedFile);
                e.Handled = true;
            }
            else if (e.Key == Key.F5)
            {
                Refresh();
                e.Handled = true;
            }
            else if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control && e.Key == Key.C)
            {
                CopyFilesToClipboard(selectedFiles, false);
                e.Handled = true;
            }
            else if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control && e.Key == Key.X)
            {
                CopyFilesToClipboard(selectedFiles, true);
                e.Handled = true;
            }
            else if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control && e.Key == Key.V)
            {
                PasteClipboard(nowFolder?.fullPath);
                e.Handled = true;
            }
            else if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control && e.Key == Key.A)
            {
                FileView.SelectAll();
                e.Handled = true;
            }
            else if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control && e.Key == Key.D)
            {
                foreach (var file in selectedFiles)
                    DuplicateFile(file, false);
                SelectFolder(nowFolder.fullPath);
                e.Handled = true;
            }
        }

        private void FileView_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _dragStartPoint = e.GetPosition(null);
            _draggedFile = GetFileItemAtPoint(e.GetPosition(FileView));
            _preserveSelectionForDrag = _draggedFile != null &&
                FileView.SelectedItems.Contains(_draggedFile) &&
                Keyboard.Modifiers == ModifierKeys.None &&
                FileView.SelectedItems.Count > 1;

            if (_preserveSelectionForDrag)
            {
                FileView.Focus();
                e.Handled = true;
            }
        }

        private void FileView_PreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_preserveSelectionForDrag && !_isDragging && _draggedFile != null)
                FileView.SelectedItem = _draggedFile;

            _preserveSelectionForDrag = false;
            _draggedFile = null;
        }

        private void FileView_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed || _draggedFile == null || _isDragging) return;

            Point pos = e.GetPosition(null);
            if (Math.Abs(pos.X - _dragStartPoint.X) < SystemParameters.MinimumHorizontalDragDistance &&
                Math.Abs(pos.Y - _dragStartPoint.Y) < SystemParameters.MinimumVerticalDragDistance) return;

            _isDragging = true;
            if (!FileView.SelectedItems.Contains(_draggedFile))
                FileView.SelectedItem = _draggedFile;

            var paths = GetSelectedFiles().Select(file => Path.GetFullPath(file.FullPath)).ToArray();
            var data = new DataObject();
            data.SetData(DataFormats.FileDrop, paths);
            data.SetData("PixelTool.AssetPaths", paths);
            DragDrop.DoDragDrop(FileView, data, DragDropEffects.Copy | DragDropEffects.Move);
            _isDragging = false;
            _preserveSelectionForDrag = false;
            _draggedFile = null;
        }

        private void FileView_Drop(object sender, DragEventArgs e)
        {
            DropPaths(e, nowFolder?.fullPath);
        }

        private void FileView_DragOver(object sender, DragEventArgs e) => SetDropEffect(e);
        private void FolderView_DragOver(object sender, DragEventArgs e) => SetDropEffect(e);

        private void FolderView_Drop(object sender, DragEventArgs e)
        {
            var target = GetFolderItemAtPoint(e.GetPosition(FolderView));
            DropPaths(e, target?.fullPath);
        }

        private static void SetDropEffect(DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                e.Effects = DragDropEffects.None;
                e.Handled = true;
                return;
            }

            bool internalDrag = e.Data.GetDataPresent("PixelTool.AssetPaths");
            bool copy = (e.KeyStates & DragDropKeyStates.ControlKey) != 0;
            e.Effects = internalDrag && !copy ? DragDropEffects.Move : DragDropEffects.Copy;
            e.Handled = true;
        }

        private void DropPaths(DragEventArgs e, string? destinationDirectory)
        {
            if (string.IsNullOrWhiteSpace(destinationDirectory) || !e.Data.GetDataPresent(DataFormats.FileDrop)) return;
            var paths = e.Data.GetData(DataFormats.FileDrop) as string[];
            if (paths == null || paths.Length == 0) return;

            bool internalDrag = e.Data.GetDataPresent("PixelTool.AssetPaths");
            bool move = internalDrag && (e.KeyStates & DragDropKeyStates.ControlKey) == 0;
            TransferPaths(paths, destinationDirectory, move);
        }

        private void CopyFilesToClipboard(IReadOnlyList<FileItem> files, bool cut)
        {
            if (files.Count == 0) return;
            try
            {
                var paths = files.Select(file => Path.GetFullPath(file.FullPath)).ToArray();
                var data = new DataObject();
                data.SetData(DataFormats.FileDrop, paths);
                data.SetData("Preferred DropEffect", new MemoryStream(new[] { cut ? (byte)2 : (byte)1, (byte)0, (byte)0, (byte)0 }));
                Clipboard.SetDataObject(data, true);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Copy failed: {ex.Message}"); }
        }

        private void PasteClipboard(string? destinationDirectory)
        {
            if (string.IsNullOrWhiteSpace(destinationDirectory)) return;
            try
            {
                var data = Clipboard.GetDataObject();
                var paths = data?.GetData(DataFormats.FileDrop) as string[];
                if (paths == null || paths.Length == 0) return;
                TransferPaths(paths, destinationDirectory, IsClipboardCut(data));
            }
            catch (Exception ex) { PixelMessageBox.Show($"Paste failed: {ex.Message}"); }
        }

        private static bool IsClipboardCut(IDataObject? data)
        {
            if (data?.GetData("Preferred DropEffect") is not MemoryStream stream) return false;
            long position = stream.Position;
            stream.Position = 0;
            int effect = stream.ReadByte();
            stream.Position = position;
            return effect == 2;
        }

        private void TransferPaths(IEnumerable<string> sources, string destinationDirectory, bool move)
        {
            try
            {
                foreach (string source in sources.Distinct(StringComparer.OrdinalIgnoreCase))
                {
                    string fullSource = Path.GetFullPath(source);
                    string destination = Path.Combine(destinationDirectory, Path.GetFileName(fullSource));
                    if (Path.GetFullPath(destination).Equals(fullSource, StringComparison.OrdinalIgnoreCase))
                    {
                        if (move) continue;
                        destination = GetUniquePath(destinationDirectory, Path.GetFileNameWithoutExtension(fullSource), Path.GetExtension(fullSource));
                    }
                    else if (File.Exists(destination) || Directory.Exists(destination))
                    {
                        destination = GetUniquePath(destinationDirectory, Path.GetFileNameWithoutExtension(fullSource), Path.GetExtension(fullSource));
                    }

                    if (File.Exists(fullSource))
                    {
                        if (move) File.Move(fullSource, destination);
                        else File.Copy(fullSource, destination);
                    }
                    else if (Directory.Exists(fullSource))
                    {
                        if (IsSubPath(destination, fullSource)) continue;
                        if (move) Directory.Move(fullSource, destination);
                        else CopyDirectory(fullSource, destination);
                    }
                }
                Refresh();
                SelectFolderItem(destinationDirectory);
            }
            catch (Exception ex) { PixelMessageBox.Show($"File operation failed: {ex.Message}"); }
        }

        private static void CopyDirectory(string source, string destination)
        {
            Directory.CreateDirectory(destination);
            foreach (string file in Directory.GetFiles(source))
                File.Copy(file, Path.Combine(destination, Path.GetFileName(file)));
            foreach (string directory in Directory.GetDirectories(source))
                CopyDirectory(directory, Path.Combine(destination, Path.GetFileName(directory)));
        }

        private static bool IsSubPath(string candidate, string parent)
        {
            string parentPath = Path.TrimEndingDirectorySeparator(Path.GetFullPath(parent)) + Path.DirectorySeparatorChar;
            string candidatePath = Path.GetFullPath(candidate);
            return candidatePath.StartsWith(parentPath, StringComparison.OrdinalIgnoreCase);
        }

        private void CopyExternalFile(string source)
        {
            try
            {
                if (!File.Exists(source)) return;
                string dest = Path.Combine(nowFolder.fullPath, Path.GetFileName(source));
                if (Path.GetFullPath(source).Equals(Path.GetFullPath(dest), StringComparison.OrdinalIgnoreCase)) return;

                if (File.Exists(dest))
                    dest = GetUniquePath(nowFolder.fullPath, Path.GetFileNameWithoutExtension(source), Path.GetExtension(source));

                File.Copy(source, dest);
            }
            catch (Exception ex) { PixelMessageBox.Show($"Import failed: {ex.Message}"); }
        }

        private static string GetUniquePath(string directory, string baseName, string extension)
        {
            string path = Path.Combine(directory, baseName + extension);
            int index = 1;
            while (File.Exists(path) || Directory.Exists(path))
            {
                path = Path.Combine(directory, $"{baseName}_{index}{extension}");
                index++;
            }
            return path;
        }

        private static void OpenExplorer(string argument)
        {
            try
            {
                Process.Start(new ProcessStartInfo
                {
                    FileName = "explorer.exe",
                    Arguments = argument,
                    UseShellExecute = true
                });
            }
            catch (Exception ex) { PixelMessageBox.Show($"Open explorer failed: {ex.Message}"); }
        }

        private static void CopyText(string text)
        {
            try { Clipboard.SetText(text); }
            catch (Exception ex) { PixelMessageBox.Show($"Copy failed: {ex.Message}"); }
        }

        private void TreeViewItem_PreviewMouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is TreeViewItem item)
            {
                item.IsSelected = true;
                item.Focus();
                e.Handled = true;
            }
        }

        private void FileViewItem_PreviewMouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is not ListViewItem item) return;
            if (!item.IsSelected)
            {
                FileView.SelectedItems.Clear();
                item.IsSelected = true;
            }
            item.Focus();
        }

        private FileItem GetFileItemFromSender(object sender)
        {
            var menuItem = sender as MenuItem;
            if (menuItem?.DataContext is FileItem file)
                return file;
            return FileView.SelectedItem as FileItem;
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
