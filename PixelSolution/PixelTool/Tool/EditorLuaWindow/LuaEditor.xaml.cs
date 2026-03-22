using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구
using PixelTool;
using System;
using System.IO;
using System.Text.RegularExpressions; // 정규식 사용
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;           // 색상 사용

namespace PixelTool
{
    public partial class LuaEditorWindow : UserControl
    {
        private string targetPath = "";
        private bool IsDirty = false;

        public static LuaEditorWindow Instance { get; private set; }
        private Dictionary<string, string> variableTypes = new Dictionary<string, string>();
        private CompletionWindow completionWindow;
        private JsonDataManager apiManager;

        public LuaEditorWindow()
        {
            InitializeComponent();
            Instance = this;
            apiManager = new JsonDataManager();
            apiManager.LoadEmbeddedJson();

            this.Loaded += (s, e) =>
            {
                // 1. 현재 UserControl을 담고 있는 진짜 부모 Window를 찾음
                Window parentWindow = Window.GetWindow(this);

                if (parentWindow != null)
                {
                    // 2. 윈도우가 포커스를 얻었을 때 (엔진 창 클릭 시) -> 입력 가능
                    parentWindow.Activated += (sender, args) => LuaEditor.IsReadOnly = false;
                    // 3. 윈도우가 포커스를 잃었을 때 (바탕화면이나 다른 창 클릭 시) -> 입력 차단
                    parentWindow.Deactivated += (sender, args) => LuaEditor.IsReadOnly = true;

                }
            };

            if (LuaEditor != null)
            {
                // 1. 기존 XML 하이라이팅을 완전히 끕니다. (충돌 방지)
                LuaEditor.SyntaxHighlighting = null;
                // 2. [핵심] 우리가 C#으로 만든 "직접 색칠하는 페인터"를 에디터에 장착합니다.
                LuaEditor.TextArea.TextView.LineTransformers.Add(new LuaDarkColorizer(apiManager));

                var options = LuaEditor.Options;
                options.ConvertTabsToSpaces = true;

                // 테스트용 텍스트
                LuaEditor.Text = "-- Welcome to Pixel Engine -- \n";
                LuaEditor.Text += "Click 'main.lua' to add game logic.";

                LuaEditor.TextArea.TextEntering += TextArea_TextEntering;
                LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
                LuaEditor.TextChanged += TextArea_TextChanged;
            }
        }
        private void TextArea_TextChanged(object sender, EventArgs e)
        {
            IsDirty = true;
            EditorChange.Foreground = Brushes.Red;
        }

        private void UpdateVariableTypes()
        {
            variableTypes.Clear();
            string scriptText = LuaEditor.Text;
            variableTypes["gameobject"] = "GameObject";
            variableTypes["transform"] = "Transform";
            variableTypes["self.transform"] = "Transform";
            variableTypes["self.gameobject"] = "GameObject";
            // 규칙 A: Engine.CreateGameObject(...) 로 생성된 변수는 무조건 "GameObject" 타입이다!
            // (매칭 예시: local obj = Engine.CreateGameObject("Obj"))
            string createObjPattern = @"local\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*Engine\.CreateGameObject";
            foreach (Match m in Regex.Matches(scriptText, createObjPattern))
            {
                string varName = m.Groups[1].Value; // "obj"
                variableTypes[varName] = "GameObject"; // 사전에 등록!
            }

            // 규칙 B: AddModule("클래스명") 으로 생성된 변수는 괄호 안의 문자열 타입이다!
            // (매칭 예시: local renderer = obj:AddModule("Renderer2D"))
            string addModulePattern = @"local\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*.*?:(?:AddModule|GetModule)\s*\(\s*""([^""]+)""\s*\)";
            foreach (Match m in Regex.Matches(scriptText, addModulePattern))
            {
                string varName = m.Groups[1].Value;   // "renderer"
                string className = m.Groups[2].Value; // "Renderer2D"
                variableTypes[varName] = className;   // 사전에 등록!
            }
        }


        public void OpenFile(string path)
        {
            if (System.IO.File.Exists(path))
            {
                if(IsDirty == true)
                {
                    var result = MessageBox.Show("파일을 변경전 저장이 필요합니다.\n","저장",MessageBoxButton.YesNo,MessageBoxImage.Warning);
                    if (result == MessageBoxResult.Yes)
                    {
                        LuaEditor.Save(targetPath);
                    }
                }

                LuaEditor.Load(path);
                targetPath = path;
                LuaEditor.Document.UndoStack.MarkAsOriginalFile();
                IsDirty = false;
                EditorChange.Foreground = Brushes.Green;
            }
        }


