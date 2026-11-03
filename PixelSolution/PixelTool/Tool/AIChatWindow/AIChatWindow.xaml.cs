using System.Diagnostics;
using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PixelTool;

public partial class AIChatWindow : UserControl
{
    private readonly string _project = ProjectPathService.ProjectRootPath;
    private CodexChatHistory _history;
    private ChatGptChatClient? _client;
    private string? _threadId;
    private string? _turnId;
    private bool _busy;
    private bool _disposed;
    private bool _historyWritable = true;
    private bool _stopRequested;
    private bool _loadingAccounts;
    private bool _restoringPreferences = true;
    private bool _restoreAttempted;
    private bool _scrollToEndQueued;
    private sealed record ChatAccount(string Id, string Label);
    private sealed record ChatModel(string Model, string DisplayName, bool IsDefault, string? DefaultEffort);
    private readonly Dictionary<string, string> _fileChanges = new();

    public AIChatWindow()
    {
        InitializeComponent();
        try { _history = CodexChatHistory.Load(_project); }
        catch (Exception)
        {
            string source = CodexChatHistory.PathFor(_project);
            try
            {
                if (File.Exists(source)) File.Copy(source, source + ".unreadable-" + Guid.NewGuid().ToString("N"));
            }
            catch (Exception) { _historyWritable = false; }
            _history = new() { ProjectPath = _project };
            StatusText.Text = "기존 대화 기록을 읽지 못했습니다. 연결 시 새 대화로 시작합니다.";
        }
        CommandApprovalBox.SelectedIndex = _history.ConfirmCommands ? 0 : 1;
        AllowEdits.IsChecked = _history.AllowEdits;
        _restoringPreferences = false;
        RenderTranscript();
        Loaded += RestoreConnection;
        Loaded += (_, _) => ScrollTranscriptToEnd();
        IsVisibleChanged += (_, e) => { if (e.NewValue is true) ScrollTranscriptToEnd(); };
    }

    private async void Connect(object sender, RoutedEventArgs e)
        => await ConnectAsync(allowBrowserLogin: true);

    private async void RestoreConnection(object sender, RoutedEventArgs e)
    {
        if (_restoreAttempted || _disposed || _client != null) return;
        _restoreAttempted = true;
        if (ChatGptSignIn.HasSavedAccount) await ConnectAsync(allowBrowserLogin: false);
    }

    private void ToggleAccountDetails(object sender, RoutedEventArgs e)
    {
        AccountDetails.Visibility = AccountDetails.Visibility == Visibility.Visible ? Visibility.Collapsed : Visibility.Visible;
    }

    private void ToggleChatSettings(object sender, RoutedEventArgs e)
    {
        SettingsPanel.Visibility = SettingsPanel.Visibility == Visibility.Visible ? Visibility.Collapsed : Visibility.Visible;
    }

    private void AllowEditsChanged(object sender, RoutedEventArgs e)
    {
        if (_restoringPreferences || _history == null) return;
        _history.AllowEdits = AllowEdits.IsChecked == true;
        SaveHistory();
    }

