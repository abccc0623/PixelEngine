using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Xml.Linq;

namespace PixelTool;

// The layout holds IDs, never scripts, engine objects or copies of a tool's content.
internal sealed class DockNode
{
    public List<string> Tabs { get; set; } = new();
    public string? Selected { get; set; }
    public DockNode? First { get; set; }
    public DockNode? Second { get; set; }
    public bool Horizontal { get; set; }
    public double Ratio { get; set; } = .5;
    [JsonIgnore] public Grid? View { get; set; }
    [JsonIgnore] public ContentControl? Presenter { get; set; }
    public DockNode() { }
    public static DockNode Pane(params string[] ids) => new() { Tabs = ids.ToList(), Selected = ids.FirstOrDefault() };
    public static DockNode Split(DockNode a, DockNode b, bool horizontal, double ratio = .5)
        => new() { First = a, Second = b, Horizontal = horizontal, Ratio = ratio };
}

internal sealed class DockSavedWindow
{
    public DockNode Root { get; set; } = new();
    public double Left { get; set; }
    public double Top { get; set; }
    public double Width { get; set; } = 640;
    public double Height { get; set; } = 480;
}
internal sealed class DockSavedLayout
{
    public int Version { get; set; } = 1;
    public DockNode Main { get; set; } = new();
    public List<DockSavedWindow> Floating { get; set; } = new();
}

public sealed class DockWorkspace : Grid, IDisposable
{
    private sealed record Document(string Id, string Title, FrameworkElement Content, bool Persistent);
    private sealed class Host(Window window, Grid surface, DockNode root, bool floating)
    {
        public Window Window = window;
        public Grid Surface = surface;
        public DockNode Root = root;
        public bool Floating = floating;
        public bool SuppressReturn, NativeMoving;
    }
    private readonly Dictionary<string, Document> documents = new();
    private readonly List<Host> hosts = new();
    private readonly Border hint = new()
    {
        Background = new SolidColorBrush(Color.FromArgb(75, 70, 160, 235)),
        BorderBrush = Brushes.DeepSkyBlue, BorderThickness = new Thickness(2), IsHitTestVisible = false
    };
    // A Popup can draw over HwndHost/DirectX; a WPF child overlay cannot.
    private readonly Popup preview = new() { AllowsTransparency = true, StaysOpen = true, IsHitTestVisible = false, Focusable = false };
    private Host? main, sourceHost, targetHost;
    private DockNode? sourcePane, targetPane;
    private string? dragId;
    private Point dragStart;
    private bool dragging, stopping;
    private string edge = "Tab";
    private string activeId = "SceneView";

    public DockWorkspace()
    {
        preview.Child = hint;
        SetResourceReference(BackgroundProperty, "PixelBackgroundBrush");
    }

    public IEnumerable<FrameworkElement> Contents => documents.Values.Select(d => d.Content);
    public void Register(string id, string title, FrameworkElement content, bool persistent = true)
        => documents.Add(id, new Document(id, title, content, persistent));

    public void Initialize(Window owner, string path, string legacyPath)
    {
        main = new Host(owner, this, DefaultLayout(), false);
        Attach(main);
        try
        {
            DockSavedLayout? saved = File.Exists(path)
                ? JsonSerializer.Deserialize<DockSavedLayout>(File.ReadAllText(path))
                : File.Exists(legacyPath) ? ReadLegacy(legacyPath) : null;
            if (saved is { Version: 1 })
            {
                var seen = new HashSet<string>();
                main.Root = Validate(saved.Main, seen, 0) ?? DockNode.Pane();
                foreach (var state in (saved.Floating ?? new()).Take(16))
                {
                    var root = Validate(state.Root, seen, 0);
                    if (root != null) CreateFloating(root, state, show: false);
                }
            }
        }
        catch (Exception ex) when (ex is IOException or JsonException or System.Xml.XmlException or InvalidOperationException or UnauthorizedAccessException)
        {
            Debug.WriteLine("Dock layout restore: " + ex.Message);
            main.Root = DefaultLayout();
        }
        if (!hosts.SelectMany(h => Panes(h.Root)).Any(p => p.Tabs.Count > 0)) main.Root = DefaultLayout();
        Render();
        foreach (var host in hosts.Where(h => h.Floating).ToArray()) host.Window.Show();
    }