        private void luaEditor_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            // 에디터에 포커스가 없으면 모든 키 입력 이벤트를 처리된 것으로 간주(Handled)하여 무시
            if (!LuaEditor.IsKeyboardFocusWithin){e.Handled = true;}

            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;

            if (isCtrlPressed)
            {
                switch (e.Key)
                {
                    case Key.S: // 저장만
                        e.Handled = true;
                        LuaEditor.Save(targetPath);
                        EditorChange.Foreground = Brushes.Green;
                        break;
                    case Key.R: // 저장 + 리로드 (ReImport)
                        e.Handled = true;
                        PixelEngineNative.Reload();
                        break;
                }
            }
        }


        private void TextArea_TextEntering(object sender, System.Windows.Input.TextCompositionEventArgs e)
        {
            if (e.Text.Length > 0 && completionWindow != null)
            {
                // 글자가 아닌 기호나 띄어쓰기를 치면 자동완성 창을 닫습니다.
                if (!char.IsLetterOrDigit(e.Text[0]))
                {
                    completionWindow.CompletionList.RequestInsertion(e);
                }
            }
        }

        private void TextArea_TextEntered(object sender, System.Windows.Input.TextCompositionEventArgs e)
        {
            if (e.Text == "." || e.Text == ":")
            {
                if (completionWindow != null || apiManager.LoadedApi == null) return;

                // 1. 전체 경로를 가져옵니다 (예: "transform.Position")
                string fullPath = GetFullPathBeforeCursor(LuaEditor.TextArea);
                if (string.IsNullOrEmpty(fullPath)) return;

                UpdateVariableTypes();

                // 2. 체인을 분석하여 최종 클래스 타입을 찾아냅니다.
                string targetClassName = ResolveTargetClassName(fullPath);

                var targetType = apiManager.LoadedApi.Types.Find(t => t.Name == targetClassName);
                if (targetType == null) return;

                // 자동완성 창 준비
                completionWindow = new CompletionWindow(LuaEditor.TextArea);
                IList<ICompletionData> data = completionWindow.CompletionList.CompletionData;

                if (e.Text == ".")
                {
                    foreach (var field in targetType.Fields)
                    {
                        // 꿀팁: field.TypeName을 표시해줘야 개발자가 편합니다.
                        data.Add(new LuaCompletionData(field.Name, "Field: " + field.Type));
                    }
                    foreach (var func in targetType.Functions)
                    {
                        data.Add(new LuaCompletionData(func.Name, func.Description));
                    }
                }
                else if (e.Text == ":")
                {
                    foreach (var func in targetType.Functions)
                    {
                        data.Add(new LuaCompletionData(func.Name, func.Description));
                    }
                }

                if (data.Count > 0)
                {
                    completionWindow.Show();
                    completionWindow.Closed += delegate { completionWindow = null; };
                }
                else
                {
                    completionWindow = null;
                }
            }
        }

        private string GetFullPathBeforeCursor(ICSharpCode.AvalonEdit.Editing.TextArea textArea)
        {
            var line = textArea.Document.GetLineByOffset(textArea.Caret.Offset);
            string lineText = textArea.Document.GetText(line.Offset, textArea.Caret.Offset - line.Offset);

            // 정규식: 단어, 점, 콜론이 연결된 마지막 부분을 찾음
            var match = System.Text.RegularExpressions.Regex.Match(lineText, @"([a-zA-Z0-9_.:]+)[.:]$");
            if (match.Success)
            {
                // 마지막 기호(. 또는 :)는 떼고 반환
                return match.Groups[1].Value;
            }
            return "";
        }

        private string ResolveTargetClassName(string path)
        {
            string[] parts = path.Split(new char[] { '.', ':' }, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 0) return "";

            // 시작점 찾기 (변수 목록에서 transform이 Transform 타입임을 확인)
            string currentTypeName = "";
            if (variableTypes.ContainsKey(parts[0]))
                currentTypeName = variableTypes[parts[0]];
            else
                currentTypeName = parts[0]; // 변수가 아니면 클래스 이름이라 가정

            // 나머지 조각들 추적 (.Position 등)
            for (int i = 1; i < parts.Length; i++)
            {
                var typeInfo = apiManager.LoadedApi.Types.Find(t => t.Name == currentTypeName);
                if (typeInfo == null) break;

                var field = typeInfo.Fields.Find(f => f.Name == parts[i]);
                if (field != null)
                {
                    currentTypeName = field.Type; // 여기서 PVector3로 갱신됨
                }
            }

            return currentTypeName;
        }

        private void SaveLuaFile(object sender, RoutedEventArgs e)
        {
            LuaEditor.Save(targetPath);
        }

        private void ReimportLuaFile(object sender, RoutedEventArgs e)
        {
            PixelEngineNative.Reload();
        }
    }
}
