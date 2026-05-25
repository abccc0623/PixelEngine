using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Editing;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using Newtonsoft.Json.Linq;
using StreamJsonRpc;
using System;
using System.Collections.Generic; // Queue 작동을 위해 필수 포함
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Media;
using System.Windows.Shapes;

namespace PixelTool
{
    public class LuaLspService
    {
        private int _documentVersion = 1;
        private Process _luaServerProcess;
        private JsonRpc _rpc;
        private string targetFilePath;
        private int currentLine;
        private int currentColumn;
        private string content = string.Empty;

        private Queue<LSPMessage> ChangeMessageQueue = new Queue<LSPMessage>();
        private class LSPMessage
        {
            public enum MessageType
            {
                NONE = 0,
                CHANGE = 1,
            }
            public int version = 0;
            public MessageType Type = MessageType.NONE;
            public string TriggerChar = "";
        }

        public async Task Initialize()
        {
            await StartLanguageServerAsync();
        }

        private async Task StartLanguageServerAsync()
        {
            try
            {
                // 1. 절대 경로 확정 및 파일 체크
                string baseDir = AppDomain.CurrentDomain.BaseDirectory;
                string serverPath = System.IO.Path.GetFullPath(System.IO.Path.Combine(baseDir, "LSP/bin/lua-language-server.exe"));

                // 루아 서버 전용 소문자 표준화 경로 가공
                string assetDir = System.IO.Path.Combine(baseDir, "Asset").Replace("\\", "/");
                string engineDir = System.IO.Path.Combine(baseDir, "Asset", "Engine").Replace("\\", "/");

                if (assetDir.Length > 1 && assetDir[1] == ':')
                {
                    assetDir = char.ToLower(assetDir[0]) + assetDir.Substring(1);
                    engineDir = char.ToLower(engineDir[0]) + engineDir.Substring(1);
                }

                string finalRootUri = "file:///" + assetDir;
                string finalLibraryUri = "file:///" + engineDir;

                ConsoleWindow.LogMessage($"[DEBUG] 실행 시도 경로: {serverPath}", 0);

                if (!File.Exists(serverPath))
                {
                    ConsoleWindow.LogMessage($"❌ [CRITICAL] 서버 파일이 존재하지 않음! 경로를 확인해: {serverPath}", 2);
                    return;
                }

                var info = new ProcessStartInfo
                {
                    FileName = serverPath,
                    WorkingDirectory = System.IO.Path.GetDirectoryName(serverPath),
                    Arguments = $"--logpath=\"{System.IO.Path.Combine(baseDir, "LSPLogs")}\"",
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true, // 🔥 서버 내부 로그 추적용
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
                    ConsoleWindow.LogMessage($"💀 [PROCESS-EXIT]: 서버가 종료됨. ExitCode: {_luaServerProcess.ExitCode}", 2);
                };

                if (!_luaServerProcess.Start())
                {
                    ConsoleWindow.LogMessage("❌ 프로세스 시작 함수 자체가 실패함!", 2);
                    return;
                }

                _luaServerProcess.BeginErrorReadLine();

                // 2. RPC 설정 및 메시지 리스닝 활성화
                _rpc = new JsonRpc(_luaServerProcess.StandardInput.BaseStream, _luaServerProcess.StandardOutput.BaseStream);
                _rpc.AddLocalRpcMethod(Methods.TextDocumentPublishDiagnosticsName, new Action<Newtonsoft.Json.Linq.JToken>(OnPublishDiagnostics));
                _rpc.AddLocalRpcMethod("window/workDoneProgress/create", new Func<Newtonsoft.Json.Linq.JToken, object>(t => new { }));
                _rpc.StartListening();

                // ⭐️ 3. 초기화 요청 파라미터 구성 (중첩 누락 방지를 위해 미니멀 구성으로 전달)
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

                string exeDir = AppDomain.CurrentDomain.BaseDirectory;
                string luaScriptsDir = System.IO.Path.Combine(exeDir, "Asset", "Engine", "EngineGenerate.lua");

                var result = await _rpc.InvokeAsync<Newtonsoft.Json.Linq.JObject>(Methods.InitializeName, initParams);
                if (result != null)
                {
                    // 서버에게 초기화 접수 수립 통보 완료 처리
                    await _rpc.NotifyAsync(Methods.InitializedName);

                    // ⭐️ [핵심 마스터 펀치]: 직렬화 교착 현상을 완벽히 파괴하기 위해, 
                    // JObject 구조를 사용하여 "Lua 5.1" 세팅과 "Asset/Engine" 폴더 스캔 명령을 강제로 주입합니다!
                    // 이 주입 알림이 꽂히는 순간 서버는 5.4 캐시를 완전히 버리고 파트너님의 컴포넌트 데이터들을 다시 로드합니다.
                    JObject configParams = new JObject();
                    JObject settings = new JObject();
                    JObject lua = new JObject();

                    lua["runtime"] = JObject.FromObject(new { version = "Lua 5.1" });
                    lua["workspace"] = JObject.FromObject(new { library = new string[] { finalLibraryUri } });
                    lua["diagnostics"] = JObject.FromObject(new { globals = new string[] { "Vector2", "Vector3", "Transform", "Rigidbody2D" } });

                    settings["Lua"] = lua;
                    configParams["settings"] = settings;

                    await _rpc.NotifyWithParameterObjectAsync(Methods.WorkspaceDidChangeConfigurationName, configParams);
                    ConsoleWindow.LogMessage("🎯 [LSP] 서버 강제 재인덱싱(Lua 5.1) 통보 완료!", 1);

                    // 4. 마스터 설계도 파일 오픈 처리
                    if (File.Exists(luaScriptsDir))
                    {
                        var content = File.ReadAllText(luaScriptsDir);
                        await NotifyFileOpenAsync("Asset/Engine/EngineGenerate.lua", content);
                    }
                    else
                    {
                        ConsoleWindow.LogMessage($"❌ [LSP] 초기화 마스터 파일을 찾을 수 없음: {luaScriptsDir}", 2);
                    }
                }
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"❌ [FATAL ERROR]: {ex.Message}\n{ex.StackTrace}", 2);
            }
        }