    private async Task ConnectAsync(bool allowBrowserLogin)
    {
        ConnectButton.IsEnabled = false;
        ModelBox.IsEnabled = SendButton.IsEnabled = NewButton.IsEnabled = false;
        try
        {
            if (_client != null) await _client.DisposeAsync();
            _threadId = _turnId = null;
            _client = new ChatGptChatClient();
            if (_history.ThreadId != null && !_history.ThreadId.StartsWith("chatgpt-"))
            {
                _history.ThreadId = null;
                _history.Messages.Add(new() { Role = "시스템", Text = "ChatGPT 직접 연결로 변경했습니다. 이전 Codex 대화 기록은 보존되지만 새 연결의 AI 문맥에는 전송되지 않습니다." });
                SaveHistory();
                RenderTranscript();
            }
            _client.Notification += (method, p) => Dispatcher.BeginInvoke(new Action(() => HandleNotification(method, p)));
            _client.Disconnected += () => Dispatcher.BeginInvoke(new Action(() =>
            {
                if (_disposed) return;
                SetBusy(false);
                SendButton.IsEnabled = RefreshButton.IsEnabled = NewButton.IsEnabled = false;
                ModelBox.IsEnabled = false;
                ConnectButton.IsEnabled = true;
                StatusText.Text = "ChatGPT 연결이 종료되었습니다. 계정 연결을 다시 눌러주세요.";
            }));
            _client.Approve = (method, p) => Dispatcher.InvokeAsync(() => ConfirmApproval(method, p)).Task;
            _client.PickAsset = () => Dispatcher.InvokeAsync(PickImportAsset).Task;
            _client.ReadEditorLogs = ConsoleWindow.ReadLogsForAi;
            _client.ReimportProject = token => Dispatcher.InvokeAsync(() =>
                ((MainWindow)Application.Current.MainWindow).ReimportForAiAsync(token)).Task.Unwrap();
            StatusText.Text = "ChatGPT 연결 중…";
            await _client.ConnectAsync(_project);
            AddAccountButton.IsEnabled = ReauthorizeButton.IsEnabled = true;
            RefreshButton.IsEnabled = true;
            if (!await LoadAccountAsync())
            {
                if (!allowBrowserLogin)
                {
                    AccountDetails.Visibility = Visibility.Visible;
                    SettingsPanel.Visibility = Visibility.Visible;
                    AccountSummary.Text = "계정 재연결 필요";
                    ConnectButton.IsEnabled = true;
                    StatusText.Text = "저장된 연결을 복원하지 못했습니다. 계정 연결을 눌러주세요.";
                    return;
                }
                var result = await _client.RequestAsync("account/login/start", new { type = "chatgpt" });
                string url = result.GetProperty("authUrl").GetString()!;
                if (!Uri.TryCreate(url, UriKind.Absolute, out var uri) || uri.Scheme != Uri.UriSchemeHttps || uri.Host != "auth.openai.com")
                    throw new InvalidDataException("ChatGPT returned an invalid login URL.");
                Process.Start(new ProcessStartInfo(uri.AbsoluteUri) { UseShellExecute = true });
                StatusText.Text = "브라우저에서 ChatGPT 로그인을 완료하세요. 자동 연결되지 않으면 로그인 확인을 누르세요.";
            }
        }
        catch (Exception exception)
        {
            AccountDetails.Visibility = Visibility.Visible;
            SettingsPanel.Visibility = Visibility.Visible;
            AccountSummary.Text = "계정 연결 확인 필요";
            StatusText.Text = "연결 실패: " + exception.Message;
            ConnectButton.IsEnabled = true;
        }
    }

    private async void AddAccount(object sender, RoutedEventArgs e)
        => await OpenLoginAsync(true);

    private async void Reauthorize(object sender, RoutedEventArgs e)
        => await OpenLoginAsync(false);

    private async Task OpenLoginAsync(bool newAccount)
    {
        if (_busy || _client == null) return;
        try
        {
            var result = await _client.RequestAsync("account/login/start", new { type = "chatgpt", newAccount });
            var uri = new Uri(result.GetProperty("authUrl").GetString()!);
            if (uri.Scheme != "https" || uri.Host != "auth.openai.com") throw new InvalidDataException("잘못된 로그인 주소입니다.");
            Process.Start(new ProcessStartInfo(uri.AbsoluteUri) { UseShellExecute = true });
            _threadId = _history.ThreadId = null;
            SendButton.IsEnabled = NewButton.IsEnabled = ModelBox.IsEnabled = false;
            StatusText.Text = "브라우저에서 새 ChatGPT 계정을 연결하세요. 기존 대화 기록은 유지됩니다.";
        }
        catch (Exception ex) { StatusText.Text = "계정 추가 실패: " + ex.Message; }
    }

    private async Task<bool> LoadAccountAsync()
    {
        if (_client == null) return false;
        var result = await _client.RequestAsync("account/read", new { refreshToken = false });
        if (!result.TryGetProperty("account", out var account) || account.ValueKind == JsonValueKind.Null) return false;
        if (account.GetProperty("type").GetString() != "chatgpt")
        {
            StatusText.Text = "ChatGPT 계정 로그인이 필요합니다. API 키로는 대화를 실행하지 않습니다.";
            return false;
        }
        var accounts = await _client.RequestAsync("account/list", new { });
        _loadingAccounts = true;
        try
        {
            var options = accounts.GetProperty("data").EnumerateArray().Select(a => new ChatAccount(a.GetProperty("id").GetString()!, a.GetProperty("label").GetString()!)).ToArray();
            AccountBox.ItemsSource = options;
            AccountBox.SelectedItem = options.FirstOrDefault(a => a.Id == accounts.GetProperty("selected").GetString());
            AccountBox.IsEnabled = true;
        }
        finally { _loadingAccounts = false; }
        await LoadModelsAsync();
        await PrepareThreadAsync();
        string plan = account.TryGetProperty("planType", out var p) ? p.GetString() ?? "" : "";
        AccountSummary.Text = "연결됨 · " + plan;
        AccountDetails.Visibility = Visibility.Collapsed;
        SettingsPanel.Visibility = Visibility.Collapsed;
        StatusText.Text = "ChatGPT 계정 연결됨 · " + plan + " · " + _project;
        SendButton.IsEnabled = NewButton.IsEnabled = true;
        ConnectButton.IsEnabled = false;
        SaveHistory();
        return true;
    }