    private static DockNode DefaultLayout() => DockNode.Split(
        DockNode.Split(DockNode.Pane("SceneView"),
            DockNode.Split(DockNode.Pane("LuaEditor"), DockNode.Pane("AIChat"), true, .6), true),
        DockNode.Split(DockNode.Pane("Asset"), DockNode.Pane("Log"), true, .7), false, .75);

    public void ResetLayout()
    {
        if (main == null) return;
        DetachContents();
        foreach (var host in hosts.Where(h => h.Floating).ToArray()) CloseHost(host);
        DetachContents();
        main.Root = DefaultLayout();
        Render();
    }

    public void ShowDocument(string id, bool activate = true)
    {
        if (main == null || !documents.ContainsKey(id)) return;
        foreach (var host in hosts)
        {
            var pane = Panes(host.Root).FirstOrDefault(p => p.Tabs.Contains(id));
            if (pane == null) continue;
            Select(host, pane, id); if (activate) host.Window.Activate(); return;
        }
        var lua = Panes(main.Root).FirstOrDefault(p => p.Tabs.Contains("LuaEditor"));
        if (id == "AIChat" && lua != null)
            main.Root = Replace(main.Root, lua, DockNode.Split(lua, DockNode.Pane(id), true, .6));
        else Panes(main.Root).First().Tabs.Add(id);
        Render();
        var destination = Panes(main.Root).First(p => p.Tabs.Contains(id));
        Select(main, destination, id);
    }

    public Action OpenTool(string title, FrameworkElement content, double width, double height)
    {
        string id = Guid.NewGuid().ToString("N");
        Register(id, title, content, false);
        var host = CreateFloating(DockNode.Pane(id), new DockSavedWindow { Width = width, Height = height }, false);
        Render(); host.Window.Show();
        return () => HideDocument(id);
    }

    private void HideDocument(string id)
    {
        CancelDrag();
        foreach (var host in hosts)
            foreach (var pane in Panes(host.Root)) pane.Tabs.Remove(id);
        PruneHosts(); Render();
        if (documents.TryGetValue(id, out var document) && !document.Persistent) documents.Remove(id);
    }

    public void SaveLayout(string path)
    {
        if (main == null) return;
        var saved = new DockSavedLayout { Main = Snapshot(main.Root) };
        foreach (var host in hosts.Where(h => h.Floating))
            saved.Floating.Add(new DockSavedWindow { Root = Snapshot(host.Root), Left = host.Window.Left,
                Top = host.Window.Top, Width = host.Window.Width, Height = host.Window.Height });
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        string temporary = path + ".tmp";
        File.WriteAllText(temporary, JsonSerializer.Serialize(saved));
        File.Move(temporary, path, true);
    }

    private DockNode Snapshot(DockNode node)
    {
        if (node.First == null) return new DockNode { Tabs = node.Tabs.Where(id => documents[id].Persistent).ToList(), Selected = node.Selected };
        return DockNode.Split(Snapshot(node.First), Snapshot(node.Second!), node.Horizontal, node.Ratio);
    }

    private DockNode? Validate(DockNode? node, HashSet<string> seen, int depth)
    {
        if (node == null || depth > 32) return null;
        if (node.First != null || node.Second != null)
        {
            var a = Validate(node.First, seen, depth + 1); var b = Validate(node.Second, seen, depth + 1);
            if (a == null) return b; if (b == null) return a;
            return DockNode.Split(a, b, node.Horizontal, double.IsFinite(node.Ratio) ? Math.Clamp(node.Ratio, .1, .9) : .5);
        }
        node.Tabs = (node.Tabs ?? new()).Where(id => id != null && documents.ContainsKey(id) && seen.Add(id)).Take(64).ToList();
        return node.Tabs.Count == 0 ? null : node;
    }