        public async Task NotifyFileOpenAsync(string fileUri, string fileText)
        {
            //상대 경로를 절때 경로로 변경
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            string absolutePath = System.IO.Path.GetFullPath(System.IO.Path.Combine(baseDir, fileUri));
            string uri = new Uri(absolutePath.Replace("\\", "/")).AbsoluteUri;

            targetFilePath = uri;
            var didOpenParams = new DidOpenTextDocumentParams
            {
                TextDocument = new TextDocumentItem
                {
                    Uri = new Uri(uri),                 // 예: "file:///C:/HyungSunEngine/Scripts/main.lua"
                    LanguageId = "lua",                 // ★ 서버가 무슨 언어인지 인식하게 하는 핵심 식별자!
                    Version = _documentVersion++,       // 문서 버전 (수정될 때마다 1씩 올림)
                    Text = fileText                     // 현재 AvalonEdit에 적혀있는 전체 코드 내용
                }
            };

            try
            {
                await _rpc.NotifyWithParameterObjectAsync(Methods.TextDocumentDidOpenName, didOpenParams);
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"파일 동기화 실패: {ex.Message}", 2);
            }
        }

        ///Text 업데이트 요청
        public async Task NotifyDidChangeAsync(string content, int currentLine, int currentColumn, string newText)
        {
            ++_documentVersion;
            ChangeMessageQueue.Enqueue(new LSPMessage
            {
                Type = LSPMessage.MessageType.CHANGE,
                version = _documentVersion,
                TriggerChar = newText,
            });

            var didChangeParams = new DidChangeTextDocumentParams
            {
                TextDocument = new VersionedTextDocumentIdentifier
                {
                    Uri = new Uri(targetFilePath),
                    Version = _documentVersion // 바뀔 때마다 버전업!
                },
                ContentChanges = new[]
                {
                    // Full Sync 모드: 전체 텍스트를 그냥 통째로 덮어씌움
                    new TextDocumentContentChangeEvent { Text = content }
                }
            };
            try
            {
                await _rpc.NotifyWithParameterObjectAsync(Methods.TextDocumentDidChangeName, didChangeParams);
                this.currentLine = currentLine;
                this.currentColumn = currentColumn;
                this.content = content;
            }
            catch (Exception ex)
            {
                await System.Windows.Application.Current.Dispatcher.BeginInvoke(new Action(() =>
                {
                    ConsoleWindow.LogMessage($"서버 텍스트 업데이트 실패 (Version: {ex})", 2);
                }));
            }
        }

        public class MyCompletionList
        {
            public CompletionItem[] Items { get; set; }
        }

        public async Task RequestCompletionAsync(string triggerChar, int currentLine, int currentColumn)
        {
            string uri = targetFilePath.Replace("\\", "/");

            var context = new CompletionContext();
            if (string.IsNullOrEmpty(triggerChar))
            {
                context.TriggerKind = CompletionTriggerKind.Invoked;
            }
            else
            {
                context.TriggerKind = CompletionTriggerKind.TriggerCharacter;
                context.TriggerCharacter = triggerChar;
            }

            var completionParams = new CompletionParams
            {
                TextDocument = new TextDocumentIdentifier { Uri = new Uri(uri) },
                Position = new Position
                {
                    Line = currentLine,
                    Character = currentColumn
                },
                Context = context
            };

            try
            {
                // 응답 스키마가 배열 노드 구조 형태로 얽혀 뒤섞이는 파싱 크래시 현상 원천 방지 가교 래핑
                var rawResult = await _rpc.InvokeWithParameterObjectAsync<JToken>(Methods.TextDocumentCompletionName, completionParams);
                CompletionItem[] completionItems = null;

                if (rawResult != null)
                {
                    if (rawResult is JArray array)
                    {
                        completionItems = array.ToObject<CompletionItem[]>();
                    }
                    else if (rawResult["items"] is JArray itemsArray)
                    {
                        completionItems = itemsArray.ToObject<CompletionItem[]>();
                    }
                }

                if (completionItems != null && completionItems.Length > 0)
                {
                    // 5. 화면에 띄우는 건 UI 작업이니까 Dispatcher.Invoke로 안전하게 메인 스레드에 맡김
                    System.Windows.Application.Current.Dispatcher.Invoke(() =>
                    {
                        var LuaEditor = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                        LuaEditor.completionWindow?.Close();

                        // 새로운 팝업창 생성 및 UI 수정
                        var bc = new BrushConverter();
                        LuaEditor.completionWindow = new CompletionWindow(LuaEditor.GetLuaEditorTextArea());
                        int caretOffset = LuaEditor.GetLuaEditorTextArea().Caret.Offset;
                        int startOffset = caretOffset;
                        while (startOffset > 0)
                        {
                            char ch = LuaEditor.GetLuaEditorTextArea().Document.GetCharAt(startOffset - 1);
                            if (!char.IsLetterOrDigit(ch) && ch != '_') break; // 변수명 규칙에 어긋나면 중단
                            startOffset--;
                        }
                        if (string.IsNullOrEmpty(triggerChar) && (caretOffset == startOffset))
                        {
                            return;
                        }
                        var completionSegment = new TextSegment
                        {
                            StartOffset = startOffset,
                            Length = caretOffset - startOffset
                        };
                        LuaEditor.completionWindow.StartOffset = startOffset;
                        LuaEditor.completionWindow.CloseAutomatically = true;
                        LuaEditor.completionWindow.AllowsTransparency = true;
                        LuaEditor.completionWindow.Background = new SolidColorBrush(System.Windows.Media.Color.FromRgb(27, 28, 46));
                        LuaEditor.completionWindow.MinWidth = 300;
                        var dataList = LuaEditor.completionWindow.CompletionList.CompletionData;

                        // 서버가 준 리스트를 우리가 만든 'LuaCompletionData' 포장지에 담아서 리스트에 쏙쏙 넣음
                        foreach (var item in completionItems)
                        {
                            if (item.Label.StartsWith("_"))
                                continue;

                            LuaEditor.completionWindow.CompletionList.CompletionData.Add(new LuaCompletionData(item));
                        }

                        LuaEditor.completionWindow.Show();
                        LuaEditor.completionWindow.Closed += (s, e) => LuaEditor.completionWindow = null;
                    });
                }
            }
            catch (Exception ex)
            {
                // 통신 에러가 나면 콘솔이나 디버그 창에 찍어보기
                System.Diagnostics.Debug.WriteLine($"자동완성 요청 에러: {ex.Message}");
            }
        }

        private void OnPublishDiagnostics(Newtonsoft.Json.Linq.JToken token)
        {
            int? version = token["version"]?.Value<int>();

            var @params = token.ToObject<PublishDiagnosticParams>();
            if (@params == null) return;

            // 1. 큐에 데이터가 있을 때까지만 반복 (안전장치)
            while (ChangeMessageQueue.Count > 0)
            {
                var m = ChangeMessageQueue.Peek();
                // 상황 A: 서버가 보낸 버전이 큐의 버전보다 크거나 같다 (처리 대상)
                if (m == null) return;
                if (version >= m.version)
                {
                    // 이제 필요 없으니 꺼냄
                    ChangeMessageQueue.Dequeue();
                    // 만약 딱 내가 기다리던 그 버전이라면?
                    if (version == m.version)
                    {
                        if (m.Type == LSPMessage.MessageType.CHANGE)
                        {
                            Task.Run(async () =>
                            {
                                await RequestCompletionAsync(m.TriggerChar, this.currentLine, this.currentColumn);
                            });
                        }
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        private void ShowCompletionWindow(CompletionItem[] items)
        {
        }
    }
}