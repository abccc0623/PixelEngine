param(
    [string]$BaseRef = 'HEAD',
    [string[]]$ChangedPaths,
    [switch]$AcknowledgeNoFlowChange
)

$repoRoot = Split-Path $PSScriptRoot -Parent
if (-not $PSBoundParameters.ContainsKey('ChangedPaths')) {
    $tracked = @(& git -C $repoRoot diff --name-only $BaseRef --)
    if ($LASTEXITCODE -ne 0) { throw "Cannot compare changes with $BaseRef" }
    $untracked = @(& git -C $repoRoot ls-files --others --exclude-standard)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot list untracked files' }
    $ChangedPaths = @($tracked) + @($untracked)
}

$paths = @($ChangedPaths | ForEach-Object { $_.Replace('\', '/') })
$diagram = 'PixelSolution/AI_PIPELINE.md'
$flowFiles = @($paths | Where-Object {
    $_ -match '^PixelSolution/PixelTool/AI/(CodexEditorClient|CodexChatClient|CodexInstructions|ChatGptProjectTools|ProjectWorkItemWiki)\.cs$' -or
    $_ -match '^PixelSolution/PixelTool/Mcp/PixelToolMcpServer\.cs$' -or
    $_ -match '^PixelSolution/PixelTool/Tool/AIChatWindow/AIChatWindow\.xaml\.cs$' -or
    $_ -match '^PixelSolution/PixelTool/LLMWiki/Skills/Pipeline/[^/]+\.md$'
})

if ($flowFiles.Count -eq 0) { Write-Output 'AI flow files unchanged.'; exit 0 }
if ($paths -contains $diagram) { Write-Output 'AI flow diagram was changed alongside flow files; review its accuracy manually.'; exit 0 }
if ($AcknowledgeNoFlowChange) { Write-Output 'Flow-file change acknowledged as not changing the AI pipeline.'; exit 0 }
Write-Error "AI flow files changed without $diagram. Review and update the diagram, or use -AcknowledgeNoFlowChange when the flow is unchanged."
exit 1
