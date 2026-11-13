using System.Windows;
using System.Windows.Controls;

namespace PixelTool;

public partial class WikiWindow : UserControl
{
    private CancellationTokenSource? search;
    private CancellationTokenSource? preview;
    private int revision;
    public WikiWindow()
    {
        InitializeComponent();
        Unloaded += (_, _) => { revision++; search?.Cancel(); preview?.Cancel(); };
        Loaded += (_, _) => QueueSearch(false);
    }
    private void Query_Changed(object sender, TextChangedEventArgs e) { if (IsLoaded) QueueSearch(true); }
    private void Refresh_Click(object sender, RoutedEventArgs e) => QueueSearch(false);
    private async void QueueSearch(bool debounce)
    {
        int current = ++revision;
        search?.Cancel();
        var source = new CancellationTokenSource();
        search = source;
        string query = Query.Text.Trim();
        preview?.Cancel(); Results.ItemsSource = null; Preview.Clear();
        if (query.Length == 0) { Status.Text = "검색어를 입력하세요. 공통 문서 · 현재 프로젝트 일감"; source.Dispose(); search = null; return; }
        Status.Text = "검색 중…";
        try
        {
            if (debounce) await Task.Delay(200, source.Token);
            var result = await Task.Run(() =>
            {
                var common = WikiSearchService.Shared.Search(query, 100, source.Token);
                if (string.IsNullOrWhiteSpace(ProjectPathService.ProjectRootPath)) return common;
                var project = new ProjectWorkItemWiki(ProjectPathService.ProjectRootPath).Search(query, 100, source.Token);
                return new WikiSearchService.Result(common.Matches.Concat(project.Matches)
                    .OrderByDescending(h => h.Score).ThenBy(h => h.Document, StringComparer.Ordinal).Take(100).ToArray(),
                    common.ElapsedMs + project.ElapsedMs, common.Documents + project.Documents,
                    common.Truncated || project.Truncated, common.Warnings.Concat(project.Warnings).ToArray());
            }, source.Token);
            if (current != revision || !IsLoaded) return;
            Results.ItemsSource = result.Matches;
            Status.Text = $"검색 결과 {result.Matches.Length}개 · {result.ElapsedMs:F1} ms · 문서 {result.Documents}개" +
                (result.Truncated ? " · 표시/검색 한도 초과" : "") +
                (result.Warnings.Length > 0 ? " · " + string.Join(" / ", result.Warnings) : "");
            Status.ToolTip = "파일 목록 확인, 변경 문서 읽기, 검색 및 정렬 시간입니다. 입력 대기와 UI 표시 시간은 제외합니다.";
        }
        catch (OperationCanceledException) { }
        catch (Exception ex) { if (current == revision) Status.Text = "검색 실패: " + ex.Message; }
        finally { if (ReferenceEquals(search, source)) search = null; source.Dispose(); }
    }
    private async void Result_Selected(object sender, SelectionChangedEventArgs e)
    {
        preview?.Cancel();
        if (Results.SelectedItem is not WikiSearchService.Hit hit) return;
        var source = new CancellationTokenSource(); preview = source;
        Preview.Text = "문서 읽는 중…";
        try
        {
            string text = await Task.Run(() => hit.Document.StartsWith("project-wiki/", StringComparison.Ordinal)
                ? new ProjectWorkItemWiki(ProjectPathService.ProjectRootPath).Read(hit.Document)
                : WikiSearchService.Shared.Read(hit.Document), source.Token);
            if (text.Length > 24000)
            {
                int end = char.IsHighSurrogate(text[23999]) ? 23999 : 24000;
                text = text[..end] + "\n\n[미리보기는 앞 24,000자까지만 표시합니다.]";
            }
            if (!source.IsCancellationRequested && IsLoaded && ReferenceEquals(Results.SelectedItem, hit))
                Preview.Text = $"{hit.Document} (검색 위치: {hit.Line}행)\n\n{text}";
        }
        catch (OperationCanceledException) { }
        catch (Exception ex) { if (!source.IsCancellationRequested) Preview.Text = "문서 읽기 실패: " + ex.Message; }
        finally { if (ReferenceEquals(preview, source)) preview = null; source.Dispose(); }
    }
}
