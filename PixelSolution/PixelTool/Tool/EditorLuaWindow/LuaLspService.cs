using ICSharpCode.AvalonEdit.CodeCompletion;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Windows.Shapes;
using Path = System.IO.Path;

namespace PixelTool
{
    internal class LuaLspService
    {
        private Process _lspProcess;
        private StreamWriter _stdin;
        private int _fileVersion = 1;

        public void StartServer()
        {
            // 1. 서버 경로 확인 (빌드 이벤트로 복사된 위치)
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            string lspExe = System.IO.Path.Combine(baseDir, "LSP","bin","lua-language-server.exe");
            string workingDir = System.IO.Path.GetDirectoryName(lspExe);

            if (!File.Exists(lspExe))
            {
                Debug.WriteLine($"[Error] LSP Server not found at: {lspExe}");
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
            string metaFolderUri = new Uri(Path.Combine(baseDir, "LSP", "bin")).AbsoluteUri;

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
                    capabilities = new
                    {
                        textDocument = new { completion = new { completionItem = new { snippetSupport = true } } }
                    },
                    initializationOptions = new
                    {
                        settings = new
                        {
                            Lua = new
                            {
                                workspace = new
                                {
                                    // [수정] 파일 경로가 아닌 '폴더 URI'로 교체했습니다.
                                    library = new[] { metaFolderUri },
                                    checkThirdParty = false,
                                    ignoreDir = new[] { ".git" } // bin 폴더 무시 방지
                                },
                                diagnostics = new
                                {
                                    globals = new[] { "Pixel", "Time", "Input", "Engine", "KeyCode", "PVector3", "Asset", "Scene", "GameObject", "Renderer2D" }
                                },
                                completion = new
                                {
                                    displayContext = 1,
                                    callSnippet = "Both"
                                }
                            }
                        }
                    }
                }
            };

            // [정리 1] SendMessage를 활용하여 초기화 전송 코드를 1줄로 단축!
            await SendMessage(initializeRequest);
            Debug.WriteLine(">>> 1. Initialize Request Sent.");

            // =====================================================================
            // 3. 메타 파일 강제 로드 (Force Load)
            // 서버가 폴더 스캔을 씹는 현상을 완벽하게 방어하는 필살기입니다.
            // =====================================================================
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
                Debug.WriteLine(">>> 2. Meta File Force Loaded (didOpen).");
            }
            else
            {
                Debug.WriteLine($"!!! Meta Load Error: File not found at {metaFilePath}");
            }
        }

        private async Task SendMessage(object payload)
        {
            if (_stdin == null) return;

            try
            {
                if (_lspProcess == null || _lspProcess.HasExited || _stdin == null)
                {
                    Debug.WriteLine("!!! [Error] LSP 서버가 실행 중이 아닙니다.");
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
                Debug.WriteLine($"!!! [LSP Write Error]: {ex.Message}");
            }
        }

        // 2. 메시지 수신 핸들러 (서버의 말에 반응함)
        private void HandleMessage(string json)
        {
            var response = JsonConvert.DeserializeObject<dynamic>(json);

            // [순서 1] Initialize에 대한 응답이 왔는가?
            if (response.id == 1)
            {
                Debug.WriteLine(">>> [Step 2] 서버 응답 확인. Initialized 확답 보냄.");
                Task.Run(async () => {
                    // [순서 2] Initialized 알림 전송 (규격상 필수)
                    await SendMessage(new { jsonrpc = "2.0", method = "initialized", @params = new { } });
                });
            }

            if(response.id == 100)
            {
                var items = response.result.items;
                if (items == null) return;

                // UI 스레드에서 팝업 띄우기
                App.Current.Dispatcher.Invoke(() => 
                {
                    var k = GlobalFunction.GetDockedWindow<LuaEditorWindow>();
                    if (k == null) return;

                    var window = new CompletionWindow(k.GetLuaEditorTextArea());
                    IList<ICompletionData> data = window.CompletionList.CompletionData;
                    foreach (var item in items)
                    {
                        int kind = (int)item.kind;
                        if (kind == 1 || kind == 14 || kind == 15) continue;
                        // label: 함수명, detail: 반환타입이나 매개변수 정보
                        //data.Add(new LuaCompletionData((string)item.label, (string)item.detail));
                        string label = (string)item.label;
                        data.Add(new LuaCompletionData(label, (string)item.detail ?? ""));
                    }

                    if (data.Count > 0)
                    {
                        window.Show();
                        window.Width = 300;
                        window.Closed += (o, e) => window = null;
                    }
                });
            }

            // [참고] 서버가 보내는 실시간 에러(Diagnostics)는 id가 없음
            if (response.method == "textDocument/publishDiagnostics")
            {
                Debug.WriteLine($">>> [Step 4] 문법 검사 결과 도착: {json}");
            }
        }

        public async Task SendDidOpenNotification(string filePath, string fileContent)
        {
            if (_stdin == null) return;

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
            Debug.WriteLine($">>> [didOpen] 서버에 파일 등록 완료: {fileUri}");
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
                    contentChanges = new[] {
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
    }
}
