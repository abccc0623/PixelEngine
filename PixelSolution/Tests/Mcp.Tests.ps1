$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
$server = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/Mcp/PixelToolMcpServer.cs')
$app = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/App.xaml.cs')
$project = [xml](Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/PixelTool.csproj'))
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
Require ($server.Contains('options.Listen(IPAddress.Loopback, Port)')) 'MCP must bind only to loopback'
Require ($server.Contains('builder.Configuration.Sources.Clear()')) 'Environment must not add public endpoints'
Require ($server.Contains('context.Request.Headers.ContainsKey("Origin")')) 'Browser-origin requests must be rejected'
Require ($server.Contains('CryptographicOperations.FixedTimeEquals(supplied, authorization)')) 'MCP requires token authentication'
Require ($server.Contains('security.SetAccessRuleProtection(true, false)')) 'Connection credentials require a user-only directory'
Require ($server.Contains('RandomNumberGenerator.GetBytes(32)')) 'Token must be cryptographically random'
Require ($server.IndexOf('application.Use(') -lt $server.IndexOf('application.MapMcp(')) 'Authentication must precede MCP tools'
Require (([regex]::Matches($server, 'McpServerTool\(Name').Count -eq 3) -and
    $server.Contains('Name = "get_project_info", ReadOnly = true') -and
    $server.Contains('Name = "get_editor_context", ReadOnly = true') -and
    $server.Contains('Name = "highlight_lua_lines", ReadOnly = false')) 'Expose context and Lua highlight tools'
Require ($server.Contains('app.Dispatcher.InvokeAsync') -and $server.Contains('window.HighlightLuaLines(path, startLine, endLine)')) 'Highlight must enter the UI thread'
Require ($server.Contains('PixelToolEditorContextSnapshot.Current')) 'Editor context tool must read the thread-safe snapshot'
Require ($server.Contains('public sealed class PixelToolProjectTools') -and -not $server.Contains('static class PixelToolProjectTools')) 'WithTools<T> cannot accept a static class as its type argument'
Require (-not ($server -match 'PixelEngineNative\.|Entity_Create|Asset_Import')) 'HTTP handlers must not call the native engine'
Require ($server.Contains('AddSingleton(new PixelToolProjectInfo(')) 'Use a snapshot instead of HTTP-thread UI access'
Require ($app.IndexOf('_mcpServer.StartAsync()') -gt $app.IndexOf('mainWindow.Show();')) 'Start MCP only after project selection and editor startup'
Require ($app.Contains('_mcpServer.DisposeAsync()')) 'Editor exit must dispose the MCP listener'
Require ($app.Contains('WaitAsync(TimeSpan.FromSeconds(4))')) 'MCP cleanup must not keep the editor alive indefinitely'
$package = @($project.Project.ItemGroup.PackageReference | Where-Object Include -eq 'ModelContextProtocol.AspNetCore')
Require ($package.Count -eq 1 -and $package[0].Version -eq '2.3.0') 'Pin the official HTTP MCP SDK'
Write-Output 'PASS: MCP local-only security, read-only context tools, lifecycle and SDK contracts (static checks only)'
