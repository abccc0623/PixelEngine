using System.Diagnostics;
using System.Collections.ObjectModel;
using System.Collections.Concurrent;
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
    private CodexEditorClient? _client;
    private string? _threadId;
    private string? _turnId;
    private bool _busy;
    private bool _disposed;
    private bool _historyWritable = true;
    private Task _historySave = Task.CompletedTask;
    private long _historySaveVersion;
    private bool _stopRequested;
    private CancellationTokenSource? _requestStop;
    private TaskCompletionSource<bool>? _requestFinished;
    private readonly HashSet<string> _finishedTurns = new(StringComparer.Ordinal);
    private bool _loadingAccounts;
    private bool _loadingProgrammerModels;
    private const string ProgrammerDefaultEffort = "모델 기본값";
    private bool _restoringPreferences = true;
    private bool _restoreAttempted;
    private bool _scrollToEndQueued;
    private readonly ConcurrentQueue<(CodexEditorClient Client, string Method, JsonElement Parameters)> _notifications = new();
    private int _notificationDrainQueued;
    private readonly object _notificationGate = new();
    private bool _notificationsClosed;
    private sealed record TranscriptMessage(string Text, bool IsUser, bool IsCodeChange = false, string Header = "", bool IsNotice = false, string Details = "")
    {
        public bool HasDetails => Details.Length > 0;
    }
    private readonly ObservableCollection<TranscriptMessage> _transcript = new();
    private readonly Dictionary<CodexChatMessage, (string Role, string Text, string? Details, TranscriptMessage? Entry)> _transcriptCache = new();
    private sealed record ChatAccount(string Id, string Label);
    private sealed record ChatModel(string Model, string DisplayName, bool IsDefault, string? DefaultEffort, string[] SupportedEfforts);
    private readonly Dictionary<string, string> _fileChanges = new();
    private readonly HashSet<string> _changedPaths = new(StringComparer.OrdinalIgnoreCase);
    private CodexChatMessage? _stageNotice;
    private TaskCompletionSource<string>? _choiceAnswer;
    private readonly Stopwatch _requestClock = new();
    private readonly System.Windows.Threading.DispatcherTimer _transcriptRenderTimer =
        new(System.Windows.Threading.DispatcherPriority.Background) { Interval = TimeSpan.FromMilliseconds(50) };
    private readonly System.Windows.Threading.DispatcherTimer _elapsedTimer =
        new(System.Windows.Threading.DispatcherPriority.Background) { Interval = TimeSpan.FromSeconds(1) };

    public AIChatWindow()
    {
        InitializeComponent();
        _elapsedTimer.Tick += (_, _) => UpdateElapsedTime();
        _transcriptRenderTimer.Tick += (_, _) => { _transcriptRenderTimer.Stop(); RenderTranscript(); };
        TranscriptBox.ItemsSource = _transcript;
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
        _restoringPreferences = false;
        RenderTranscript();
        Loaded += RestoreConnection;
        Loaded += (_, _) => ScrollTranscriptToEnd();
        IsVisibleChanged += (_, e) => { if (e.NewValue is true) ScrollTranscriptToEnd(); };
    }

    private async void Connect(object sender, RoutedEventArgs e)
        => await ConnectAsync(allowBrowserLogin: true);

    private async void InstallCli(object sender, RoutedEventArgs e)
    {
        if (_busy) return;
        InstallCliButton.IsEnabled = ConnectButton.IsEnabled = false;
        StatusText.Text = "Codex CLI 설치 중…";
        using var timeout = new CancellationTokenSource(TimeSpan.FromMinutes(5));
        try
        {
            await ArtCodexCli.InstallAsync(timeout.Token);
            await ConnectAsync(allowBrowserLogin: true);
        }
        catch (Exception ex) { StatusText.Text = "CLI 설치 실패: " + ex.Message; ConnectButton.IsEnabled = true; }
        finally { InstallCliButton.IsEnabled = true; }
    }

    private async void RestoreConnection(object sender, RoutedEventArgs e)
    {
        if (_restoreAttempted || _disposed || _client != null) return;
        _restoreAttempted = true;
        await ConnectAsync(allowBrowserLogin: false);
    }

    private void ToggleAccountDetails(object sender, RoutedEventArgs e)
    {
        AccountDetails.Visibility = AccountDetails.Visibility == Visibility.Visible ? Visibility.Collapsed : Visibility.Visible;
    }

    private void ToggleChatSettings(object sender, RoutedEventArgs e)
    {
        SettingsPanel.Visibility = SettingsPanel.Visibility == Visibility.Visible ? Visibility.Collapsed : Visibility.Visible;
    }

    private async Task ConnectAsync(bool allowBrowserLogin)
    {
        ConnectButton.IsEnabled = false;
        ProgrammerModelBox.IsEnabled = ProgrammerEffortBox.IsEnabled = false;
        PlannerModelBox.IsEnabled = PlannerEffortBox.IsEnabled = ArtistModelBox.IsEnabled = ArtistEffortBox.IsEnabled = false;
        ModelBox.IsEnabled = SendButton.IsEnabled = NewButton.IsEnabled = false;
        try
        {
            if (_client != null) await _client.DisposeAsync();
            _threadId = _turnId = null;
            _client = new CodexEditorClient();
            if (_history.ThreadId?.StartsWith("chatgpt-", StringComparison.Ordinal) == true)
            {
                _history.ThreadId = null;
                _history.Messages.Add(new() { Role = "작업 알림", Text = "Codex CLI 연결로 변경했습니다. 기존 대화는 화면에 보존되며 새 CLI 대화로 시작합니다." });
                SaveHistory();
                RenderTranscript();
            }
            var connectedClient = _client;
            _client.Notification += (method, p) => QueueNotification(connectedClient, method, p);
            _client.Disconnected += details => QueueNotification(connectedClient, "connection/disconnected", JsonSerializer.SerializeToElement(details));
            _client.Approve = (_, _) => Task.FromResult(false);
            _client.PickAsset = () => Dispatcher.InvokeAsync(PickImportAsset).Task;
            _client.ReadEditorLogs = (afterCursor, limit) => ConsoleWindow.ReadLogsForAi(afterCursor, limit);
            _client.ClearEditorLogs = () => Dispatcher.InvokeAsync(() =>
                ((MainWindow)Application.Current.MainWindow).ClearEditorLogsForAi()).Task;
            _client.TestGameKey = (key, frames, token) => Dispatcher.InvokeAsync(() =>
                ((MainWindow)Application.Current.MainWindow).TestGameKeyForAiAsync(key, frames, token)).Task.Unwrap();
            _client.SetEditorMode = mode => Dispatcher.InvokeAsync(() =>
                ((MainWindow)Application.Current.MainWindow).SetEditorModeForAi(mode)).Task;
            _client.HighlightLuaLines = (path, first, last) => Dispatcher.InvokeAsync(() =>
                ((MainWindow)Application.Current.MainWindow).HighlightLuaLines(path, first, last)).Task;
            _client.CanDeleteAssetFile = relative => Dispatcher.InvokeAsync(() => !_disposed && _busy && !_stopRequested &&
                GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.HasUnsavedFile(Path.Combine(_project, relative)) != true).Task;
            _client.AskChoice = (question, options, token) => Dispatcher.InvokeAsync(() =>
                AskChoiceAsync(question, options, token)).Task.Unwrap();
            _client.AskPlanApproval = (question, details, options, token) => Dispatcher.InvokeAsync(() =>
                AskChoiceWithDetailsAsync(question, options, token, details)).Task.Unwrap();
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
            ProgrammerModelBox.IsEnabled = ProgrammerEffortBox.IsEnabled = false;
        PlannerModelBox.IsEnabled = PlannerEffortBox.IsEnabled = ArtistModelBox.IsEnabled = ArtistEffortBox.IsEnabled = false;
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
        NewButton.IsEnabled = true;
        UpdateProgrammerSelection();
        if (!ProgrammerSelectionValid || !PlannerSelectionValid || !ArtistSelectionValid) SettingsPanel.Visibility = Visibility.Visible;
        ConnectButton.IsEnabled = false;
        SaveHistory();
        return true;
    }

    private async void AccountChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingAccounts || _busy || _client == null || AccountBox.SelectedItem is not ChatAccount selected) return;
        ProgrammerModelBox.IsEnabled = ProgrammerEffortBox.IsEnabled = false;
        PlannerModelBox.IsEnabled = PlannerEffortBox.IsEnabled = ArtistModelBox.IsEnabled = ArtistEffortBox.IsEnabled = false;
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
        ProgrammerModelBox.IsEnabled = ProgrammerEffortBox.IsEnabled = false;
        PlannerModelBox.IsEnabled = PlannerEffortBox.IsEnabled = ArtistModelBox.IsEnabled = ArtistEffortBox.IsEnabled = false;
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
                    entry.TryGetProperty("defaultReasoningEffort", out var effort) ? effort.GetString() : null,
                    ReadSupportedEfforts(entry)));
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
        _loadingProgrammerModels = true;
        try
        {
            PlannerModelBox.ItemsSource = models;
            ArtistModelBox.ItemsSource = models;
            PlannerModelBox.SelectedItem = models.FirstOrDefault(m => m.Model == _history.PlannerModel)
                ?? (string.IsNullOrWhiteSpace(_history.PlannerModel) ? ModelBox.SelectedItem : null);
            ArtistModelBox.SelectedItem = models.FirstOrDefault(m => m.Model == _history.ArtistModel)
                ?? (string.IsNullOrWhiteSpace(_history.ArtistModel) ? ModelBox.SelectedItem : null);
            ProgrammerModelBox.ItemsSource = models;
            string? saved = _history.ProgrammerModel;
            ProgrammerModelBox.SelectedItem = !string.IsNullOrWhiteSpace(saved)
                ? models.FirstOrDefault(m => m.Model == saved)
                : models.FirstOrDefault(m => m.Model.StartsWith("gpt-5.", StringComparison.OrdinalIgnoreCase) && m.Model.Contains("codex", StringComparison.OrdinalIgnoreCase))
                    ?? models.FirstOrDefault(m => m.Model.StartsWith("gpt-5.", StringComparison.OrdinalIgnoreCase))
                    ?? models.FirstOrDefault(m => m.IsDefault) ?? models[0];
            if (ProgrammerModelBox.SelectedItem is ChatModel programmer)
            {
                _history.ProgrammerModel = programmer.Model;
                LoadProgrammerEfforts(programmer, _history.ProgrammerEffort);
            }
            else ProgrammerEffortBox.ItemsSource = null;
            if (PlannerModelBox.SelectedItem is ChatModel planner)
            {
                _history.PlannerModel = planner.Model;
                LoadRoleEfforts(PlannerEffortBox, planner, _history.PlannerEffort, effort => _history.PlannerEffort = effort);
            }
            else PlannerEffortBox.ItemsSource = null;
            if (ArtistModelBox.SelectedItem is ChatModel artist)
            {
                _history.ArtistModel = artist.Model;
                LoadRoleEfforts(ArtistEffortBox, artist, _history.ArtistEffort, effort => _history.ArtistEffort = effort);
            }
            else ArtistEffortBox.ItemsSource = null;
        }
        finally { _loadingProgrammerModels = false; }
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private static string[] ReadSupportedEfforts(JsonElement entry)
    {
        if (entry.TryGetProperty("supportedReasoningEfforts", out var efforts) && efforts.ValueKind == JsonValueKind.Array)
            return efforts.EnumerateArray().Select(e => e.ValueKind == JsonValueKind.String ? e.GetString()
                : e.TryGetProperty("reasoningEffort", out var value) ? value.GetString() : null)
                .Where(e => !string.IsNullOrWhiteSpace(e)).Cast<string>().Distinct().ToArray();
        return entry.TryGetProperty("defaultReasoningEffort", out var fallback) && fallback.GetString() is string effort
            ? new[] { effort } : Array.Empty<string>();
    }

    private void LoadProgrammerEfforts(ChatModel model, string? saved)
    {
        LoadRoleEfforts(ProgrammerEffortBox, model, saved, effort => _history.ProgrammerEffort = effort);
    }

    private static void LoadRoleEfforts(ComboBox box, ChatModel model, string? saved, Action<string> save)
    {
        box.ItemsSource = new[] { ProgrammerDefaultEffort }.Concat(model.SupportedEfforts).ToArray();
        string selected = saved ?? model.DefaultEffort ?? "";
        box.SelectedItem = selected.Length == 0 ? ProgrammerDefaultEffort : selected;
        save(selected);
    }

    private bool ProgrammerSelectionValid => ProgrammerModelBox.SelectedItem is ChatModel model &&
        (string.IsNullOrWhiteSpace(_history.ProgrammerEffort) || model.SupportedEfforts.Contains(_history.ProgrammerEffort));

    private bool PlannerSelectionValid => PlannerModelBox.SelectedItem is ChatModel model &&
        (string.IsNullOrWhiteSpace(_history.PlannerEffort) || model.SupportedEfforts.Contains(_history.PlannerEffort));

    private bool ArtistSelectionValid => ArtistModelBox.SelectedItem is ChatModel model &&
        (string.IsNullOrWhiteSpace(_history.ArtistEffort) || model.SupportedEfforts.Contains(_history.ArtistEffort));

    private void UpdateProgrammerSelection()
    {
        bool ready = !_busy && _threadId != null;
        ProgrammerModelBox.IsEnabled = !_busy && ProgrammerModelBox.Items.Count > 0;
        ProgrammerEffortBox.IsEnabled = !_busy && ProgrammerModelBox.SelectedItem is ChatModel && ProgrammerEffortBox.Items.Count > 0;
        PlannerModelBox.IsEnabled = !_busy && PlannerModelBox.Items.Count > 0;
        PlannerEffortBox.IsEnabled = !_busy && PlannerModelBox.SelectedItem is ChatModel && PlannerEffortBox.Items.Count > 0;
        ArtistModelBox.IsEnabled = !_busy && ArtistModelBox.Items.Count > 0;
        ArtistEffortBox.IsEnabled = !_busy && ArtistModelBox.SelectedItem is ChatModel && ArtistEffortBox.Items.Count > 0;
        ProgrammerModelWarning.Visibility = ProgrammerSelectionValid && PlannerSelectionValid && ArtistSelectionValid ? Visibility.Collapsed : Visibility.Visible;
        ProgrammerModelWarning.Text = !PlannerSelectionValid ? "기획자 모델과 추론 강도를 선택하세요."
            : !ArtistSelectionValid ? "아티스트 모델과 추론 강도를 선택하세요."
            : ProgrammerModelBox.SelectedItem is not ChatModel
            ? string.IsNullOrWhiteSpace(_history.ProgrammerModel)
                ? "플머 모델을 선택하세요."
                : $"저장된 플머 모델 '{_history.ProgrammerModel}'을 사용할 수 없습니다. 모델을 선택하세요."
            : $"저장된 플머 추론 강도 '{_history.ProgrammerEffort}'를 사용할 수 없습니다. 추론 강도를 선택하세요.";
        SendButton.IsEnabled = ready && ModelBox.SelectedItem is ChatModel && ProgrammerSelectionValid && PlannerSelectionValid && ArtistSelectionValid;
    }

    private void ProgrammerModelChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || ProgrammerModelBox.SelectedItem is not ChatModel model) return;
        _loadingProgrammerModels = true;
        try
        {
            _history.ProgrammerModel = model.Model;
            LoadProgrammerEfforts(model, null);
        }
        finally { _loadingProgrammerModels = false; }
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void ProgrammerEffortChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || ProgrammerEffortBox.SelectedItem is not string effort) return;
        _history.ProgrammerEffort = effort == ProgrammerDefaultEffort ? "" : effort;
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void PlannerModelChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || PlannerModelBox.SelectedItem is not ChatModel model) return;
        _loadingProgrammerModels = true;
        try { _history.PlannerModel = model.Model; LoadRoleEfforts(PlannerEffortBox, model, null, effort => _history.PlannerEffort = effort); }
        finally { _loadingProgrammerModels = false; }
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void PlannerEffortChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || PlannerEffortBox.SelectedItem is not string effort) return;
        _history.PlannerEffort = effort == ProgrammerDefaultEffort ? "" : effort;
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void ArtistModelChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || ArtistModelBox.SelectedItem is not ChatModel model) return;
        _loadingProgrammerModels = true;
        try { _history.ArtistModel = model.Model; LoadRoleEfforts(ArtistEffortBox, model, null, effort => _history.ArtistEffort = effort); }
        finally { _loadingProgrammerModels = false; }
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void ArtistEffortChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_loadingProgrammerModels || ArtistEffortBox.SelectedItem is not string effort) return;
        _history.ArtistEffort = effort == ProgrammerDefaultEffort ? "" : effort;
        SaveHistory();
        UpdateProgrammerSelection();
    }

    private void ModelChanged(object sender, SelectionChangedEventArgs e)
    {
        if (ModelBox.SelectedItem is not ChatModel selected) return;
        _history.Model = selected.Model;
        SaveHistory();
    }

    private async Task PrepareThreadAsync()
    {
        if (_threadId != null || _client == null) return;
        JsonElement result;
        if (!string.IsNullOrEmpty(_history.ThreadId) && _history.ToolSchemaVersion < CodexChatHistory.CurrentToolSchemaVersion)
        {
            _history.PreviousThreadIds ??= new();
            if (!_history.PreviousThreadIds.Contains(_history.ThreadId)) _history.PreviousThreadIds.Add(_history.ThreadId);
            _history.ThreadId = null;
            AddWorkNotice("새 작업 도구를 적용하려고 새 AI 대화로 연결합니다. 화면의 이전 메시지와 이전 대화 ID는 보관됩니다. 과거 작업은 프로젝트 기억을 검색해 확인합니다.");
            SaveHistory();
        }
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
        // CLI persists a new thread only after its first turn; do not save an empty thread id.
        _threadId = thread.GetProperty("id").GetString();
        _history.ToolSchemaVersion = CodexChatHistory.CurrentToolSchemaVersion;
    }

    private async void RefreshAccount(object sender, RoutedEventArgs e)
    {
        if (_busy) return;
        try { if (!await LoadAccountAsync()) StatusText.Text = "ChatGPT 로그인 대기 중입니다."; }
        catch (Exception exception) { StatusText.Text = "계정 확인 실패: " + exception.Message; }
    }

    private async void Send(object sender, RoutedEventArgs e)
    {
        if (_busy || !ProgrammerSelectionValid || !PlannerSelectionValid || !ArtistSelectionValid || _client == null || _threadId == null || ModelBox.SelectedItem is not ChatModel selected || string.IsNullOrWhiteSpace(PromptBox.Text)) return;
        string text = PromptBox.Text;
        _changedPaths.Clear();
        _stageNotice = null;
        SetBusy(true);
        var request = _requestStop!;
        var client = _client;
        PromptBox.Clear();
        _history.Messages.Add(new() { Role = "사용자", Text = text });
        SetPipelineStage("요청 분석");
        try
        {
            var auth = await client.RequestAsync("account/read", new { refreshToken = false }).WaitAsync(request.Token);
            request.Token.ThrowIfCancellationRequested();
            if (!auth.TryGetProperty("account", out var account) || account.ValueKind == JsonValueKind.Null ||
                account.GetProperty("type").GetString() != "chatgpt")
                throw new InvalidOperationException("ChatGPT 계정 연결을 다시 확인하세요. API 키로는 요청하지 않습니다.");
            var result = await client.RequestAsync("turn/start", new
            {
                threadId = _threadId,
                model = selected.Model,
                effort = selected.DefaultEffort,
                programmerModel = _history.ProgrammerModel,
                programmerEffort = string.IsNullOrEmpty(_history.ProgrammerEffort) ? null : _history.ProgrammerEffort,
                plannerModel = _history.PlannerModel,
                plannerEffort = string.IsNullOrEmpty(_history.PlannerEffort) ? null : _history.PlannerEffort,
                artistModel = _history.ArtistModel,
                artistEffort = string.IsNullOrEmpty(_history.ArtistEffort) ? null : _history.ArtistEffort,
                requirePlan = RequirePlan.IsChecked == true,
                approvalPolicy = "on-request",
                sandboxPolicy = new { type = "workspaceWrite", writableRoots = new[] { _project }, networkAccess = false },
                input = new[]
                {
                    new { type = "text", text }
                }
            }, request.Token);
            if (_busy && ReferenceEquals(_requestStop, request))
            {
                _history.ThreadId = _threadId;
                SaveHistory();
                _turnId = result.GetProperty("turn").GetProperty("id").GetString();
                // The adapter also cancels requests stopped before this ID arrived.
            }
        }
        catch (OperationCanceledException) when (request.IsCancellationRequested)
        {
            if (!ReferenceEquals(_requestStop, request)) return;
            SetBusy(false);
            StatusText.Text = "전송 취소됨";
            AddWorkNotice("사용자가 요청 전송을 중지했습니다. 이미 수행한 변경은 자동으로 되돌리지 않습니다.");
        }
        catch (Exception exception)
        {
            if (!ReferenceEquals(_requestStop, request)) return;
            await client.DisposeAsync();
            _client = null;
            _threadId = null;
            ConnectButton.IsEnabled = true;
            _history.Messages.Add(new() { Role = "작업 알림", Text = "요청 실패: " + exception.Message + "\n이미 수행한 변경은 유지됩니다. 계정·연결 상태를 확인한 뒤 다시 요청해 주세요." });
            SetBusy(false);
            SaveHistory();
            RenderTranscript();
        }
    }

    private void QueueNotification(CodexEditorClient client, string method, JsonElement parameters)
    {
        // CLI reasoning/tool progress that has no presentation must never flood the UI dispatcher.
        if (_disposed || method is not ("account/login/completed" or "turn/usage" or "turn/pipelineStage" or
            "turn/contextCompacted" or "thread/compacted" or "turn/toolWarning" or "turn/started" or
            "item/started" or "item/completed" or "item/agentMessage/delta" or "turn/completed" or "connection/disconnected")) return;
        lock (_notificationGate)
        {
            if (_notificationsClosed) return;
            _notifications.Enqueue((client, method, parameters));
        }
        ScheduleNotificationDrain();
    }

    private void ScheduleNotificationDrain()
    {
        if (Interlocked.CompareExchange(ref _notificationDrainQueued, 1, 0) == 0)
            Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Background, new Action(DrainNotifications));
    }

    private void DrainNotifications()
    {
        try
        {
            if (_disposed) { _notifications.Clear(); return; }
            var clock = Stopwatch.StartNew();
            // Preserve event order (including file receipts before completion), but yield to input/rendering.
            for (int count = 0; count < 64 && clock.ElapsedMilliseconds < 4 && _notifications.TryDequeue(out var pending); count++)
            {
                if (!ReferenceEquals(_client, pending.Client)) continue;
                if (pending.Method == "connection/disconnected") HandleDisconnected(pending.Parameters.GetString()!);
                else HandleNotification(pending.Method, pending.Parameters);
            }
        }
        finally
        {
            Interlocked.Exchange(ref _notificationDrainQueued, 0);
            if (!_notifications.IsEmpty) ScheduleNotificationDrain();
        }
    }

    private void HandleDisconnected(string details)
    {
        bool wasBusy = _busy;
        SetBusy(false);
        SendButton.IsEnabled = RefreshButton.IsEnabled = NewButton.IsEnabled = false;
        ProgrammerModelBox.IsEnabled = ProgrammerEffortBox.IsEnabled = ModelBox.IsEnabled = false;
        ConnectButton.IsEnabled = true;
        StatusText.Text = "ChatGPT 연결이 종료되었습니다. 아래 오류 상세를 확인하고 계정 연결을 다시 눌러주세요.";
        AddWorkNotice((wasBusy ? "연결이 끊겨 요청을 완료하지 못했습니다. 이미 수행한 변경은 유지됩니다." : "ChatGPT 연결이 종료되었습니다.") + "\n" + details);
    }

    private void QueueTranscriptRender()
    {
        if (!_transcriptRenderTimer.IsEnabled) _transcriptRenderTimer.Start();
    }

    private async void HandleNotification(string method, JsonElement p)
    {
        if (_disposed || p.ValueKind != JsonValueKind.Object) return;
        try { ApplyNotification(method, p); }
        catch (Exception exception) when (exception is JsonException or InvalidOperationException or KeyNotFoundException)
        {
            if (_client != null)
            {
                try { await _client.RequestAsync("turn/interrupt", new { threadId = _threadId, turnId = _turnId }); }
                catch (Exception) { /* The visible notice below also covers transport failure. */ }
            }
            SetBusy(false);
            SendButton.IsEnabled = NewButton.IsEnabled = false;
            ConnectButton.IsEnabled = true;
            StatusText.Text = "ChatGPT 응답 형식을 처리하지 못했습니다. 응답을 중지하고 계정 연결을 다시 눌러주세요.";
            AddWorkNotice("응답 형식을 처리하지 못해 작업을 중지했습니다. 계정을 다시 연결해 주세요. 이미 수행한 변경은 유지됩니다.");
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
        string? eventTurn = p.TryGetProperty("turnId", out var turnId) ? turnId.GetString()
            : p.TryGetProperty("turn", out var turnValue) && turnValue.TryGetProperty("id", out var idValue) ? idValue.GetString() : null;
        if (!_busy || (eventTurn != null && (_finishedTurns.Contains(eventTurn) || (_turnId != null && eventTurn != _turnId)))) return;
        // Keep the stop status stable while the CLI finishes interrupting. File receipts
        // are retained because an operation committed just before cancellation may exist.
        if (_stopRequested && method is "item/agentMessage/delta" or "turn/teamStatus" or "turn/agentDispatch") return;
        if (method == "turn/usage")
        {
            UsageText.Text = p.GetProperty("text").GetString();
            return;
        }
        if (method == "turn/agentDispatch")
        {
            return;
        }
        if (method == "turn/pipelineStage")
        {
            SetPipelineStage(p.GetProperty("stage").GetString() ?? "작업 중");
            return;
        }
        if (method == "turn/teamStatus")
        {
            return;
        }
        if (method == "turn/contextCompacted" || method == "thread/compacted")
        {
            StatusText.Text = "전송용 대화 문맥 정리 완료 · 화면의 대화 기록은 유지됩니다.";
            return;
        }
        if (method == "turn/toolWarning")
        {
            StatusText.Text = "도구 오류 · Editor/AIToolFailures.md 확인";
            return;
        }
        if (method == "turn/started" && p.TryGetProperty("turn", out var started))
        {
            _turnId = started.GetProperty("id").GetString();
            return;
        }
        if (method is "item/started" or "item/completed")
        {
            var item = p.GetProperty("item");
            string? type = item.GetProperty("type").GetString();
            string id = ItemKey(p, item.GetProperty("id").GetString()!);
            if (type == "fileChange")
            {
                _fileChanges[id] = item.GetRawText();
                if (method == "item/completed")
                {
                    if (item.TryGetProperty("path", out var changedPath) && changedPath.ValueKind == JsonValueKind.String)
                        _changedPaths.Add(changedPath.GetString()!);
                    _history.Messages.Add(new() { Role = "파일 변경", Text = item.GetRawText() });
                    if (item.TryGetProperty("result", out var result) && result.ValueKind == JsonValueKind.String &&
                        (result.GetString() ?? "").TrimStart().StartsWith('{'))
                    {
                        using var change = JsonDocument.Parse(result.GetString()!);
                        var data = change.RootElement;
                        foreach (string key in new[] { "saved", "created", "moved", "destination", "deleted" })
                            if (data.TryGetProperty(key, out var changed) && changed.ValueKind == JsonValueKind.String)
                                _changedPaths.Add(changed.GetString()!);
                        if (data.TryGetProperty("moved", out var moved) && data.TryGetProperty("destination", out var destination))
                            GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.UpdateMovedFilePaths(
                                Path.Combine(_project, moved.GetString()!), Path.Combine(_project, destination.GetString()!));
                        if (data.TryGetProperty("moved", out _) || data.TryGetProperty("deleted", out _) || data.TryGetProperty("created", out _))
                            GlobalFunction.GetDockedWindow<AssetWindow>()?.RefreshAfterAiChange();
                    }
                }
            }
            if (type == "agentMessage" && method == "item/completed" && !_stopRequested)
            {
                var message = _history.Messages.FirstOrDefault(m => m.ItemId == id);
                if (message == null) _history.Messages.Add(new() { Role = "ChatGPT", ItemId = id, Text = item.GetProperty("text").GetString() ?? "" });
                else message.Text = item.GetProperty("text").GetString() ?? "";
            }
            if (method == "item/completed" && type is ("fileChange" or "agentMessage")) QueueTranscriptRender();
            return;
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
            QueueTranscriptRender();
            return;
        }
        if (method == "turn/completed")
        {
            _transcriptRenderTimer.Stop();
            if (!_busy) return;
            var turn = p.GetProperty("turn");
            if (turn.TryGetProperty("error", out var error) && error.ValueKind != JsonValueKind.Null)
                _history.Messages.Add(new() { Role = "시스템", Text = error.GetRawText() });
            if (eventTurn != null) _finishedTurns.Add(eventTurn);
            bool stopped = _stopRequested;
            SetBusy(false);
            _turnId = null;
            string state = turn.TryGetProperty("status", out var status) ? status.GetString() ?? "failed" : "failed";
            if (stopped && state == "completed") state = "interrupted";
            StatusText.Text = state == "completed" ? "응답 완료"
                : state == "interrupted" ? "응답 중지됨 · 이미 수행한 작업은 되돌리지 않습니다."
                : "응답 실패 · 로그와 계정 상태를 확인하세요. 이미 수행한 작업은 유지됩니다.";
            if (state != "completed")
            {
                string reason = state == "interrupted" ? "사용자가 요청을 중지했습니다." : "요청 처리에 실패했습니다.";
                if (error.ValueKind == JsonValueKind.Object && error.TryGetProperty("message", out var detail) && detail.ValueKind == JsonValueKind.String)
                    reason += "\n" + detail.GetString();
                AddWorkNotice(reason + "\n" + ElapsedTimeText.Text + " · 이미 수행한 변경은 자동으로 되돌리지 않습니다.");
            }
            if (_changedPaths.Count > 0)
                AddWorkNotice("변경된 파일\n" + string.Join("\n", _changedPaths.Order(StringComparer.OrdinalIgnoreCase).Select(path => "- " + path)));
            SaveHistory();
            RenderTranscript();
        }
    }

    private ChatGptProjectTools.ImportedAsset? PickImportAsset()
    {
        if (_disposed || !_busy || _stopRequested) return null;
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Title = "AI가 Asset으로 가져올 파일을 직접 선택하세요",
            Filter = "이미지·오디오·폰트|*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds;*.wav;*.ogg;*.mp3;*.ttf;*.otf",
            CheckFileExists = true, Multiselect = false
        };
        if (dialog.ShowDialog(Application.Current.MainWindow) != true || _disposed || !_busy || _stopRequested) return null;
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
        if (!_busy || _stopRequested || _client == null || _threadId == null) return;
        _stopRequested = true;
        var request = _requestStop;
        var finished = _requestFinished!.Task;
        var client = _client;
        request?.Cancel();
        StopButton.Content = "중지 중…";
        StatusText.Text = "요청 중지 중… 이미 수행한 변경은 유지됩니다.";
        try
        {
            // Covers both a missing turn/start ID and an interrupt reply without a
            // terminal event. Never leave canceled tools behind a permanently busy UI.
            await Task.WhenAll(client.RequestAsync("turn/interrupt", new { threadId = _threadId, turnId = _turnId }), finished)
                .WaitAsync(TimeSpan.FromSeconds(10));
        }
        catch (Exception)
        {
            if (!_busy || !ReferenceEquals(_requestStop, request) || !ReferenceEquals(_client, client)) return;
            _client = null;
            _threadId = _turnId = null;
            SetBusy(false);
            ConnectButton.IsEnabled = false;
            StatusText.Text = "응답 중지를 확인하지 못해 AI 연결을 닫는 중…";
            string notice = "중지 응답을 확인하지 못해 이 채팅의 AI 연결과 보조 작업을 닫았습니다. 이미 수행한 변경은 유지됩니다.";
            try { await client.DisposeAsync(); }
            catch (Exception ex) when (ex is not OutOfMemoryException)
            {
                notice = "로컬 도구는 취소했지만 AI 연결 종료를 확인하지 못했습니다. " + CodexChatClient.SanitizeDiagnostic(ex.Message) + " 이미 수행한 변경은 유지됩니다.";
            }
            if (!_disposed && _client == null)
            {
                ConnectButton.IsEnabled = true;
                StatusText.Text = "설정에서 AI 계정을 다시 연결한 뒤 요청하세요.";
                AddWorkNotice(notice);
            }
        }
    }

    private Task<string> AskChoiceAsync(string question, string[] options, CancellationToken token) =>
        AskChoiceWithDetailsAsync(question, options, token, "");

    private async Task<string> AskChoiceWithDetailsAsync(string question, string[] options, CancellationToken token, string details)
    {
        token.ThrowIfCancellationRequested();
        if (_disposed || !_busy || _stopRequested) throw new OperationCanceledException(token);
        var answer = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
        _choiceAnswer = answer;
        ChoiceQuestion.Text = question;
        ChoiceDetailsText.Text = details;
        ChoiceDetails.IsExpanded = false;
        ChoiceDetails.Visibility = details.Length == 0 ? Visibility.Collapsed : Visibility.Visible;
        ChoiceOptions.ItemsSource = options;
        ChoiceCustomText.Clear();
        ChoicePanel.IsEnabled = true;
        ChoicePanel.Visibility = Visibility.Visible;
        StatusText.Text = "답변 대기 중 · 선택지를 클릭하거나 직접 입력해 주세요.";
        _history.Messages.Add(new() { Role = "ChatGPT", Text = question + "\n" +
            string.Join("\n", options.Select((option, index) => $"{index + 1}. {option}")), Details = details });
        SaveHistory();
        RenderTranscript();
        using var cancellation = token.Register(() => answer.TrySetCanceled(token));
        try
        {
            string selected = await answer.Task;
            token.ThrowIfCancellationRequested();
            _history.Messages.Add(new() { Role = "사용자", Text = selected });
            SaveHistory();
            RenderTranscript();
            return selected;
        }
        finally
        {
            if (ReferenceEquals(_choiceAnswer, answer))
            {
                _choiceAnswer = null;
                ChoicePanel.Visibility = Visibility.Collapsed;
                ChoiceDetails.IsExpanded = false;
                ChoiceDetails.Visibility = Visibility.Collapsed;
                ChoiceDetailsText.Clear();
                ChoiceOptions.ItemsSource = null;
                ChoiceCustomText.Clear();
                if (!_disposed && _busy && !_stopRequested) StatusText.Text = "AI 자비스 응답 중…";
            }
        }
    }

    private void ChooseOption(object sender, RoutedEventArgs e)
    {
        if (sender is Button { DataContext: string option }) SubmitChoice(option);
    }

    private void SubmitCustomChoice(object sender, RoutedEventArgs e) => SubmitChoice(ChoiceCustomText.Text.Trim());

    private void SubmitChoice(string answer)
    {
        if (!string.IsNullOrWhiteSpace(answer) && _choiceAnswer?.TrySetResult(answer) == true)
            ChoicePanel.IsEnabled = false;
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
        _transcriptCache.Clear();
        _requestClock.Reset();
        ElapsedTimeText.Visibility = Visibility.Collapsed;
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
        bool wasBusy = _busy;
        _busy = busy;
        if (busy && !wasBusy)
        {
            _requestStop = new CancellationTokenSource();
            _requestFinished = new(TaskCreationOptions.RunContinuationsAsynchronously);
            _turnId = null;
            _requestClock.Restart();
            UsageText.Text = "토큰 사용량 수신 대기 · 청구 비용은 표시하지 않음";
            _elapsedTimer.Start();
        }
        else if (!busy && wasBusy)
        {
            _requestFinished?.TrySetResult(true);
            _requestStop?.Cancel();
            _requestStop?.Dispose();
            _requestStop = null;
            _requestClock.Stop();
            _elapsedTimer.Stop();
        }
        if (busy || wasBusy) UpdateElapsedTime();
        if (busy) _stopRequested = false;
        RequirePlan.IsEnabled = !busy;
        AddAccountButton.IsEnabled = !busy && _client != null;
        ReauthorizeButton.IsEnabled = !busy && _client != null;
        AccountBox.IsEnabled = !busy && _client != null;
        ModelBox.IsEnabled = !busy && _threadId != null && ModelBox.SelectedItem is ChatModel;
        NewButton.IsEnabled = ModelBox.IsEnabled;
        UpdateProgrammerSelection();
        StopButton.IsEnabled = busy;
        StopButton.Content = "요청 중지";
        SendButton.Content = busy ? "응답 진행 중…" : "전송 (Ctrl+Enter)";
        StopButton.Background = (System.Windows.Media.Brush)FindResource(busy ? "PixelAccentBrush" : "PixelSurfaceBrush");
        StopButton.Foreground = busy ? System.Windows.Media.Brushes.Black
            : (System.Windows.Media.Brush)FindResource("PixelTextBrush");
        RefreshButton.IsEnabled = !busy && _client != null;
        if (busy) StatusText.Text = "AI 자비스 응답 중…";
    }

    private void UpdateElapsedTime()
    {
        var elapsed = _requestClock.Elapsed;
        string duration = $"{(int)elapsed.TotalHours:00}:{elapsed.Minutes:00}:{elapsed.Seconds:00}";
        ElapsedTimeText.Text = (_busy ? "경과 " : "총 소요 ") + duration;
        ElapsedTimeText.Visibility = Visibility.Visible;
    }

    private void RenderTranscript()
    {
        int index = 0;
        bool changed = false;
        foreach (var message in _history.Messages)
        {
            // Historic text and before/after diffs are immutable between updates. Do not rebuild
            // them for every streamed token; a collapsed diff can contain an entire source file.
            if (!_transcriptCache.TryGetValue(message, out var cached) || cached.Role != message.Role ||
                !ReferenceEquals(cached.Text, message.Text) || !ReferenceEquals(cached.Details, message.Details))
            {
                TranscriptMessage? entry = null;
                if (message.Role == "파일 변경") entry = ReadCodeChange(message.Text);
                else if (message.Role is "사용자" or "ChatGPT" or "작업 알림")
                {
                    string role = message.Role == "ChatGPT" ? "AI 자비스" : message.Role;
                    string text = message.Role == "작업 알림" ? message.Text : "[" + role + "]\n" + message.Text;
                    entry = new TranscriptMessage(text, message.Role == "사용자", IsNotice: message.Role == "작업 알림", Details: message.Details ?? "");
                }
                cached = (message.Role, message.Text, message.Details, entry);
                _transcriptCache[message] = cached;
            }
            if (cached.Entry == null) continue;
            if (index == _transcript.Count) { _transcript.Add(cached.Entry); changed = true; }
            else if (_transcript[index] != cached.Entry) { _transcript[index] = cached.Entry; changed = true; }
            index++;
        }
        while (_transcript.Count > index) { _transcript.RemoveAt(_transcript.Count - 1); changed = true; }
        if (changed) ScrollTranscriptToEnd();
    }

    private void AddWorkNotice(string text)
    {
        _history.Messages.Add(new() { Role = "작업 알림", Text = text });
        SaveHistory();
        RenderTranscript();
    }

    private void SetPipelineStage(string stage)
    {
        if (_stageNotice?.Text == "진행 단계: " + stage) return;
        _stageNotice ??= new CodexChatMessage { Role = "작업 알림" };
        if (!_history.Messages.Contains(_stageNotice)) _history.Messages.Add(_stageNotice);
        _stageNotice.Text = "진행 단계: " + stage;
        StatusText.Text = _stageNotice.Text;
        SaveHistory();
        RenderTranscript();
    }

    private static TranscriptMessage? ReadCodeChange(string json)
    {
        try
        {
            using var document = JsonDocument.Parse(json);
            var item = document.RootElement;
            if (!item.TryGetProperty("path", out var path) || !item.TryGetProperty("before", out var before) ||
                !item.TryGetProperty("after", out var after)) return null;
            string[] oldLines = (before.GetString() ?? "").Replace("\r\n", "\n").Split('\n');
            string[] newLines = (after.GetString() ?? "").Replace("\r\n", "\n").Split('\n');
            int start = 0, oldEnd = oldLines.Length, newEnd = newLines.Length;
            while (start < oldEnd && start < newEnd && oldLines[start] == newLines[start]) start++;
            while (oldEnd > start && newEnd > start && oldLines[oldEnd - 1] == newLines[newEnd - 1]) { oldEnd--; newEnd--; }
            if (start == oldEnd && start == newEnd) return null;
            string diff = $"@@ -{start + 1},{oldEnd - start} +{start + 1},{newEnd - start} @@\n" +
                string.Join("\n", oldLines[start..oldEnd].Select(line => "- " + line)
                    .Concat(newLines[start..newEnd].Select(line => "+ " + line)));
            return new TranscriptMessage(diff, false, true, "변경된 코드 · " + path.GetString());
        }
        catch (JsonException) { return null; }
    }

    private void CopyTranscript(object sender, RoutedEventArgs e)
    {
        if (_transcript.Count == 0) return;
        try { Clipboard.SetText(string.Join("\n\n", _transcript.Select(message =>
            message.IsCodeChange ? message.Header + "\n" + message.Text :
                message.Text + (message.HasDetails ? "\n\n승인 상세\n" + message.Details : "")))); }
        catch (System.Runtime.InteropServices.COMException) { StatusText.Text = "클립보드가 사용 중입니다. 다시 복사해 주세요."; }
    }

    private void TranscriptKeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key != Key.C || Keyboard.Modifiers != (ModifierKeys.Control | ModifierKeys.Shift)) return;
        CopyTranscript(sender, e);
        e.Handled = true;
    }

    private void ScrollTranscriptToEnd()
    {
        if (_disposed || !IsVisible || _scrollToEndQueued) return;
        _scrollToEndQueued = true;
        // Scroll only after the loaded/visible pane has measured its actual viewport.
        Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.ContextIdle, new Action(() =>
        {
            _scrollToEndQueued = false;
            if (!_disposed && IsVisible &&
                TranscriptBox.Template.FindName("TranscriptScrollHost", TranscriptBox) is ScrollViewer scroll)
                scroll.ScrollToEnd();
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
        var snapshot = _history.Snapshot();
        _historySave = SaveAfterAsync(_historySave, snapshot, ++_historySaveVersion);
    }

    private async Task SaveAfterAsync(Task previous, CodexChatHistory snapshot, long version)
    {
        try { await previous.ConfigureAwait(false); }
        catch (Exception) { /* A later snapshot still needs to be saved. */ }
        if (version != Interlocked.Read(ref _historySaveVersion)) return;
        try { await Task.Run(snapshot.Save).ConfigureAwait(false); }
        catch (Exception exception)
        {
            _ = Dispatcher.BeginInvoke(new Action(() =>
            {
                if (!_disposed) StatusText.Text = "대화 저장 실패: " + exception.Message;
            }));
        }
    }

    public void DisposeChat()
    {
        // Stop admitting events, then persist everything already received before closing.
        lock (_notificationGate) _notificationsClosed = true;
        while (_notifications.TryDequeue(out var pending))
        {
            if (!ReferenceEquals(_client, pending.Client)) continue;
            if (pending.Method == "connection/disconnected") HandleDisconnected(pending.Parameters.GetString()!);
            else HandleNotification(pending.Method, pending.Parameters);
        }
        _disposed = true;
        _requestStop?.Cancel();
        _elapsedTimer.Stop();
        _transcriptRenderTimer.Stop();
        _requestClock.Stop();
        SaveHistory();
        _historySave.GetAwaiter().GetResult();
        // No UI callbacks are awaited during transport shutdown.
        try { _client?.DisposeAsync().AsTask().GetAwaiter().GetResult(); }
        catch (Exception) { System.Diagnostics.Debug.WriteLine("AI Chat connection closed during editor shutdown."); }
    }
}