    private static DockSavedLayout ReadLegacy(string path)
    {
        var xml = XDocument.Load(path);
        DockNode? Read(XElement e)
        {
            var ids = e.Elements().Where(c => c.Name.LocalName is "LayoutDocument" or "LayoutAnchorable")
                .Select(c => (string?)c.Attribute("ContentId")).Where(id => !string.IsNullOrEmpty(id)).Cast<string>().ToArray();
            if (ids.Length > 0)
            {
                var pane = DockNode.Pane(ids);
                if (int.TryParse((string?)e.Attribute("SelectedContentIndex"), out int index) && index >= 0 && index < ids.Length) pane.Selected = ids[index];
                return pane;
            }
            bool horizontal = (string?)e.Attribute("Orientation") != "Vertical";
            var children = e.Elements().Select(child => (Element: child, Node: Read(child))).Where(child => child.Node != null).ToList();
            if (children.Count == 0) return null;
            double Weight(XElement child)
            {
                string value = ((string?)child.Attribute(horizontal ? "DockWidth" : "DockHeight") ?? "1*").TrimEnd('*');
                return double.TryParse(value, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out double weight)
                    && double.IsFinite(weight) && weight > 0 ? weight : 1;
            }
            var result = children[0].Node!;
            double total = Weight(children[0].Element);
            foreach (var child in children.Skip(1))
            {
                double weight = Weight(child.Element);
                result = DockNode.Split(result, child.Node!, horizontal, total / (total + weight));
                total += weight;
            }
            return result;
        }
        var panel = xml.Descendants().First(e => e.Name.LocalName == "LayoutPanel");
        var result = new DockSavedLayout { Main = Read(panel) ?? DefaultLayout() };
        foreach (var floating in xml.Descendants().Where(e => e.Name.LocalName is "LayoutDocumentFloatingWindow" or "LayoutAnchorableFloatingWindow"))
            if (Read(floating) is { } root) result.Floating.Add(new DockSavedWindow { Root = root });
        return result;
    }

    private void Attach(Host host)
    {
        hosts.Add(host);
        host.Window.PreviewMouseMove += Move;
        host.Window.PreviewMouseLeftButtonUp += Release;
        host.Window.PreviewKeyDown += DockKeyDown;
        host.Surface.LostMouseCapture += LostCapture;
        host.Window.Activated += Activated;
        host.Window.PreviewGotKeyboardFocus += DockFocus;
    }
    private void Activated(object? sender, EventArgs e)
    {
        var host = hosts.FirstOrDefault(h => h.Window == sender);
        if (host != null) { hosts.Remove(host); hosts.Add(host); }
    }
    private Host CreateFloating(DockNode root, DockSavedWindow state, bool show)
    {
        var window = new Window { Owner = main!.Window, Width = double.IsFinite(state.Width) ? Math.Clamp(state.Width, 240, 4000) : 640,
            Height = double.IsFinite(state.Height) ? Math.Clamp(state.Height, 180, 3000) : 480,
            MinWidth = 240, MinHeight = 180, FontFamily = main.Window.FontFamily, WindowStartupLocation = WindowStartupLocation.CenterOwner };
        window.SetResourceReference(Window.BackgroundProperty, "PixelBackgroundBrush");
        window.SetResourceReference(Window.ForegroundProperty, "PixelTextBrush");
        var surface = new Grid(); window.Content = surface;
        var host = new Host(window, surface, root, true);
        Attach(host);
        if (double.IsFinite(state.Left) && double.IsFinite(state.Top) &&
            state.Left + window.Width > SystemParameters.VirtualScreenLeft && state.Left < SystemParameters.VirtualScreenLeft + SystemParameters.VirtualScreenWidth &&
            state.Top >= SystemParameters.VirtualScreenTop && state.Top < SystemParameters.VirtualScreenTop + SystemParameters.VirtualScreenHeight)
        { window.WindowStartupLocation = WindowStartupLocation.Manual; window.Left = state.Left; window.Top = state.Top; }
        window.SourceInitialized += (_, _) => ((HwndSource)PresentationSource.FromVisual(window)).AddHook(
            (IntPtr hwnd, int message, IntPtr w, IntPtr l, ref bool handled) => FloatingMessage(host, message));
        window.Closing += (_, _) =>
        {
            if (!stopping && !host.SuppressReturn)
            {
                DetachContents();
                var items = Panes(host.Root).SelectMany(p => p.Tabs).ToArray();
                Panes(main.Root).First().Tabs.AddRange(items.Where(id => documents[id].Persistent));
                foreach (string id in items.Where(id => !documents[id].Persistent)) documents.Remove(id);
                host.Root = DockNode.Pane();
            }
        };
        window.Closed += (_, _) => { hosts.Remove(host); if (!stopping) Render(); };
        if (show) { Render(); window.Show(); }
        return host;
    }

