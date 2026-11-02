using System;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class AssetWindow
    {
        private AssetBrowserState _browserState = new();
        private bool _restoringTree;
        private FileSystemWatcher? _watcher;
        private DispatcherTimer? _refreshTimer;
        private Point _folderDragStart;
        private FolderItem? _draggedFolder;
        private string BrowserStatePath => ProjectPathService.GetEditorFilePath("AssetBrowser.json");

        private void InitializeBrowser()
        {
            try { _browserState = AssetBrowserState.Load(BrowserStatePath); }
            catch (Exception ex) { System.Diagnostics.Debug.WriteLine("Asset browser state: " + ex.Message); }
            Loaded += (_, _) => { StartWatcher(); Refresh(); };
            Unloaded += (_, _) => StopWatcher();
        }

        private FolderItem CreateFolderItem(string path)
        {
            var item = new FolderItem(path);
            string relative = Path.GetRelativePath(ProjectPathService.AssetPath, path);
            item.IsExpanded = _browserState.Expanded.TryGetValue(relative, out bool expanded) ? expanded : relative == ".";
            item.PropertyChanged += FolderStateChanged;
            return item;
        }

        private void FolderStateChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (_restoringTree || sender is not FolderItem folder || e.PropertyName != nameof(FolderItem.IsExpanded)) return;
            _browserState.Expanded[Path.GetRelativePath(ProjectPathService.AssetPath, folder.fullPath)] = folder.IsExpanded;
            SaveBrowserState();
        }

        private void SaveBrowserState()
        {
            if (_restoringTree) return;
            if (nowFolder != null) _browserState.Selected = Path.GetRelativePath(ProjectPathService.AssetPath, nowFolder.fullPath);
            try { _browserState.Save(BrowserStatePath); }
            catch (Exception ex) { System.Diagnostics.Debug.WriteLine("Asset browser state: " + ex.Message); }
        }

        private void StartWatcher()
        {
            if (_watcher != null || !Directory.Exists(ProjectPathService.AssetPath)) return;
            _refreshTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(250) };
            _refreshTimer.Tick += (_, _) => { if (_isDragging) return; _refreshTimer.Stop(); Refresh(); };
            try
            {
                _watcher = new FileSystemWatcher(ProjectPathService.AssetPath)
                {
                    IncludeSubdirectories = true,
                    NotifyFilter = NotifyFilters.FileName | NotifyFilters.DirectoryName | NotifyFilters.LastWrite | NotifyFilters.Size
                };
                _watcher.Created += AssetChanged;
                _watcher.Deleted += AssetChanged;
                _watcher.Changed += AssetChanged;
                _watcher.Renamed += AssetRenamed;
                _watcher.Error += (_, _) => QueueRefresh();
                _watcher.EnableRaisingEvents = true;
            }
            catch (Exception ex) { StopWatcher(); System.Diagnostics.Debug.WriteLine("Asset watcher: " + ex.Message); }
        }

        private void StopWatcher()
        {
            _refreshTimer?.Stop();
            _watcher?.Dispose();
            _watcher = null;
            SaveBrowserState();
        }

        private void AssetChanged(object sender, FileSystemEventArgs e) => QueueRefresh();

        private void AssetRenamed(object sender, RenamedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (_watcher == null) return;
                UpdateMovedPaths(e.OldFullPath,e.FullPath);
                QueueRefresh();
            }));
        }

        private void QueueRefresh()
        {
            if (Dispatcher.HasShutdownStarted) return;
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (_watcher == null || _refreshTimer == null) return;
                _refreshTimer.Stop();
                _refreshTimer.Start();
            }));
        }

        private void UpdateMovedPaths(string oldPath, string newPath)
        {
            if (nowFolder != null && AssetFileOperations.IsInside(nowFolder.fullPath,oldPath))
                nowFolder.fullPath = Path.Combine(newPath,Path.GetRelativePath(oldPath,nowFolder.fullPath));
            foreach (var pair in _browserState.Expanded.ToArray())
            {
                string full = Path.Combine(ProjectPathService.AssetPath,pair.Key);
                if (!AssetFileOperations.IsInside(full,oldPath)) continue;
                _browserState.Expanded.Remove(pair.Key);
                string moved = Path.Combine(newPath,Path.GetRelativePath(oldPath,full));
                _browserState.Expanded[Path.GetRelativePath(ProjectPathService.AssetPath,moved)] = pair.Value;
            }
            GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.UpdateMovedFilePaths(oldPath,newPath);
            SaveBrowserState();
        }

        private void FolderItem_Copy(object sender, RoutedEventArgs e) => CopyFolderToClipboard(false);
        private void FolderItem_Cut(object sender, RoutedEventArgs e) => CopyFolderToClipboard(true);

        private void CopyFolderToClipboard(bool cut)
        {
            if (nowFolder == null || nowFolder == rootFolder) return;
            CopyPathsToClipboard(new[] { nowFolder.fullPath },cut);
        }

        private void FolderView_KeyDown(object sender, KeyEventArgs e)
        {
            if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
            {
                if (e.Key == Key.C) CopyFolderToClipboard(false);
                else if (e.Key == Key.X) CopyFolderToClipboard(true);
                else if (e.Key == Key.V) PasteClipboard(nowFolder?.fullPath);
                else return;
            }
            else if (e.Key == Key.F2) FolderItem_Rename(sender,e);
            else if (e.Key == Key.Delete) FolderItem_Delete(sender,e);
            else if (e.Key == Key.F5) Refresh();
            else return;
            e.Handled = true;
        }

        private void FolderView_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _folderDragStart = e.GetPosition(FolderView);
            _draggedFolder = GetFolderItemAtPoint(_folderDragStart);
            // The expander is not a draggable folder label.
            for (var element = e.OriginalSource as System.Windows.DependencyObject; element != null;)
            {
                if (element is System.Windows.Controls.Primitives.ToggleButton) { _draggedFolder = null; break; }
                if (element is TreeViewItem) break;
                element = element is System.Windows.Media.Visual || element is System.Windows.Media.Media3D.Visual3D
                    ? System.Windows.Media.VisualTreeHelper.GetParent(element)
                    : LogicalTreeHelper.GetParent(element);
            }
        }

        private void FolderView_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed || _draggedFolder == null || _draggedFolder == rootFolder || _isDragging) return;
            Point point = e.GetPosition(FolderView);
            if (Math.Abs(point.X - _folderDragStart.X) < SystemParameters.MinimumHorizontalDragDistance &&
                Math.Abs(point.Y - _folderDragStart.Y) < SystemParameters.MinimumVerticalDragDistance) return;
            var data = new DataObject();
            var paths = new[] { _draggedFolder.fullPath };
            data.SetData(DataFormats.FileDrop,paths);
            data.SetData("PixelTool.AssetPaths",paths);
            _isDragging = true;
            try { DragDrop.DoDragDrop(FolderView,data,DragDropEffects.Move | DragDropEffects.Copy); }
            finally { _isDragging = false; _draggedFolder = null; }
        }
    }
}
