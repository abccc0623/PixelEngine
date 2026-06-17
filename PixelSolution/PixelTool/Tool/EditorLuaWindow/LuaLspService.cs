using ICSharpCode.AvalonEdit.CodeCompletion;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using Newtonsoft.Json.Linq;
using StreamJsonRpc;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace PixelTool
{
    public class LuaLspService : IDisposable
    {
        public event Action<PublishDiagnosticParams> DiagnosticsPublished;

        private int _documentVersion = 1;
        private Process _luaServerProcess;
        private JsonRpc _rpc;
        private string _targetFilePath;
        private int _currentLine;
        private int _currentColumn;

        private readonly SemaphoreSlim _completionLock = new SemaphoreSlim(1, 1);
        private bool _disposed = false;

        public async Task Initialize()
        {
            await StartLanguageServerAsync();
        }

        private async Task StartLanguageServerAsync()
        {
            try
            {
                string baseDir = AppDomain.CurrentDomain.BaseDirectory;
                string serverPath = Path.GetFullPath(Path.Combine(baseDir, "LSP/bin/lua-language-server.exe"));
                string finalRootUri = new Uri(ProjectPathService.ProjectRootPath).AbsoluteUri;

                // .luarc.json 자동 생성
                string enginePath = ProjectPathService.EnginePath.Replace("\\", "/");
                string luarcPath = ProjectPathService.GetEditorFilePath(".luarc.json");
                string luarcContent = $@"{{
    ""runtime"": {{ ""version"": ""Lua 5.1"" }},
    ""workspace"": {{ ""library"": [""{enginePath}""] }},
    ""diagnostics"": {{ ""globals"": [""Vector2"", ""Vector3"", ""Transform"", ""Renderer2D"", ""Camera"", ""Animation2D"", ""BoxCollider2D"", ""CircleCollider2D"", ""Rigidbody2D"", ""LuaEvent""] }}
}}";
                File.WriteAllText(luarcPath, luarcContent);

                ConsoleWindow.LogMessage($"[DEBUG] 실행 시도 경로: {serverPath}", 0);
                if (!File.Exists(serverPath))
                {
                    ConsoleWindow.LogMessage($"❌ [CRITICAL] 서버 파일 없음: {serverPath}", 2);
                    return;
                }

                string luarcArgPath = luarcPath.Replace("\\", "/");
                var info = new ProcessStartInfo
                {
                    FileName = serverPath,
                    WorkingDirectory = Path.GetDirectoryName(serverPath),
                    Arguments = $"--logpath=\"{Path.Combine(baseDir, "LSPLogs")}\" --configpath=\"{luarcArgPath}\"",
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                _luaServerProcess = new Process { StartInfo = info, EnableRaisingEvents = true };
                _luaServerProcess.ErrorDataReceived += (s, e) =>
                {
                    if (!string.IsNullOrEmpty(e.Data))
                        ConsoleWindow.LogMessage($"⚠️ [SERVER-ERROR]: {e.Data}", 1);
                };
                _luaServerProcess.Exited += (s, e) =>
                {
                    ConsoleWindow.LogMessage($"💀 [PROCESS-EXIT]: ExitCode: {_luaServerProcess?.ExitCode}", 2);
                };

                if (!_luaServerProcess.Start())
                {
                    ConsoleWindow.LogMessage("❌ 프로세스 시작 실패!", 2);
                    return;
                }

                _luaServerProcess.BeginErrorReadLine();

                _rpc = new JsonRpc(_luaServerProcess.StandardInput.BaseStream, _luaServerProcess.StandardOutput.BaseStream);
                _rpc.AddLocalRpcMethod(Methods.TextDocumentPublishDiagnosticsName, new Action<JToken>(HandleDiagnostics));
                _rpc.AddLocalRpcMethod("window/workDoneProgress/create", new Func<JToken, object>(_ => new { }));
                _rpc.StartListening();

                var initParams = new Dictionary<string, object>
                {
                    ["processId"] = Environment.ProcessId,
                    ["rootUri"] = finalRootUri,
                    ["capabilities"] = new
                    {
                        window = new { workDoneProgress = false },
                        textDocument = new
                        {
                            completion = new { completionItem = new { snippetSupport = true } }
                        }
                    }
                };

                var result = await _rpc.InvokeAsync<JObject>(Methods.InitializeName, initParams);
                if (result == null)
                {
                    ConsoleWindow.LogMessage("❌ LSP 초기화 실패", 2);
                    return;
                }

                await _rpc.NotifyAsync(Methods.InitializedName);

                string engineGeneratePath = ProjectPathService.GetEngineFilePath("EngineGenerate.lua");
                if (File.Exists(engineGeneratePath))
                {
                    var content = File.ReadAllText(engineGeneratePath);
                    await NotifyFileOpenAsync(engineGeneratePath, content);
                }
                else
                {
                    ConsoleWindow.LogMessage($"❌ [LSP] 마스터 파일 없음: {engineGeneratePath}", 2);
                }
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ [FATAL]: {ex.Message}\n{ex.StackTrace}", 2);
            }
        }

        private void HandleDiagnostics(JToken parameters)
        {
            try
            {
                var diagnostics = parameters.ToObject<PublishDiagnosticParams>();
                if (diagnostics != null)
                {
                    DiagnosticsPublished?.Invoke(diagnostics);
                }
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ LSP 진단 처리 실패: {ex.Message}", 2);
            }
        }

        public async Task NotifyFileOpenAsync(string fileUri, string fileText)
        {
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            string absolutePath = Path.GetFullPath(Path.Combine(baseDir, fileUri));
            string uri = new Uri(absolutePath.Replace("\\", "/")).AbsoluteUri;

            _targetFilePath = uri;

            var didOpenParams = new DidOpenTextDocumentParams
            {
                TextDocument = new TextDocumentItem
                {
                    Uri = new Uri(uri),
                    LanguageId = "lua",
                    Version = _documentVersion++,
                    Text = fileText
                }
            };

            try
            {
                await _rpc.NotifyWithParameterObjectAsync(Methods.TextDocumentDidOpenName, didOpenParams);
                ConsoleWindow.LogMessage($"✅ [LSP] 파일 열기 완료: {uri}", 0);
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ 파일 동기화 실패: {ex.Message}", 2);
            }
        }

        // 텍스트 동기화만 담당 (자동완성 요청 없음)
        public async Task SyncTextAsync(string content, int currentLine, int currentColumn)
        {
            if (_rpc == null || _targetFilePath == null) return;

            int version = Interlocked.Increment(ref _documentVersion);

            var didChangeParams = new DidChangeTextDocumentParams
            {
                TextDocument = new VersionedTextDocumentIdentifier
                {
                    Uri = new Uri(_targetFilePath),
                    Version = version
                },
                ContentChanges = new[]
                {
                    new TextDocumentContentChangeEvent { Text = content }
                }
            };

            try
            {
                await _rpc.NotifyWithParameterObjectAsync(Methods.TextDocumentDidChangeName, didChangeParams);
                _currentLine = currentLine;
                _currentColumn = currentColumn;
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ 텍스트 동기화 실패: {ex.Message}", 2);
            }
        }

        // 자동완성 요청만 담당
        public async Task RequestCompletionAsync(string triggerChar, int currentLine, int currentColumn)
        {
            if (_rpc == null || _targetFilePath == null) return;
            if (!await _completionLock.WaitAsync(0)) return;

            try
            {
                var context = new CompletionContext();
                if (triggerChar != "." && triggerChar != ":")
                    context.TriggerKind = CompletionTriggerKind.Invoked;
                else
                {
                    context.TriggerKind = CompletionTriggerKind.TriggerCharacter;
                    context.TriggerCharacter = triggerChar;
                }

                var completionParams = new CompletionParams
                {
                    TextDocument = new TextDocumentIdentifier { Uri = new Uri(_targetFilePath) },
                    Position = new Position { Line = currentLine, Character = currentColumn },
                    Context = context
                };

                var rawResult = await _rpc.InvokeWithParameterObjectAsync<JToken>(Methods.TextDocumentCompletionName, completionParams);
                CompletionItem[] completionItems = null;

                if (rawResult is JArray array)
                    completionItems = array.ToObject<CompletionItem[]>();
                else if (rawResult?["items"] is JArray itemsArray)
                    completionItems = itemsArray.ToObject<CompletionItem[]>();

                if (completionItems == null || completionItems.Length == 0)
                {
                    ConsoleWindow.LogMessage("⚠️ [LSP] 자동완성 결과 없음", 1);
                    return;
                }

                System.Windows.Application.Current.Dispatcher.Invoke(() =>
                {
                    var luaEditor = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                    if (luaEditor == null) return;

                    luaEditor.completionWindow?.Close();

                    var area = luaEditor.GetLuaEditorTextArea();
                    var completionWindow = new CompletionWindow(area);

                    int caretOffset = area.Caret.Offset;
                    int startOffset = caretOffset;
                    while (startOffset > 0)
                    {
                        char ch = area.Document.GetCharAt(startOffset - 1);
                        if (!char.IsLetterOrDigit(ch) && ch != '_') break;
                        startOffset--;
                    }

                    if (string.IsNullOrEmpty(triggerChar) && caretOffset == startOffset) return;

                    completionWindow.StartOffset = startOffset;
                    completionWindow.CloseAutomatically = true;
                    completionWindow.AllowsTransparency = true;
                    completionWindow.Background = new SolidColorBrush(System.Windows.Media.Color.FromRgb(27, 28, 46));

                    double longestItemWidth = 0;
                    var typeface = new Typeface(area.FontFamily, area.FontStyle, area.FontWeight, area.FontStretch);
                    bool isForCompletion = area.Document.GetText(startOffset, caretOffset - startOffset)
                        .Equals("for", StringComparison.OrdinalIgnoreCase);

                    if (isForCompletion)
                    {
                        completionWindow.CompletionList.CompletionData.Add(new LuaCompletionData("for - ipairs", "for_ipairs"));
                        completionWindow.CompletionList.CompletionData.Add(new LuaCompletionData("for - pairs", "for_pairs"));
                        completionWindow.CompletionList.CompletionData.Add(new LuaCompletionData("for i =", "for_numeric"));
                        longestItemWidth = Math.Max(longestItemWidth, 120);
                    }

                    foreach (var item in completionItems)
                    {
                        if (item.Label.StartsWith("_")) continue;
                        if (isForCompletion && IsLuaForSnippet(item.Label)) continue;

                        var formattedText = new FormattedText(
                            item.Label,
                            CultureInfo.CurrentUICulture,
                            FlowDirection.LeftToRight,
                            typeface,
                            area.FontSize,
                            Brushes.White,
                            VisualTreeHelper.GetDpi(area).PixelsPerDip);
                        longestItemWidth = Math.Max(longestItemWidth, formattedText.WidthIncludingTrailingWhitespace);

                        completionWindow.CompletionList.CompletionData.Add(new LuaCompletionData(item));
                    }

                    if (completionWindow.CompletionList.CompletionData.Count == 0) return;

                    double availableWidth = Math.Max(300, area.ActualWidth - 32);
                    completionWindow.Width = Math.Min(availableWidth, Math.Max(300, longestItemWidth + 72));
                    completionWindow.MaxHeight = 320;

                    var completionListBox = completionWindow.CompletionList.ListBox;
                    ScrollViewer.SetVerticalScrollBarVisibility(completionListBox, ScrollBarVisibility.Visible);
                    ScrollViewer.SetHorizontalScrollBarVisibility(completionListBox, ScrollBarVisibility.Disabled);

                    luaEditor.completionWindow = completionWindow;
                    completionWindow.Show();
                    completionWindow.Closed += (s, e) => luaEditor.completionWindow = null;
                });
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ 자동완성 요청 실패: {ex.Message}", 2);
            }
            finally
            {
                _completionLock.Release();
            }
        }

        private static bool IsLuaForSnippet(string label)
        {
            string normalized = label.Trim().ToLowerInvariant();
            return normalized.Contains("ipairs") ||
                   normalized.Contains("pairs") ||
                   normalized.Contains("for i");
        }

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;

            _completionLock?.Dispose();

            try { _rpc?.Dispose(); } catch { }
            try
            {
                if (_luaServerProcess != null && !_luaServerProcess.HasExited)
                {
                    _luaServerProcess.Kill();
                    _luaServerProcess.Dispose();
                }
            }
            catch { }
        }
    }
}
