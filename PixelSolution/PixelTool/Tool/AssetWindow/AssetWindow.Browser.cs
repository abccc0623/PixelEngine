using System;
using System.ComponentModel;
using System.Collections.Generic;
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
        private FileSystemWatcher? _directoryWatcher;
        private DispatcherTimer? _refreshTimer;
        private readonly object _refreshLock = new();
        private readonly HashSet<string> _changedAssetPaths = new(StringComparer.OrdinalIgnoreCase);
        private bool _fullRefreshPending;
        private bool _refreshDispatchPending;
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

        internal void RefreshAfterAiChange()
        {
            // Watchers batch updates while loaded. Preserve manual receipts if setup failed.
            if (IsLoaded && _watcher == null) Refresh();
        }

        private void StartWatcher()
        {
            if (_watcher != null || !Directory.Exists(ProjectPathService.AssetPath)) return;
            _refreshTimer = new DispatcherTimer(DispatcherPriority.Background) { Interval = TimeSpan.FromMilliseconds(250) };
            _refreshTimer.Tick += (_, _) => RefreshChangedAssets();
            try
            {
                _watcher = new FileSystemWatcher(ProjectPathService.AssetPath)
                {
                    IncludeSubdirectories = true,
                    NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite | NotifyFilters.Size
                };
                _watcher.Created += AssetChanged;
                _watcher.Deleted += AssetChanged;
                _watcher.Changed += AssetChanged;
                _watcher.Renamed += AssetRenamed;
                _watcher.Error += (_, _) => QueueRefresh();
                // A separate directory-only stream distinguishes deleted folders from files
                // without probing paths that no longer exist (including atomic-save temp files).
                _directoryWatcher = new FileSystemWatcher(ProjectPathService.AssetPath)
                {
                    IncludeSubdirectories = true,
                    NotifyFilter = NotifyFilters.DirectoryName
                };
                _directoryWatcher.Created += DirectoryChanged;
                _directoryWatcher.Deleted += DirectoryChanged;
                _directoryWatcher.Renamed += AssetRenamed;
                _directoryWatcher.Error += (_, _) => QueueRefresh();
                _watcher.EnableRaisingEvents = true;
                _directoryWatcher.EnableRaisingEvents = true;
            }
            catch (Exception ex) { StopWatcher(); System.Diagnostics.Debug.WriteLine("Asset watcher: " + ex.Message); }
        }

        private void StopWatcher()
        {
            _refreshTimer?.Stop();
            var watcher = _watcher;
            var directoryWatcher = _directoryWatcher;
            lock (_refreshLock)
            {
                _watcher = null;
                _directoryWatcher = null;
                _changedAssetPaths.Clear();
                _fullRefreshPending = false;
                _refreshDispatchPending = false;
            }
            watcher?.Dispose();
            directoryWatcher?.Dispose();
            SaveBrowserState();
        }

        private void AssetChanged(object sender, FileSystemEventArgs e)
        {
            if (ReferenceEquals(sender, _watcher)) QueueRefresh(e.FullPath);
        }

        private void DirectoryChanged(object sender, FileSystemEventArgs e)
        {
            if (ReferenceEquals(sender, _directoryWatcher)) QueueRefresh();
        }

        private void AssetRenamed(object sender, RenamedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (_watcher == null || (!ReferenceEquals(sender, _watcher) && !ReferenceEquals(sender, _directoryWatcher))) return;
                UpdateMovedPaths(e.OldFullPath,e.FullPath);
                if (ReferenceEquals(sender, _directoryWatcher)) QueueRefresh();
                else
                {
                    QueueRefresh(e.OldFullPath);
                    QueueRefresh(e.FullPath);
                }
            }), DispatcherPriority.Background);
        }

        private void QueueRefresh(string? changedPath = null)
        {
            if (Dispatcher.HasShutdownStarted) return;
            FileSystemWatcher watcher;
            lock (_refreshLock)
            {
                if (_watcher == null) return;
                watcher = _watcher;
                if (changedPath == null) _fullRefreshPending = true;
                else _changedAssetPaths.Add(changedPath);
                // Coalesce before posting so a burst cannot flood the render dispatcher.
                if (_refreshDispatchPending) return;
                _refreshDispatchPending = true;
            }
            Dispatcher.BeginInvoke(new Action(() =>
            {
                lock (_refreshLock)
                {
                    if (!ReferenceEquals(watcher, _watcher)) return;
                    _refreshDispatchPending = false;
                }
                if (_refreshTimer == null) return;
                _refreshTimer.Stop();
                _refreshTimer.Start();
            }), DispatcherPriority.Background);
        }

        private void RefreshChangedAssets()
        {
            if (_isDragging) return;
            _refreshTimer?.Stop();
            bool fullRefresh;
            string[] changedPaths;
            lock (_refreshLock)
            {
                fullRefresh = _fullRefreshPending;
                changedPaths = _changedAssetPaths.ToArray();
                _fullRefreshPending = false;
                _changedAssetPaths.Clear();
            }
            if (fullRefresh) { Refresh(); return; }
            // File writes, creations, deletions and atomic replacements do not change folders.
            if (nowFolder == null || !changedPaths.Any(path =>
                string.Equals(Path.GetDirectoryName(path), nowFolder.fullPath, StringComparison.OrdinalIgnoreCase))) return;
            var selectedFiles = GetSelectedFiles().Select(file => file.FullPath).ToHashSet(StringComparer.OrdinalIgnoreCase);
            SelectFolder(nowFolder.fullPath);
            foreach (var file in FileItems.Where(file => selectedFiles.Contains(file.FullPath))) FileView.SelectedItems.Add(file);
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