    private async void AccountChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingAccounts || _busy || _client == null || AccountBox.SelectedItem is not ChatAccount selected) return;
        SendButton.IsEnabled = NewButton.IsEnabled = ModelBox.IsEnabled = AccountBox.IsEnabled = false;
        try
        {
            await _client.RequestAsync("account/select", new { id = selected.Id });
            _threadId = _history.ThreadId = null;
            SaveHistory();
            await LoadAccountAsync();
        }
        catch (Exception ex) { StatusText.Text = "계정 변경 실패: " + ex.Message; AccountBox.IsEnabled = true; }
    }

    private async Task LoadModelsAsync()
    {
        ModelBox.IsEnabled = SendButton.IsEnabled = NewButton.IsEnabled = false;
        var models = new List<ChatModel>();
        var cursors = new HashSet<string>();
        string? cursor = null;
        do
        {
            var result = await _client!.RequestAsync("model/list", new { limit = 100, cursor, includeHidden = false });
            foreach (var entry in result.GetProperty("data").EnumerateArray())
            {
                string? model = entry.GetProperty("model").GetString();
                if (string.IsNullOrWhiteSpace(model) || models.Any(m => m.Model == model)) continue;
                models.Add(new(model,
                    entry.TryGetProperty("displayName", out var name) ? name.GetString() ?? model : model,
                    entry.TryGetProperty("isDefault", out var recommended) && recommended.GetBoolean(),
                    entry.TryGetProperty("defaultReasoningEffort", out var effort) ? effort.GetString() : null));
            }
            cursor = result.TryGetProperty("nextCursor", out var next) ? next.GetString() : null;
            if (cursor != null && !cursors.Add(cursor)) throw new InvalidDataException("모델 목록의 다음 페이지가 반복됩니다.");
        } while (cursor != null);
        if (models.Count == 0) throw new InvalidOperationException("이 ChatGPT 계정에서 선택 가능한 모델이 없습니다. 로그인과 사용 권한을 확인하세요.");
        string? savedModel = _history.Model;
        ModelBox.ItemsSource = models;
        ModelBox.SelectedItem = models.FirstOrDefault(m => m.Model == savedModel)
            ?? models.FirstOrDefault(m => m.IsDefault) ?? models[0];
        ModelBox.IsEnabled = true;
    }

    private void ModelChanged(object sender, SelectionChangedEventArgs e)
    {
        if (ModelBox.SelectedItem is not ChatModel selected) return;
        _history.Model = selected.Model;
        SaveHistory();
    }

    private void CommandApprovalChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_restoringPreferences || _history == null || CommandApprovalBox.SelectedIndex < 0) return;
        _history.ConfirmCommands = CommandApprovalBox.SelectedIndex == 0;
        SaveHistory();
    }

    private async Task PrepareThreadAsync()
    {
        if (_threadId != null || _client == null) return;
        JsonElement result;
        if (!string.IsNullOrEmpty(_history.ThreadId))
        {
            // A failed resume never silently discards the old conversation.
            result = await _client.RequestAsync("thread/resume", new
            {
                threadId = _history.ThreadId, cwd = _project, model = _history.Model,
                developerInstructions = EngineGuide.Load(), approvalPolicy = "on-request", sandbox = "read-only"
            });
        }
        else
        {
            result = await _client.RequestAsync("thread/start", new
            {
                cwd = _project, model = _history.Model, developerInstructions = EngineGuide.Load(),
                approvalPolicy = "on-request", sandbox = "read-only"
            });
        }
        var thread = result.GetProperty("thread");
        if (thread.TryGetProperty("cwd", out var cwd) && !string.Equals(Path.GetFullPath(cwd.GetString()!), Path.GetFullPath(_project), StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("ChatGPT thread belongs to a different project.");
        _history.ThreadId = _threadId = thread.GetProperty("id").GetString();
    }

    private async void RefreshAccount(object sender, RoutedEventArgs e)
    {
        if (_busy) return;
        try { if (!await LoadAccountAsync()) StatusText.Text = "ChatGPT 로그인 대기 중입니다."; }
        catch (Exception exception) { StatusText.Text = "계정 확인 실패: " + exception.Message; }
    }

    private async void Send(object sender, RoutedEventArgs e)
    {
        if (_busy || _client == null || _threadId == null || ModelBox.SelectedItem is not ChatModel selected || string.IsNullOrWhiteSpace(PromptBox.Text)) return;
        bool confirmCommands = _history.ConfirmCommands;
        string text = PromptBox.Text;
        PromptBox.Clear();
        _history.Messages.Add(new() { Role = "사용자", Text = text });
        SaveHistory();
        RenderTranscript();
        SetBusy(true);
        try
        {
            var auth = await _client.RequestAsync("account/read", new { refreshToken = false });
            if (!auth.TryGetProperty("account", out var account) || account.ValueKind == JsonValueKind.Null ||
                account.GetProperty("type").GetString() != "chatgpt")
                throw new InvalidOperationException("ChatGPT 계정 연결을 다시 확인하세요. API 키로는 요청하지 않습니다.");
            if (_stopRequested)
            {
                SetBusy(false);
                StatusText.Text = "전송 취소됨";
                return;
            }
            var result = await _client.RequestAsync("turn/start", new
            {
                threadId = _threadId,
                model = selected.Model,
                effort = selected.DefaultEffort,
                confirmActions = confirmCommands,
                approvalPolicy = "on-request",
                sandboxPolicy = AllowEdits.IsChecked == true
                    ? (object)new { type = "workspaceWrite", writableRoots = new[] { _project }, networkAccess = false }
                    : new { type = "readOnly" },
                input = new[]
                {
                    new { type = "text", text = CommandExecutionPreference.ForTurn(confirmCommands) },
                    new { type = "text", text }
                }
            });
            if (_busy)
            {
                _turnId = result.GetProperty("turn").GetProperty("id").GetString();
                if (_stopRequested) await _client.RequestAsync("turn/interrupt", new { threadId = _threadId, turnId = _turnId });
            }
        }
        catch (Exception exception)
        {
            await _client.DisposeAsync();
            _client = null;
            _threadId = null;
            ConnectButton.IsEnabled = true;
            _history.Messages.Add(new() { Role = "시스템", Text = "요청 실패: " + exception.Message });
            SetBusy(false);
            SaveHistory();
            RenderTranscript();
        }
    }

    private void HandleNotification(string method, JsonElement p)
    {
        if (_disposed || p.ValueKind != JsonValueKind.Object) return;
        try { ApplyNotification(method, p); }
        catch (Exception exception) when (exception is JsonException or InvalidOperationException or KeyNotFoundException)
        {
            SendButton.IsEnabled = NewButton.IsEnabled = false;
            ConnectButton.IsEnabled = true;
            StatusText.Text = "ChatGPT 응답 형식을 처리하지 못했습니다. 응답을 중지하고 계정 연결을 다시 눌러주세요.";
        }
    }

    private void ApplyNotification(string method, JsonElement p)
    {
        if (_disposed) return;
        if (method == "account/login/completed")
        {
            if (p.TryGetProperty("success", out var success) && success.GetBoolean()) RefreshAccount(this, new RoutedEventArgs());
            else
            {
                StatusText.Text = "로그인이 완료되지 않았습니다. 계정 연결을 다시 시도하세요.";
                ConnectButton.IsEnabled = true;
            }
            return;
        }
        if (p.TryGetProperty("threadId", out var thread) && thread.GetString() != _threadId) return;
        if (method == "turn/started" && p.TryGetProperty("turn", out var started))
            _turnId = started.GetProperty("id").GetString();
        if (method is "item/started" or "item/completed")
        {
            var item = p.GetProperty("item");
            string? type = item.GetProperty("type").GetString();
            string id = ItemKey(p, item.GetProperty("id").GetString()!);
            if (type == "fileChange")
            {
                _fileChanges[id] = item.GetRawText();
                if (method == "item/completed")
                    _history.Messages.Add(new() { Role = "파일 변경", Text = item.GetRawText() });
            }
            if (type == "agentMessage" && method == "item/completed")
            {
                var message = _history.Messages.FirstOrDefault(m => m.ItemId == id);
                if (message == null) _history.Messages.Add(new() { Role = "ChatGPT", ItemId = id, Text = item.GetProperty("text").GetString() ?? "" });
                else message.Text = item.GetProperty("text").GetString() ?? "";
            }
        }
        if (method == "item/agentMessage/delta")
        {
            string id = ItemKey(p, p.GetProperty("itemId").GetString()!);
            var message = _history.Messages.FirstOrDefault(m => m.ItemId == id);
            if (message == null)
            {
                message = new() { Role = "ChatGPT", ItemId = id };
                _history.Messages.Add(message);
            }
            message.Text += p.GetProperty("delta").GetString();
        }
        if (method == "turn/completed")
        {
            var turn = p.GetProperty("turn");
            if (turn.TryGetProperty("error", out var error) && error.ValueKind != JsonValueKind.Null)
                _history.Messages.Add(new() { Role = "시스템", Text = error.GetRawText() });
            SetBusy(false);
            _turnId = null;
            SaveHistory();
            string state = turn.TryGetProperty("status", out var status) ? status.GetString() ?? "failed" : "failed";
            StatusText.Text = state == "completed" ? "응답 완료 · 대화 저장됨"
                : state == "interrupted" ? "응답 중지됨 · 이미 수행한 작업은 되돌리지 않습니다."
                : "응답 실패 · 로그와 계정 상태를 확인하세요. 이미 수행한 작업은 유지됩니다.";
        }
        RenderTranscript();
    }

    private bool ConfirmApproval(string method, JsonElement parameters)
    {
        if (_disposed || !_busy) return false;
        if (parameters.TryGetProperty("threadId", out var thread) && thread.GetString() != _threadId) return false;
        if (AllowEdits.IsChecked != true) return false;
        string details = JsonSerializer.Serialize(parameters, new JsonSerializerOptions { WriteIndented = true });
        if (parameters.TryGetProperty("itemId", out var id) && _fileChanges.TryGetValue(ItemKey(parameters, id.GetString()!), out var changes))
            details += "\n\n변경 내용:\n" + changes;
        var window = new Window
        {
            Title = method.Contains("fileChange") ? "AI 파일 수정 승인" : "AI 명령 실행 승인",
            Owner = Application.Current.MainWindow, Width = 760, Height = 560,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            Background = (System.Windows.Media.Brush)FindResource("PixelPanelBrush")
        };
        var grid = new Grid { Margin = new Thickness(12) };
        grid.RowDefinitions.Add(new RowDefinition());
        grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        grid.Children.Add(new TextBox
        {
            Text = "프로젝트: " + _project + "\n이번 요청만 승인합니다. 아래 파일·명령과 경로를 확인하세요.\n\n" + details,
            IsReadOnly = true, TextWrapping = TextWrapping.Wrap, VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
            Background = (System.Windows.Media.Brush)FindResource("PixelBackgroundBrush"),
            Foreground = (System.Windows.Media.Brush)FindResource("PixelTextBrush")
        });
        var buttons = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right };
        var decline = new Button { Content = "거절", IsCancel = true, Padding = new Thickness(18, 7, 18, 7), Margin = new Thickness(5) };
        var accept = new Button { Content = "이번 요청 승인", Padding = new Thickness(18, 7, 18, 7), Margin = new Thickness(5) };
        decline.Click += (_, _) => window.DialogResult = false;
        accept.Click += (_, _) => window.DialogResult = true;
        buttons.Children.Add(decline); buttons.Children.Add(accept);
        Grid.SetRow(buttons, 1); grid.Children.Add(buttons); window.Content = grid;
        bool accepted = window.ShowDialog() == true;
        _history.Messages.Add(new() { Role = "승인", Text = (accepted ? "승인: " : "거절: ") + method });
        SaveHistory();
        return accepted && !_disposed;
    }

    private ChatGptProjectTools.ImportedAsset? PickImportAsset()
    {
        if (_disposed || !_busy || AllowEdits.IsChecked != true) return null;
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Title = "AI가 Asset으로 가져올 파일을 직접 선택하세요",
            Filter = "이미지·오디오·폰트|*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds;*.wav;*.ogg;*.mp3;*.ttf;*.otf",
            CheckFileExists = true, Multiselect = false
        };
        if (dialog.ShowDialog(Application.Current.MainWindow) != true || _disposed || !_busy) return null;
        if (!ChatGptProjectTools.AssetExtensions.Contains(Path.GetExtension(dialog.FileName)))
            throw new InvalidOperationException("지원하지 않는 에셋 형식입니다.");
        using var file = new FileStream(dialog.FileName, FileMode.Open, FileAccess.Read, FileShare.Read);
        if (file.Length > 16 * 1024 * 1024) throw new InvalidOperationException("가져올 에셋은 16MB까지 지원합니다.");
        byte[] bytes = new byte[checked((int)file.Length)];
        file.ReadExactly(bytes);
        return new(Path.GetFileName(dialog.FileName), bytes);
    }

    private async void Stop(object sender, RoutedEventArgs e)
    {
        if (!_busy || _client == null || _threadId == null) return;
        _stopRequested = true;
        if (_turnId == null) return;
        try { await _client.RequestAsync("turn/interrupt", new { threadId = _threadId, turnId = _turnId }); }
        catch (Exception exception) { StatusText.Text = "중지 요청 실패: " + exception.Message; }
    }

    private void NewConversation(object sender, RoutedEventArgs e)
    {
        if (_busy) return;
        // Archive the local view; Codex's old thread remains available in its own history.
        if (_history.Messages.Count > 0)
        {
            string archive = CodexChatHistory.PathFor(_project) + "." + DateTime.UtcNow.ToString("yyyyMMddHHmmssfff") + ".archive";
            try { File.Copy(CodexChatHistory.PathFor(_project), archive); }
            catch (Exception exception) { StatusText.Text = "기록 보관 실패: " + exception.Message; return; }
        }
        _history.ThreadId = _threadId = null;
        _history.Messages.Clear();
        SaveHistory();
        RenderTranscript();
        RefreshAccount(this, new RoutedEventArgs());
    }

    private void PromptKeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter && Keyboard.Modifiers == ModifierKeys.Control) { e.Handled = true; Send(sender, e); }
    }

    private void SetBusy(bool busy)
    {
        _busy = busy;
        if (busy) _stopRequested = false;
        AllowEdits.IsEnabled = !busy;
        AddAccountButton.IsEnabled = !busy && _client != null;
        ReauthorizeButton.IsEnabled = !busy && _client != null;
        AccountBox.IsEnabled = !busy && _client != null;
        ModelBox.IsEnabled = !busy && _threadId != null && ModelBox.SelectedItem is ChatModel;
        SendButton.IsEnabled = NewButton.IsEnabled = ModelBox.IsEnabled;
        StopButton.IsEnabled = busy;
        SendButton.Content = busy ? "응답 진행 중…" : "전송 (Ctrl+Enter)";
        StopButton.Background = (System.Windows.Media.Brush)FindResource(busy ? "PixelAccentBrush" : "PixelSurfaceBrush");
        StopButton.Foreground = busy ? System.Windows.Media.Brushes.Black
            : (System.Windows.Media.Brush)FindResource("PixelTextBrush");
        RefreshButton.IsEnabled = !busy && _client != null;
        if (busy) StatusText.Text = "ChatGPT 응답 중…";
    }

    private void RenderTranscript()
    {
        TranscriptBox.Text = string.Join("\n\n", _history.Messages.Select(m => "[" + m.Role + "]\n" + m.Text));
        ScrollTranscriptToEnd();
    }

    private void ScrollTranscriptToEnd()
    {
        if (_disposed || !IsVisible || _scrollToEndQueued) return;
        _scrollToEndQueued = true;
        // Scroll only after the loaded/visible pane has measured its actual viewport.
        Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.ContextIdle, new Action(() =>
        {
            _scrollToEndQueued = false;
            if (!_disposed && IsVisible) TranscriptBox.ScrollToEnd();
        }));
    }

    private string ItemKey(JsonElement parameters, string id)
    {
        return (parameters.TryGetProperty("turnId", out var turn) ? turn.GetString() : _turnId) + ":" + id;
    }

    private void SaveHistory()
    {
        if (!_historyWritable)
        {
            StatusText.Text = "기존 대화 파일을 보관하지 못해 덮어쓰기를 중단했습니다. 현재 대화는 저장되지 않습니다.";
            return;
        }
        try { _history.Save(); }
        catch (Exception exception) { StatusText.Text = "대화 저장 실패: " + exception.Message; }
    }

    public void DisposeChat()
    {
        _disposed = true;
        SaveHistory();
        // No UI callbacks are awaited during transport shutdown.
        try { _client?.DisposeAsync().AsTask().GetAwaiter().GetResult(); }
        catch (Exception) { System.Diagnostics.Debug.WriteLine("AI Chat connection closed during editor shutdown."); }
    }
}
