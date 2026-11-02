param([string]$AvalonEditDll)

$ErrorActionPreference = 'Stop'
if (-not $AvalonEditDll) {
    $profilePath = [Environment]::GetFolderPath('UserProfile')
    $AvalonEditDll = Join-Path $profilePath '.nuget/packages/avalonedit/6.3.1.120/lib/net462/ICSharpCode.AvalonEdit.dll'
}
Add-Type -AssemblyName PresentationFramework,PresentationCore,WindowsBase
[Reflection.Assembly]::LoadFrom($AvalonEditDll) | Out-Null

function Require([bool]$condition, [string]$message) {
    if (-not $condition) { throw $message }
}

$folder = Join-Path $PSScriptRoot '../PixelTool/Tool/EditorLuaWindow'
$editorSource = Get-Content (Join-Path $folder 'LuaEditor.xaml.cs') -Raw
$lspSource = Get-Content (Join-Path $folder 'LuaLspService.cs') -Raw
Require ($editorSource.Contains('Key key = e.Key == Key.System ? e.SystemKey : e.Key;')) 'Alt key normalization missing'
Require ($editorSource.Contains('if (isShiftPressed)') -and $editorSource.Contains('RectangleSelection.BoxSelectUpByLine') -and $editorSource.Contains('RectangleSelection.BoxSelectDownByLine')) 'Multi-line selection commands missing'
Require ($editorSource.Contains('MoveSelectedLines(key == Key.Up ? -1 : 1);')) 'Alt-only line movement missing'
Require ($editorSource.Contains('e.Key == Key.Escape && LuaEditor.TextArea.Selection is RectangleSelection')) 'Escape cancellation missing'
Require ($editorSource.Contains('if (LuaEditor.TextArea.Selection is RectangleSelection) return;')) 'Completion request guard missing'
Require ($lspSource.Contains('Selection is ICSharpCode.AvalonEdit.Editing.RectangleSelection')) 'Late completion response guard missing'

$area = New-Object ICSharpCode.AvalonEdit.Editing.TextArea
$area.FontFamily = New-Object Windows.Media.FontFamily('Consolas')
$area.Document = New-Object ICSharpCode.AvalonEdit.Document.TextDocument
$area.Options.EnableRectangularSelection = $true
$area.Measure((New-Object Windows.Size(800,600)))
$area.Arrange((New-Object Windows.Rect(0,0,800,600)))

function Select-Column([int]$startLine, [int]$endLine, [int]$column) {
    $area.InvalidateMeasure()
    $area.Measure((New-Object Windows.Size(800,600)))
    $area.Arrange((New-Object Windows.Rect(0,0,800,600)))
    $area.UpdateLayout()
    $area.TextView.EnsureVisualLines()
    $start = New-Object ICSharpCode.AvalonEdit.TextViewPosition($startLine,$column,($column - 1))
    $end = New-Object ICSharpCode.AvalonEdit.TextViewPosition($endLine,$column,($column - 1))
    $area.Selection = New-Object ICSharpCode.AvalonEdit.Editing.RectangleSelection($area,$start,$end)
    $area.Caret.Position = $end
    $area.Caret.DesiredXPos = [double]::NaN
}

$hostWindow = New-Object Windows.Window
$hostWindow.ShowInTaskbar = $false
$hostWindow.ShowActivated = $false
$hostWindow.Opacity = 0
$hostWindow.Left = -32000
$hostWindow.Top = -32000
$hostWindow.Width = 800
$hostWindow.Height = 600
$hostWindow.Content = $area
$hostWindow.Show()
try {
$area.Document.Text = "abcd`nefgh`nijkl"
Select-Column 1 1 2
[ICSharpCode.AvalonEdit.Editing.RectangleSelection]::BoxSelectDownByLine.Execute($null,$area)
[ICSharpCode.AvalonEdit.Editing.RectangleSelection]::BoxSelectDownByLine.Execute($null,$area)
Require (@($area.Selection.Segments).Count -eq 3) 'Repeated Down must add more than two cursors'
[ICSharpCode.AvalonEdit.Editing.RectangleSelection]::BoxSelectDownByLine.Execute($null,$area)
Require (@($area.Selection.Segments).Count -eq 3) 'Document boundary must not add a duplicate cursor'
$area.Selection.ReplaceSelectionWithText('X')
Require ($area.Document.Text -eq "aXbcd`neXfgh`niXjkl") 'Three-line input failed'
# Keep command targeting deterministic without keyboard focus in the off-screen host.
$area.Caret.Position = $area.Selection.EndPosition
[Windows.Documents.EditingCommands]::Backspace.Execute($null,$area)
Require ($area.Document.Text -eq "abcd`nefgh`nijkl") 'Three-line Backspace failed'
$area.Document.UndoStack.Undo()
Require ($area.Document.Text -eq "aXbcd`neXfgh`niXjkl") 'Grouped undo failed'
Select-Column 1 3 3
[Windows.Documents.EditingCommands]::Delete.Execute($null,$area)
Require ($area.Document.Text -eq "aXcd`neXgh`niXkl") 'Three-line Delete failed'

$area.Document.Text = "abcd`n`nxy"
Select-Column 3 3 3
[ICSharpCode.AvalonEdit.Editing.RectangleSelection]::BoxSelectUpByLine.Execute($null,$area)
[ICSharpCode.AvalonEdit.Editing.RectangleSelection]::BoxSelectUpByLine.Execute($null,$area)
$area.Selection.ReplaceSelectionWithText('X')
Require ($area.Document.Text -eq "abXcd`n  X`nxyX") 'Upward selection / empty-line padding failed'
$area.ClearSelection()
Require (-not ($area.Selection -is [ICSharpCode.AvalonEdit.Editing.RectangleSelection])) 'Selection cancellation failed'
Write-Output 'PASS: shortcut source guards; AvalonEdit Up/Down commands, multi-line input, Backspace/Delete, undo, empty lines, boundaries, cancellation. No full editor build/UI test.'
} finally {
    $hostWindow.Close()
}