    private void DetachContents()
    {
        foreach (var pane in hosts.SelectMany(h => Panes(h.Root)))
            if (pane.Presenter != null) pane.Presenter.Content = null;
    }
    private void Render()
    {
        DetachContents();
        foreach (var host in hosts)
        {
            host.Surface.Children.Clear(); host.Surface.Children.Add(Build(host, host.Root));
            if (host.Floating) host.Window.Title = string.Join(" / ", Panes(host.Root).SelectMany(p => p.Tabs).Select(id => documents[id].Title));
        }
    }

    private FrameworkElement Build(Host host, DockNode node)
    {
        var grid = new Grid(); node.View = grid;
        if (node.First == null)
        {
            grid.Margin = new Thickness(1);
            grid.RowDefinitions.Add(new() { Height = GridLength.Auto }); grid.RowDefinitions.Add(new());
            var bar = new StackPanel { Orientation = Orientation.Horizontal };
            var tabs = new ScrollViewer { Content = bar, Margin = new Thickness(0, 0, 22, 0), HorizontalScrollBarVisibility = ScrollBarVisibility.Hidden, VerticalScrollBarVisibility = ScrollBarVisibility.Disabled };
            grid.Children.Add(tabs);
            var more = new Button { Content = "▾", Width = 22, HorizontalAlignment = HorizontalAlignment.Right,
                Background = Brushes.Transparent, BorderThickness = new Thickness(0), Focusable = false };
            more.SetResourceReference(Control.ForegroundProperty, "PixelMutedTextBrush");
            more.Click += (_, _) =>
            {
                var menu = new ContextMenu { PlacementTarget = more, Placement = PlacementMode.Bottom };
                foreach (string id in node.Tabs)
                {
                    var item = new MenuItem { Header = documents[id].Title, IsCheckable = true, IsChecked = node.Selected == id };
                    item.Click += (_, _) => Select(host, node, id);
                    menu.Items.Add(item);
                }
                menu.IsOpen = true;
            };
            grid.Children.Add(more);
            node.Presenter = new ContentControl { HorizontalContentAlignment = HorizontalAlignment.Stretch, VerticalContentAlignment = VerticalAlignment.Stretch };
            Grid.SetRow(node.Presenter, 1); grid.Children.Add(node.Presenter);
            if (!node.Tabs.Contains(node.Selected ?? "")) node.Selected = node.Tabs.FirstOrDefault();
            foreach (string id in node.Tabs)
            {
                var title = new Button { Content = documents[id].Title, FontSize = 12, FontWeight = FontWeights.Medium,
                    Padding = new Thickness(12, 5, 8, 5), Focusable = false, BorderThickness = new Thickness(0) };
                title.Background = Brushes.Transparent;
                title.SetResourceReference(Control.ForegroundProperty, id == activeId ? "PixelAccentBrush" : "PixelMutedTextBrush");
                title.PreviewMouseLeftButtonDown += (_, e) => { sourceHost = host; sourcePane = node; dragId = id; dragStart = e.GetPosition(host.Surface); };
                title.Click += (_, _) => Select(host, node, id);
                var close = new Button { Content = "×", Padding = new Thickness(4, 0, 6, 0), Focusable = false, Background = Brushes.Transparent, BorderThickness = new Thickness(0) };
                close.SetResourceReference(Control.ForegroundProperty, "PixelMutedTextBrush");
                close.Click += (_, _) => HideDocument(id);
                var row = new StackPanel { Orientation = Orientation.Horizontal }; row.Children.Add(title); row.Children.Add(close);
                var tab = new Border { Child = row, MinWidth = 96, Margin = new Thickness(2, 1, 0, 0), BorderThickness = new Thickness(0, 0, 0, 2), Tag = id };
                tab.SetResourceReference(Border.BorderBrushProperty, id == activeId ? "PixelAccentBrush" : "PixelDockBorderBrush");
                tab.SetResourceReference(Border.BackgroundProperty, id == activeId ? "PixelPanelBrush" : "PixelBackgroundBrush");
                bar.Children.Add(tab);
            }
            if (node.Selected != null) node.Presenter.Content = documents[node.Selected].Content;
            return grid;
        }
        double ratio = Math.Clamp(node.Ratio, .05, .95);
        var splitter = new GridSplitter { ShowsPreview = true, ResizeBehavior = GridResizeBehavior.PreviousAndNext,
            HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };
        splitter.SetResourceReference(Control.BackgroundProperty, "PixelDockBorderBrush");
        var first = Build(host, node.First); var second = Build(host, node.Second!);
        if (node.Horizontal)
        {
            grid.ColumnDefinitions.Add(new() { Width = new GridLength(ratio, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new() { Width = new GridLength(5) });
            grid.ColumnDefinitions.Add(new() { Width = new GridLength(1 - ratio, GridUnitType.Star) });
            Grid.SetColumn(splitter, 1); Grid.SetColumn(second, 2); splitter.ResizeDirection = GridResizeDirection.Columns;
        }
        else
        {
            grid.RowDefinitions.Add(new() { Height = new GridLength(ratio, GridUnitType.Star) });
            grid.RowDefinitions.Add(new() { Height = new GridLength(5) });
            grid.RowDefinitions.Add(new() { Height = new GridLength(1 - ratio, GridUnitType.Star) });
            Grid.SetRow(splitter, 1); Grid.SetRow(second, 2); splitter.ResizeDirection = GridResizeDirection.Rows;
        }
        splitter.DragCompleted += (_, _) =>
        {
            double a = node.Horizontal ? grid.ColumnDefinitions[0].ActualWidth : grid.RowDefinitions[0].ActualHeight;
            double b = node.Horizontal ? grid.ColumnDefinitions[2].ActualWidth : grid.RowDefinitions[2].ActualHeight;
            if (a + b > 0) node.Ratio = a / (a + b);
        };
        grid.Children.Add(first); grid.Children.Add(splitter); grid.Children.Add(second);
        return grid;
    }

    private void Select(Host host, DockNode pane, string id)
    {
        if (pane.Selected != id)
        {
            pane.Selected = id;
            pane.Presenter!.Content = documents[id].Content;
        }
        activeId = id;
        RefreshHeaders();
    }
    private void DockFocus(object sender, KeyboardFocusChangedEventArgs e)
    {
        var focused = documents.Values.FirstOrDefault(d => d.Content.IsKeyboardFocusWithin);
        if (focused == null || focused.Id == activeId) return;
        activeId = focused.Id;
        RefreshHeaders();
    }
    private void RefreshHeaders()
    {
        // Only the header brushes change; selecting a tab never rebuilds the dock tree.
        foreach (var pane in hosts.SelectMany(h => Panes(h.Root)))
        {
            if (pane.View?.Children.Count is not > 0) continue;
            var bar = (StackPanel)((ScrollViewer)pane.View.Children[0]).Content;
            foreach (Border tab in bar.Children)
            {
                bool active = (string)tab.Tag == activeId;
                tab.SetResourceReference(Border.BorderBrushProperty, active ? "PixelAccentBrush" : "PixelDockBorderBrush");
                tab.SetResourceReference(Border.BackgroundProperty, active ? "PixelPanelBrush" : "PixelBackgroundBrush");
                ((Button)((StackPanel)tab.Child).Children[0]).SetResourceReference(Control.ForegroundProperty, active ? "PixelAccentBrush" : "PixelMutedTextBrush");
            }
        }
    }

    private static IEnumerable<DockNode> Panes(DockNode node)
    {
        if (node.First == null) { yield return node; yield break; }
        foreach (var p in Panes(node.First)) yield return p;
        foreach (var p in Panes(node.Second!)) yield return p;
    }
    private static DockNode Replace(DockNode node, DockNode target, DockNode replacement)
    {
        if (ReferenceEquals(node, target)) return replacement;
        if (node.First != null) { node.First = Replace(node.First, target, replacement); node.Second = Replace(node.Second!, target, replacement); }
        return node;
    }
    private static DockNode? Prune(DockNode node)
    {
        if (node.First == null) return node.Tabs.Count == 0 ? null : node;
        var a = Prune(node.First); var b = Prune(node.Second!);
        if (a == null) return b; if (b == null) return a;
        node.First = a; node.Second = b; return node;
    }
    private void PruneHosts()
    {
        // Detach before pruning, otherwise the removed pane still owns its content.
        DetachContents();
        foreach (var host in hosts.ToArray())
        {
            var root = Prune(host.Root);
            host.Root = root ?? DockNode.Pane();
            if (root == null && host.Floating) CloseHost(host);
        }
    }
    private void CloseHost(Host host) { host.SuppressReturn = true; host.Window.Close(); }

    private void Move(object sender, MouseEventArgs e)
    {
        if (dragId == null || sourceHost == null || e.LeftButton != MouseButtonState.Pressed) return;
        var point = e.GetPosition(sourceHost.Surface);
        if (!dragging)
        {
            if ((point - dragStart).Length < 6) return;
            if (!sourceHost.Surface.CaptureMouse()) { CancelDrag(); return; }
            dragging = true; Mouse.OverrideCursor = Cursors.SizeAll;
        }
        FindTarget(sourceHost.Surface.PointToScreen(point), null);
        e.Handled = true;
    }
    private void FindTarget(Point screenPoint, Host? skip)
    {
        targetPane = null; targetHost = null; preview.IsOpen = false;
        foreach (var host in hosts.AsEnumerable().Reverse())
        {
            if (host == skip || !host.Window.IsVisible) continue;
            var point = host.Surface.PointFromScreen(screenPoint);
            foreach (var pane in Panes(host.Root))
            {
                if (pane.View == null || pane.View.ActualWidth < 1 || pane.View.ActualHeight < 1) continue;
                var rect = new Rect(pane.View.TranslatePoint(new Point(), host.Surface), pane.View.RenderSize);
                if (!rect.Contains(point)) continue;
                targetHost = host; targetPane = pane;
                double x = (point.X - rect.X) / rect.Width, y = (point.Y - rect.Y) / rect.Height;
                edge = x < .23 ? "Left" : x > .77 ? "Right" : y < .23 ? "Top" : y > .77 ? "Bottom" : "Tab";
                if (edge == "Left") rect.Width /= 2;
                if (edge == "Right") { rect.X += rect.Width / 2; rect.Width /= 2; }
                if (edge == "Top") rect.Height /= 2;
                if (edge == "Bottom") { rect.Y += rect.Height / 2; rect.Height /= 2; }
                hint.Width = rect.Width; hint.Height = rect.Height;
                preview.Placement = PlacementMode.Relative; preview.PlacementTarget = host.Surface;
                preview.HorizontalOffset = rect.X; preview.VerticalOffset = rect.Y; preview.IsOpen = true;
                return;
            }
        }
    }
    private void Release(object sender, MouseButtonEventArgs e)
    {
        if (!dragging || sourceHost == null || sourcePane == null || dragId == null) { CancelDrag(); return; }
        var from = sourceHost; var pane = sourcePane; var id = dragId;
        var screenPoint = from.Surface.PointToScreen(e.GetPosition(from.Surface));
        FindTarget(screenPoint, null);
        var to = targetHost; var destination = targetPane; string dropEdge = edge;
        if (pane == destination && (dropEdge == "Tab" || pane.Tabs.Count == 1)) { CancelDrag(); return; }
        CancelDrag(); DetachContents();
        pane.Tabs.Remove(id);
        var payload = DockNode.Pane(id);
        if (to != null && destination != null) Insert(to, destination, payload, dropEdge);
        else
        {
            var logical = PresentationSource.FromVisual(from.Window)!.CompositionTarget!.TransformFromDevice.Transform(screenPoint);
            CreateFloating(payload, new DockSavedWindow { Left = logical.X - 100, Top = logical.Y - 35 }, false);
        }
        PruneHosts(); Render();
        foreach (var host in hosts.Where(h => h.Floating && !h.Window.IsVisible).ToArray()) host.Window.Show();
        e.Handled = true;
    }
    private static void Insert(Host host, DockNode destination, DockNode payload, string dropEdge)
    {
        if (dropEdge == "Tab") { destination.Tabs.AddRange(Panes(payload).SelectMany(p => p.Tabs)); destination.Selected = Panes(payload).First().Selected; }
        else
        {
            bool before = dropEdge is "Left" or "Top";
            host.Root = Replace(host.Root, destination, DockNode.Split(before ? payload : destination, before ? destination : payload, dropEdge is "Left" or "Right"));
        }
    }

    [StructLayout(LayoutKind.Sequential)] private struct CursorPoint { public int X, Y; }
    [DllImport("user32.dll")] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetCursorPos(out CursorPoint point);
    private IntPtr FloatingMessage(Host host, int message)
    {
        if (message == 0x0216) // WM_MOVING: resize alone must not trigger docking.
        {
            host.NativeMoving = true; Mouse.OverrideCursor = Cursors.SizeAll;
            if (GetCursorPos(out var point)) FindTarget(new Point(point.X, point.Y), host);
        }
        else if (message == 0x0232 && host.NativeMoving)
        {
            host.NativeMoving = false;
            if (GetCursorPos(out var point)) FindTarget(new Point(point.X, point.Y), host);
            var destinationHost = targetHost; var pane = targetPane; string dropEdge = edge;
            CancelDrag();
            if (destinationHost != null && pane != null)
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    if (stopping || !hosts.Contains(host) || !hosts.Contains(destinationHost) || !Panes(destinationHost.Root).Contains(pane)) return;
                    DetachContents(); var payload = host.Root; host.Root = DockNode.Pane();
                    Insert(destinationHost, pane, payload, dropEdge);
                    PruneHosts(); Render(); destinationHost.Window.Activate();
                }));
        }
        return IntPtr.Zero;
    }
    private void DockKeyDown(object sender, KeyEventArgs e) { if (e.Key == Key.Escape) CancelDrag(); }
    private void LostCapture(object sender, MouseEventArgs e) { if (dragging) CancelDrag(); }
    private void CancelDrag()
    {
        dragging = false; dragId = null; sourcePane = targetPane = null; targetHost = null;
        Mouse.OverrideCursor = null; preview.IsOpen = false;
        var source = sourceHost; sourceHost = null;
        if (source?.Surface.IsMouseCaptured == true) source.Surface.ReleaseMouseCapture();
    }
    public void BeginShutdown() { stopping = true; CancelDrag(); }
    public void Dispose()
    {
        BeginShutdown();
        foreach (var host in hosts.ToArray())
        {
            host.Window.PreviewMouseMove -= Move; host.Window.PreviewMouseLeftButtonUp -= Release;
            host.Window.PreviewKeyDown -= DockKeyDown; host.Surface.LostMouseCapture -= LostCapture; host.Window.Activated -= Activated;
            host.Window.PreviewGotKeyboardFocus -= DockFocus;
            if (host.Floating) CloseHost(host);
        }
        preview.Child = null;
    }
}
