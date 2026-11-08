$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$app = Get-Content -Raw -Encoding UTF8 (Join-Path $root "PixelTool/App.xaml.cs")
$main = Get-Content -Raw -Encoding UTF8 (Join-Path $root "PixelTool/MainWindow.xaml.cs")
$toolbar = Get-Content -Raw -Encoding UTF8 (Join-Path $root "PixelTool/Tool/ToolbarWindow/ToolbarWindow.xaml.cs")
[xml]$ui = Get-Content -Raw -Encoding UTF8 (Join-Path $root "PixelTool/Tool/ToolbarWindow/ToolbarWindow.xaml")
function Require($condition, $message) { if (-not $condition) { throw $message } }
Require ($ui.SelectSingleNode('//*[@Click="ChangeProject"]') -ne $null -and $toolbar.Contains("main.ChangeProject()")) "The toolbar must invoke project switching"
$closeStart = $main.IndexOf("private void MainWindow_Closing")
$closeEnd = $main.IndexOf("internal void ChangeProject", $closeStart)
$close = $main.Substring($closeStart, $closeEnd - $closeStart)
Require ($close.IndexOf("app.ProjectRestart =") -gt $close.IndexOf("ConfirmCloseAll()") -and $close.IndexOf("app.ProjectRestart =") -gt $close.IndexOf("ConfirmCloseTools()") -and $close.Contains("e.Cancel = true")) "Restart must be armed only after all save/cancel guards accept closing"
Require ($main.Contains("finally { _projectRestart = null; }") -and $main.Contains("MessageBoxResult.Yes")) "Cancelling must not leave a pending project restart"
$exit = $app.Substring($app.IndexOf("protected override void OnExit"))
Require ($exit.IndexOf("Process.Start(restart)") -gt $exit.IndexOf("_mcpServer.DisposeAsync()") -and $exit.IndexOf("Process.Start(restart)") -gt $exit.IndexOf("base.OnExit(e)")) "The next process must start after editor and MCP cleanup"
Require ($app.IndexOf("startupWindow.ShowDialog()") -lt $app.IndexOf("var mainWindow = new MainWindow()")) "Restart must return to the existing project selector before loading an engine"
Write-Output "PASS: project switch wiring, save/cancel ordering, cleanup-before-restart and project selection contracts (static checks only)"
