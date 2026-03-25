using ICSharpCode.AvalonEdit.CodeCompletion;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Windows.Media;
using System.Windows.Shapes;
using Path = System.IO.Path;

namespace PixelTool
{
    internal class LuaLspService
    {
        private Process _lspProcess;
        private StreamWriter _stdin;
        private int _fileVersion = 1;

        string filePath;
        string fileContent;

        public void StartServer()
        {
            // 1. 서버 경로 확인 (빌드 이벤트로 복사된 위치)
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            string lspExe = System.IO.Path.Combine(baseDir, "LSP", "bin", "lua-language-server.exe");
            string workingDir = System.IO.Path.GetDirectoryName(lspExe);

            if (!File.Exists(lspExe))
            {
                ConsoleWindow.LogMessage($"[LSP] LSP Server not found at{lspExe}", 2);
                return;
            }

            // 2. 프로세스 설정
            var startInfo = new ProcessStartInfo
            {
                FileName = lspExe,
                Arguments = "--stdio", // 표준 입출력 모드
                WorkingDirectory = workingDir, // 중요: 실행 파일 폴더로 작업 디렉토리 설정
                RedirectStandardInput = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true,
                StandardOutputEncoding = Encoding.UTF8
            };

            _lspProcess = new Process { StartInfo = startInfo };

            // 3. 로그 및 에러 출력 바인딩
            _lspProcess.ErrorDataReceived += (s, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data)) Debug.WriteLine($"[LSP Server Log]: {e.Data}");
            };

            if (_lspProcess.Start())
            {
                _lspProcess.BeginErrorReadLine();
                _stdin = _lspProcess.StandardInput;

                Debug.WriteLine(">>> Lua LSP Server Started Successfully.");

                // 4. 별도 태스크에서 출력 읽기 시작
                Task.Run(() => ListenToServer(_lspProcess.StandardOutput));
            }

