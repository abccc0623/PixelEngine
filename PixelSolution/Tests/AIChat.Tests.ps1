$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function ReadSource($relative) {
    Get-Content -Raw -Encoding UTF8 (Join-Path $solution $relative)
}
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
$client = ReadSource 'PixelTool/AI/CodexChatClient.cs'
$history = ReadSource 'PixelTool/AI/CodexChatHistory.cs'
$view = ReadSource 'PixelTool/Tool/AIChatWindow/AIChatWindow.xaml.cs'
$main = ReadSource 'PixelTool/MainWindow.xaml.cs'
$toolbar = ReadSource 'PixelTool/Tool/ToolbarWindow/ToolbarWindow.xaml.cs'
[xml]$xaml = ReadSource 'PixelTool/Tool/AIChatWindow/AIChatWindow.xaml'
Require ($main.Contains('panel.Children.Insert(panel.Children.IndexOf(luaPane) + 1, pane)')) 'Chat must open beside the Lua editor'
Require ($toolbar.Contains('ShowAIChat()') -and $main.Contains('["AIChat"] = _aiChat')) 'Toolbar and persisted docking must refer to the same chat view'
Require ($client.Contains('start.ArgumentList.Add("app-server")') -and $client.Contains('UseShellExecute = false')) 'Launch the official Codex transport without a shell'
Require ($client.Contains('"initialize"') -and $client.Contains('method = "initialized"')) 'App-server handshake missing'
Require ($view.Contains('type = "chatgpt"') -and $view.Contains('account.GetProperty("type").GetString() != "chatgpt"')) 'Chat must refuse API-key inference'
Require ($client.Contains('start.Environment.Remove("OPENAI_API_KEY")')) 'Do not inherit API-key credentials from the editor'
Require ($client.Contains('start.Environment["PIXELTOOL_MCP_TOKEN"] = token') -and $view.Contains('IsMcpRunning == true')) 'Connect only to this editor instance and keep MCP credentials off the command line'
Require ($view.Contains('"thread/resume"') -and $view.Contains('"turn/interrupt"')) 'Resume and stop controls missing'
Require ($client.Contains('decision = accepted ? "accept" : "decline"')) 'Approval must be explicit and once-only'
Require ($view.Contains('if (AllowEdits.IsChecked != true) return false;') -and $view.Contains('type = "readOnly"')) 'Read-only mode must not approve elevated commands'
Require ($view.Contains('networkAccess = false') -and $view.Contains('writableRoots = new[] { _project }')) 'Writable scope must be the selected project only'
Require ($client.Contains('permissions = new { }, scope = "turn"')) 'Extra permission requests must fail closed'
Require ($history.Contains('SHA256.HashData') -and $history.Contains('"AIChat", key + ".json"')) 'Persist conversations separately per project'
Require ($history.Contains('File.Move(temporary, path, overwrite: true)')) 'History writes must use a temporary file'
Require ($main.Contains('_aiChat.DisposeChat()') -and $client.Contains('process.Kill(entireProcessTree: true)')) 'Owned Codex processes must stop on editor exit'
Require ($client.Contains('ReadLineAsync(_transportStop.Token)') -and $client.Contains('_transportStop.Cancel()')) 'Shutdown must cancel pipe reads rather than wait forever for EOF'
Require ($client.Contains('Task.WhenAll') -and $client.Contains('WaitAsync(TimeSpan.FromSeconds(2))')) 'Transport readers must have a shutdown deadline'
Require ($main.Contains('_dockThemeTimer.Stop()')) 'Stop native FPS callbacks when the main window closes'
$graphics = ReadSource 'PixelTool/Tool/GraphicsWindow/GraphicsWindow.cs'
$destroy = $graphics.Substring($graphics.IndexOf('protected override void DestroyWindowCore'))
Require (-not $destroy.Contains('lock (_engineLock)')) 'Closing must not re-enter an unbounded native render lock after the timed join'
$guide = ReadSource 'PixelTool/AI/LuaApiGuide.md'
$loader = ReadSource 'PixelTool/AI/EngineGuide.cs'
[xml]$project = ReadSource 'PixelTool/PixelTool.csproj'
foreach ($name in @('AIRestrictions.md', 'EditorGuide.md', 'LuaApiGuide.md')) {
    $resource = @($project.Project.ItemGroup.EmbeddedResource | Where-Object Include -eq ('AI\' + $name))
    Require ($resource.Count -eq 1 -and $resource[0].LogicalName -eq ('PixelTool.AI.' + $name)) "Required guide must be embedded: $name"
    Require ($loader.Contains('"' + $name + '"')) "Loader must require guide: $name"
}
Require ($loader.Contains('Engine/*.lua') -and $loader.Contains('AGENTS.md')) 'API verification and supplementary project instructions must remain available'
Require ($loader.Contains('IsNullOrWhiteSpace(text)') -and $loader.Contains('throw new InvalidOperationException')) 'Missing or empty policy documents must fail closed'
Require (([regex]::Matches($view, 'developerInstructions = EngineGuide.Load\(\)')).Count -eq 2) 'Both new and resumed threads must receive developer instructions'
Require ($view.Contains('text = context + text') -eq $false -and $guide.Contains('Entity.CreateCamera()')) 'Application guides must not masquerade as user input'
Require ($view.Contains('"model/list"') -and $view.Contains('nextCursor') -and $view.Contains('includeHidden = false')) 'Model picker must use the paginated visible server catalog'
Require ($view.Contains('model = selected.Model') -and $view.Contains('effort = selected.DefaultEffort')) 'Turn must use selected model and compatible default effort'
Require ($history.Contains('public string? Model') -and $view.Contains('m.Model == savedModel')) 'Model choice must persist and restore per project'
Require ($xaml.SelectSingleNode('//*[@*[local-name()="Name"]="ModelBox"]') -ne $null -and $view.Contains('ModelBox.IsEnabled = !busy')) 'Model selector must be present and locked during a response'
Write-Output 'PASS: AI Chat docking, protocol, account-only access, approval, history and shutdown contracts (static checks only)'
