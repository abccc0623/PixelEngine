using ICSharpCode.AvalonEdit.Document;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class LuaEditorWindow
    {
        private readonly DispatcherTimer externalFileTimer = new DispatcherTimer();
        private bool checkingExternalFiles;

        private void InitializeExternalFileRefresh()
        {
            externalFileTimer.Interval = TimeSpan.FromSeconds(1);
            externalFileTimer.Tick += (_, _) => CheckExternalFiles();
            Loaded += (_, _) => externalFileTimer.Start();
            Unloaded += (_, _) => externalFileTimer.Stop();
        }

        private void CheckExternalFiles()
        {
            if (checkingExternalFiles) return;
            checkingExternalFiles = true;
            try
            {
                foreach (var file in new List<FileState>(files.Values))
                    CheckExternalFile(file, false);
            }
            finally { checkingExternalFiles = false; }
        }

        // False cancels a pending save after reload/cancel, preventing stale writes.
        private bool CheckExternalFile(FileState file, bool saving)
        {
            try
            {
                if (!File.Exists(file.Path)) return true;
                string diskText = File.ReadAllText(file.Path);
                if (diskText == file.DiskText) return true;
                if (diskText == file.Document.Text)
                {
                    file.DiskText = diskText;
                    file.Document.UndoStack.MarkAsOriginalFile();
                    file.Label.Text = Path.GetFileName(file.Path);
                    UpdateFileStatus();
                    return true;
                }
                if (!file.Document.UndoStack.IsOriginalFile)
                {
                    if (!saving && diskText == file.DeferredDiskText) return true;
                    // Defer conflict dialogs for background tabs or an inactive editor.
                    if (!saving && (file != activeFile || Window.GetWindow(this)?.IsActive != true)) return true;
                    bool wasChecking = checkingExternalFiles;
                    checkingExternalFiles = true;
                    MessageBoxResult answer;
                    try
                    {
                        answer = PixelMessageBox.Show(
                            Path.GetFileName(file.Path) + " 파일이 외부에서 변경되었습니다.\n\n" +
                            "예: 외부 파일로 다시 불러오기 (편집 중인 내용은 사라집니다)\n" +
                            "아니요: 내 수정 유지 (다음 저장 시 외부 파일을 덮어씁니다)\n" +
                            "취소: 지금 결정하지 않기",
                            "외부 스크립트 변경", MessageBoxButton.YesNoCancel, MessageBoxImage.Warning);
                    }
                    finally { checkingExternalFiles = wasChecking; }
                    if (answer == MessageBoxResult.Cancel)
                    {
                        file.DeferredDiskText = diskText;
                        return false;
                    }
                    if (answer == MessageBoxResult.No)
                    {
                        file.DiskText = diskText;
                        return true;
                    }
                }
                ReloadExternalFile(file, diskText);
                return false;
            }
            // External editors can briefly lock or replace a file; retry on the next tick.
            catch (IOException) { return !saving; }
            catch (UnauthorizedAccessException) { return !saving; }
        }

        private void ReloadExternalFile(FileState file, string diskText)
        {
            bool active = file == activeFile;
            int caret = active ? LuaEditor.CaretOffset : file.Caret;
            int selectionStart = active ? LuaEditor.SelectionStart : file.SelectionStart;
            int selectionLength = active ? LuaEditor.SelectionLength : file.SelectionLength;
            double vertical = active ? LuaEditor.VerticalOffset : file.VerticalOffset;
            double horizontal = active ? LuaEditor.HorizontalOffset : file.HorizontalOffset;
            bool wasSwitching = switchingFile;
            switchingFile = true;
            try
            {
                if (active)
                {
                    completionWindow?.Close();
                    ClearEditedBreakpoints();
                    diagnosticRenderer.SetErrors(new List<ISegment>());
                }
                file.Document.Text = diskText;
                file.Document.UndoStack.ClearAll();
                file.Document.UndoStack.MarkAsOriginalFile();
                file.DiskText = diskText;
                file.Caret = Math.Min(caret, file.Document.TextLength);
                file.SelectionStart = Math.Min(selectionStart, file.Document.TextLength);
                file.SelectionLength = Math.Min(selectionLength, file.Document.TextLength - file.SelectionStart);
                file.Label.Text = Path.GetFileName(file.Path);
                if (active)
                {
                    LuaEditor.CaretOffset = file.Caret;
                    LuaEditor.Select(file.SelectionStart, file.SelectionLength);
                    LuaEditor.ScrollToVerticalOffset(vertical);
                    LuaEditor.ScrollToHorizontalOffset(horizontal);
                    documentSync = SyncDocumentAsync(documentSync, file.Path, diskText,
                        LuaEditor.TextArea.Caret.Line - 1, LuaEditor.TextArea.Caret.Column - 1);
                }
            }
            finally { switchingFile = wasSwitching; }
            UpdateFileStatus();
        }
    }
}