            _lspProcess.Exited += _lspProcess_Exited;
        }

        private void _lspProcess_Exited(object? sender, EventArgs e)
        {
            ConsoleWindow.LogMessage("[LSP] 서버가 중단되었습니다. 3초 후 재시작합니다...", 2);
            StopServer();
            // 너무 빨리 재시작하면 무한 루프에 빠질 수 있으니 지연 시간을 둡니다.
            Task.Delay(3000).ContinueWith(t =>
            {
                App.Current.Dispatcher.Invoke(() =>
                {
                    StartServer();
                    SendDidOpenNotification(filePath, fileContent);
                });
            });
        }

        private async Task ListenToServer(StreamReader reader)
        {
            try
            {
                while (_lspProcess != null && !_lspProcess.HasExited)
                {
                    // 1. 헤더 읽기
                    string line = await reader.ReadLineAsync();
                    if (string.IsNullOrEmpty(line)) continue;

                    if (line.StartsWith("Content-Length:"))
                    {
                        int contentLength = int.Parse(line.Replace("Content-Length:", "").Trim());

                        // 2. 헤더 뒤의 빈 줄(\r\n) 버리기
                        await reader.ReadLineAsync();

                        // ==============================================================
                        // [핵심] 글자 수(Length)가 아니라 바이트 수(ByteCount)로 읽어야 합니다!
                        // 다음 메시지의 'C'를 파먹지 않도록 한 글자씩 안전하게 읽습니다.
                        // ==============================================================
                        var sb = new StringBuilder();
                        int currentBytes = 0;
                        char[] charBuf = new char[1];

                        while (currentBytes < contentLength)
                        {
                            int read = await reader.ReadAsync(charBuf, 0, 1);
                            if (read == 0) break; // 스트림 끊김 방어

                            char c = charBuf[0];
                            sb.Append(c);

                            // 방금 읽은 1글자가 실제 UTF-8로 몇 바이트였는지 계산해서 더함 (한글=3, 영어=1)
                            currentBytes += Encoding.UTF8.GetByteCount(charBuf);
                        }

                        string jsonBody = sb.ToString();

                        // [로그 확인] 이제 뒤에 찌꺼기 'C'가 안 붙어 나오는 걸 볼 수 있습니다.
                        System.Diagnostics.Debug.WriteLine($"[LSP Received]: {jsonBody}");

                        // 4. JSON 분석 (이제 예외 안 터짐!)
                        HandleMessage(jsonBody);
                    }
                }
            }
            catch (Exception ex)
            {
                // 죽더라도 앱이 꺼지진 않게 방어
                System.Diagnostics.Debug.WriteLine($"!!! [LSP Listen Error]: {ex.Message}");
            }
        }

        public async Task SendInitializeRequest()
        {
            if (_stdin == null) return;

            // 1. 경로 세팅
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            string metaFilePath = Path.Combine(baseDir, "LSP", "bin", "GenerateLuaAPI.lua");

            string rootUri = new Uri(baseDir).AbsoluteUri;
            string metaFileUri = new Uri(metaFilePath).AbsoluteUri;
            string metaFolderUri = new Uri(Path.Combine(baseDir, "LSP", "bin", "GenerateLuaAPI.lua")).AbsoluteUri;
            string apiContent = File.ReadAllText(metaFilePath);

            // 2. 초기화(Initialize) 요청 구성
            var initializeRequest = new
            {
                jsonrpc = "2.0",
                id = 1,
                method = "initialize",
                @params = new
                {
                    processId = Process.GetCurrentProcess().Id,
                    rootUri = rootUri,
                    capabilities = new { /* 네 기존 capabilities */ },
                    initializationOptions = GetLspInitOptions() // 👈 깔-끔
                }
            };

            // [정리 1] SendMessage를 활용하여 초기화 전송 코드를 1줄로 단축!
            await SendMessage(initializeRequest);
            Debug.WriteLine(">>> 1. Initialize Request Sent.");

            // =====================================================================
            // 3. 메타 파일 강제 로드 (Force Load)
            // 서버가 폴더 스캔을 씹는 현상을 완벽하게 방어하는 필살기입니다.
            // =====================================================================

            var configRequest = new
            {
                jsonrpc = "2.0",
                method = "workspace/didChangeConfiguration",
                @params = new
                {
                    settings = new Dictionary<string, object>
                    {
                        ["Lua"] = new Dictionary<string, object>
                        {
                            ["runtime"] = new Dictionary<string, object>
                            {
                                ["version"] = "Lua 5.1",
                                ["builtin"] = new Dictionary<string, string>
                                {
                                    ["basic"] = "disable",
                                    ["string"] = "disable",
                                    ["table"] = "disable",
                                    ["math"] = "disable",
                                    ["coroutine"] = "disable",
                                    ["debug"] = "disable",
                                    ["os"] = "disable",
                                    ["io"] = "disable"
                                }
                            },
                            ["completion"] = new Dictionary<string, object>
                            {
                                ["callSnippet"] = "Both",
                                ["displayContext"] = 1,
                                ["workspaceWord"] = false,
                                ["showWord"] = "Disable"
                            }
                        }
                    }
                }
            };
            await SendMessage(configRequest);
            if (File.Exists(metaFilePath))
            {
                var forceOpenRequest = new
                {
                    jsonrpc = "2.0",
                    method = "textDocument/didOpen",
                    @params = new
                    {
                        textDocument = new
                        {
                            uri = metaFileUri,
                            languageId = "lua",
                            version = 1,
                            text = File.ReadAllText(metaFilePath)
                        }
                    }
                };
            
                // [정리 2] 이 역시 SendMessage로 깔끔하게 전송!
                await SendMessage(forceOpenRequest);
            }
            else
            {
                ConsoleWindow.LogMessage($"[LSP] Error File not found at{metaFilePath}", 2);
            }
        }

        private async Task SendMessage(object payload)
        {
            if (_stdin == null) return;

            try
            {
                if (_lspProcess == null || _lspProcess.HasExited || _stdin == null)
                {
                    ConsoleWindow.LogMessage($"[LSP] LSP 서버가 실행 중이 아닙니다.", 2);
                    return;
                }
                string jsonPayload = JsonConvert.SerializeObject(payload);

                // [핵심] string의 길이가 아니라, UTF8로 변환했을 때의 '진짜 바이트 수'를 구합니다.
                byte[] utf8Bytes = Encoding.UTF8.GetBytes(jsonPayload);
                int realByteLength = utf8Bytes.Length;

                // 헤더 구성 (realByteLength를 사용)
                string header = $"Content-Length: {realByteLength}\r\n\r\n";

                // 전송 (헤더와 본문을 합쳐서 쏘거나 순서대로 쏩니다)
                byte[] headerBytes = Encoding.ASCII.GetBytes(header);

                await _stdin.BaseStream.WriteAsync(headerBytes, 0, headerBytes.Length);
                await _stdin.BaseStream.WriteAsync(utf8Bytes, 0, utf8Bytes.Length);
                await _stdin.BaseStream.FlushAsync();
            }
            catch (IOException ex)
            {
                ConsoleWindow.LogMessage($"[LSP] Error{ex.Message}", 2);
            }
        }

        // 2. 메시지 수신 핸들러 (서버의 말에 반응함)
        private void HandleMessage(string json)
        {
            // 1. JSON 분석은 백그라운드 스레드에서 (UI와 상관없음)
            var response = JsonConvert.DeserializeObject<dynamic>(json);
            if (response == null) return;

            // 2. UI와 관련된 모든 작업은 Dispatcher 내부로 완전히 몰아넣습니다.
            App.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                try
                {
                    // 초기화 확인 응답
                    if (response.id == 1)
                    {
                        Task.Run(async () =>
                        {
                            await SendMessage(new { jsonrpc = "2.0", method = "initialized", @params = new { } });
                        });
                    }

                    // 자동 완성 처리 (CompletionWindow 생성 및 표시)
                    if (response.id == 100)
                    {
                        var items = response.result?.items;
                        if (items.Count <= 0) return;

                        var editorWindow = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                        if (editorWindow.completionWindow != null) return;

                        editorWindow.completionWindow = new CompletionWindow(editorWindow.GetLuaEditorTextArea());
                        WindowUISetting(editorWindow.completionWindow);
                        editorWindow.completionWindow.Closed += (s, ev) =>
                        {
                            editorWindow.completionWindow = null;
                        };
                        var offset = editorWindow.GetLuaEditorTextArea().Caret.Offset;
                        char lastChar = editorWindow.GetLuaEditorTextArea().Document.GetCharAt(offset - 1);
                        if (lastChar != '.' && lastChar != ':')
                        {
                            editorWindow.completionWindow.StartOffset -= 1;
                        }


                        IList<ICompletionData> data = editorWindow.completionWindow.CompletionList.CompletionData;
                        foreach (var item in items)
                        {
                            int kind = (int)(item.kind ?? 0);
                            if (kind == 1 || kind == 14 || kind == 15) continue;
                            string label = (string)item.label;
                            string detail = (string)(item.detail ?? "");
                            data.Add(new LuaCompletionData(label, detail));
                        }
                        editorWindow.completionWindow.Show();
                        editorWindow.completionWindow.Width = 300;
                    }

                    // 실시간 진단 로그
                    if (response.method == "textDocument/publishDiagnostics")
                    {
                        //ConsoleWindow.LogMessage($"[LSP] 문법 검사 결과 도착", 0);
                    }
                }
                catch (Exception ex)
                {
                    // 여기서 터지는건 UI 로직 문제임
                    ConsoleWindow.LogMessage($"[LSP] UI Thread Error {ex.Message}", 0);
                }
            }));
        }

        public async Task SendDidOpenNotification(string filePath, string fileContent)
        {
            if (_stdin == null) return;

            this.filePath = filePath;
            this.fileContent = fileContent;
            string fileUri = new Uri(Path.GetFullPath(filePath), UriKind.Absolute).AbsoluteUri;

            var notification = new
            {
                jsonrpc = "2.0",
                method = "textDocument/didOpen", // [중요] 반드시 didOpen이 먼저 가야 함!
                @params = new
                {
                    textDocument = new
                    {
                        uri = fileUri,
                        languageId = "lua", // 서버에게 루아 파일임을 명시
                        version = 1,
                        text = fileContent // 현재 에디터의 전체 내용
                    }
                }
            };

            await SendMessage(notification);
            ConsoleWindow.LogMessage($"[LSP] Lua 파일 Open {fileUri}", 0);
        }


        // 3. 파일 열기 알림 (서버가 분석을 시작하는 시점)
        public async Task SendDidChangeNotification(string filePath, string fileContent)
        {
            if (_stdin == null) return;

            string fileUri = new Uri(Path.GetFullPath(filePath), UriKind.Absolute).AbsoluteUri;

            var notification = new
            {
                jsonrpc = "2.0",
                method = "textDocument/didChange",
                @params = new
                {
                    textDocument = new
                    {
                        uri = fileUri,
                        version = ++_fileVersion // 버전업!
                    },
                    // 전체 텍스트 동기화 방식 (간단함)
                    contentChanges = new[]
                    {
                        new { text = fileContent }
                    }
                }
            };

            await SendMessage(notification);
        }

        public async Task SendCompletionRequest(string filePath, int line, int character)
        {
            if (_stdin == null) return;

            string fileUri = new Uri(Path.GetFullPath(filePath), UriKind.Absolute).AbsoluteUri;

            var request = new
            {
                jsonrpc = "2.0",
                id = 100, // 응답을 매칭하기 위한 ID (임의 지정)
                method = "textDocument/completion",
                @params = new
                {
                    textDocument = new { uri = fileUri },
                    position = new { line = line - 1, character = character } // 0부터 시작함에 주의!
                }
            };

            await SendMessage(request);
        }

        public void StopServer()
        {
            _lspProcess?.Kill();
            _lspProcess?.Dispose();
        }


        void WindowUISetting(CompletionWindow window)
        {
            window.CompletionList.ListBox.Background = new SolidColorBrush(Color.FromRgb(30, 30, 30));
            window.CompletionList.ListBox.Foreground = Brushes.LightGray;
            window.CompletionList.ListBox.BorderBrush = new SolidColorBrush(Color.FromRgb(63, 63, 70));
            window.CompletionList.ListBox.BorderThickness = new System.Windows.Thickness(1);
            window.CompletionList.ListBox.FontFamily = new FontFamily("Consolas");
            window.CompletionList.ListBox.FontSize = 13;
        }
        public object GetLspInitOptions()
        {
            // 1. 루아 런타임 (내장 라이브러리 비활성화)
            var runtime = new Dictionary<string, object>
            {
                ["version"] = "Lua 5.1",
                ["builtin"] = new Dictionary<string, string>
                {
                    ["basic"] = "disable",
                    ["string"] = "disable",
                    ["table"] = "disable",
                    ["math"] = "disable",
                    ["coroutine"] = "disable",
                    ["debug"] = "disable",
                    ["os"] = "disable",
                    ["io"] = "disable"
                }
            };

            // 2. 진단 및 자동완성
            var diagnostics = new Dictionary<string, object>
            {
                ["globals"] = new[] { "Pixel", "Time", "Input", "Engine", "KeyCode", "PVector3", "Asset", "Scene", "GameObject", "Transform", "Renderer2D", "LuaScript" }
            };

            var completion = new Dictionary<string, object>
            {
                ["callSnippet"] = "Both",
                ["displayContext"] = 1
            };

            // 3. 파일 확장자 연결
            var files = new Dictionary<string, object>
            {
                ["associations"] = new Dictionary<string, string>
                {
                    ["*.pxm"] = "lua",
                    ["*.scene"] = "lua"
                }
            };

            // 4. 최종 조립 리턴
            return new
            {
                settings = new Dictionary<string, object>
                {
                    ["Lua"] = new Dictionary<string, object>
                    {
                        ["runtime"] = runtime,
                        ["diagnostics"] = diagnostics,
                        ["completion"] = completion
                    },
                    ["files"] = files
                }
            };
        }
    }

}
