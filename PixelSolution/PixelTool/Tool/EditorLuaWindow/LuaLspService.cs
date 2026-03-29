using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Editing;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using Newtonsoft.Json.Linq;
using StreamJsonRpc;
using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
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
                string apiFilePath = System.IO.Path.GetFullPath(System.IO.Path.Combine(baseDir, "LSP/bin"));

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
                    RedirectStandardError = true, // 🔥 서버의 비명을 가로채기 위해 필수
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                _luaServerProcess = new Process { StartInfo = info, EnableRaisingEvents = true };

                // 🔥 서버가 뱉는 '진짜 에러'를 실시간으로 엔진 로그창에 뿌림
                _luaServerProcess.ErrorDataReceived += (s, e) => {
                    if (!string.IsNullOrEmpty(e.Data))
                        ConsoleWindow.LogMessage($"⚠️ [SERVER-ERROR]: {e.Data}", 1);
                };

                // 서버가 죽었을 때 원인을 파악
                _luaServerProcess.Exited += (s, e) => {
                    ConsoleWindow.LogMessage($"💀 [PROCESS-EXIT]: 서버가 종료됨. ExitCode: {_luaServerProcess.ExitCode}", 2);
                };

                if (!_luaServerProcess.Start())
                {
                    ConsoleWindow.LogMessage("❌ 프로세스 시작 함수 자체가 실패함!", 2);
                    return;
                }

                _luaServerProcess.BeginErrorReadLine(); // 에러 읽기 시작

                // 2. RPC 설정 및 골든 시퀀스
                _rpc = new JsonRpc(_luaServerProcess.StandardInput.BaseStream, _luaServerProcess.StandardOutput.BaseStream);

                // 핸들러 등록
                _rpc.AddLocalRpcMethod(Methods.TextDocumentPublishDiagnosticsName, new Action<Newtonsoft.Json.Linq.JToken>(OnPublishDiagnostics));
                _rpc.AddLocalRpcMethod("window/workDoneProgress/create", new Func<Newtonsoft.Json.Linq.JToken, object>(t => new { }));

                // 리스닝 시작
                _rpc.StartListening();

                // 3. 초기화 요청 (Dictionary 방식으로 안전하게)
                var initParams = new Dictionary<string, object>
                {
                    ["processId"] = Environment.ProcessId,
                    ["rootUri"] = new Uri(System.IO.Path.Combine(baseDir, "Asset")).AbsoluteUri,
                    ["capabilities"] = new
                    {
                        window = new { workDoneProgress = false },
                        textDocument = new 
                        {
                            completion = new 
                            {
                                completionItem = new
                                {
                                    snippetSupport = true
                                }
                            }
                        }
                    },
                    ["initializationOptions"] = new
                    {
                        settings = new
                        {
                            Lua = new
                            {
                                workspace = new { library = new[] { new Uri(apiFilePath).AbsoluteUri } },
                                runtime = new { version = "Lua 5.1" }
                            }
                        }
                    }
                };

                var result = await _rpc.InvokeAsync<Newtonsoft.Json.Linq.JObject>(Methods.InitializeName, initParams);
                if (result != null)
                {
                    await _rpc.NotifyAsync(Methods.InitializedName);

                    var content = File.ReadAllText("./LSP/bin/GenerateLuaAPI.lua");
                    await NotifyFileOpenAsync("./LSP/bin/GenerateLuaAPI.lua", content);
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
        public async Task NotifyDidChangeAsync(string content,int currentLine,int currentColumn,string newText)
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

        public async Task RequestCompletionAsync(string triggerChar,int currentLine,int currentColumn)
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

                var result = await _rpc.InvokeWithParameterObjectAsync<MyCompletionList>(Methods.TextDocumentCompletionName, completionParams);
                var completionItems = result?.Items;

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
            //_completionWindow = new CompletionWindow(textEditor.TextArea);
            //var data = _completionWindow.CompletionList.CompletionData;
            //
            //foreach (var item in items)
            //{
            //    // 저번에 우리가 만든 MyCompletionData 클래스 기억나지? 거기에 서버가 준 데이터를 담아!
            //    data.Add(new MyCompletionData
            //    {
            //        Text = item.Label,
            //        Description = item.Detail ?? "설명 없음"
            //    });
            //}
            //
            //_completionWindow.Show();
            //_completionWindow.Closed += delegate { _completionWindow = null; };
        }
    }
}
