$ErrorActionPreference = 'Stop'
$folder = Join-Path $PSScriptRoot '../PixelTool/Tool/EditorLuaWindow'
$editor = Get-Content (Join-Path $folder 'LuaEditor.xaml.cs') -Raw
$refresh = Get-Content (Join-Path $folder 'LuaEditor.ExternalFiles.cs') -Raw
function Require([bool]$condition, [string]$message) {
    if (-not $condition) { throw $message }
}
Require ($editor.Contains('file.DiskText = file.Document.Text;')) 'Initial/save disk snapshot missing'
Require ($editor.Contains('if (!CheckExternalFile(file, true)) return false;')) 'Save conflict guard missing'
Require ($editor.Contains('externalFileTimer.Stop();')) 'Dispose must stop refresh'
Require ($refresh.Contains('Loaded +=') -and $refresh.Contains('Unloaded +=')) 'Refresh lifecycle missing'
Require ($refresh.Contains('if (!file.Document.UndoStack.IsOriginalFile)')) 'Dirty document protection missing'
Require ($refresh.Contains('diskText == file.DeferredDiskText')) 'Cancel must not reopen every tick'
Require ($refresh.Contains('MessageBoxButton.YesNoCancel')) 'Conflict choices missing'
Require ($refresh.Contains('UndoStack.ClearAll()') -and $refresh.Contains('UndoStack.MarkAsOriginalFile()')) 'Reload must reset undo/save state'
Require ($refresh.Contains('ScrollToVerticalOffset(vertical)') -and $refresh.Contains('ScrollToHorizontalOffset(horizontal)')) 'Scroll restoration missing'
Require ($refresh.Contains('documentSync = SyncDocumentAsync')) 'LSP refresh missing'
Require (-not $refresh.Contains('PixelEngineNative.Reload')) 'External refresh must not restart gameplay'
Write-Output 'PASS: Lua external refresh source guards (static checks only)'
