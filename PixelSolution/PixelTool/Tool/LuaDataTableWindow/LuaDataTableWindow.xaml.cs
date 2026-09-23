using Microsoft.Win32;
using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;

namespace PixelTool
{
    public partial class LuaDataTableWindow : Window
    {
        private static readonly Regex GroupPattern = new("^\\s*\\[\\\"(?<name>(?:\\\\.|[^\\\"])*)\\\"\\]\\s*=\\s*\\{\\s*$");
        private static readonly Regex ValuePattern = new("^\\s*\\[\\\"(?<key>(?:\\\\.|[^\\\"])*)\\\"\\]\\s*=\\s*(?<value>.*),\\s*$");

        public static string[] ValueTypeOptions { get; } = { "String", "Number", "Boolean" };
        public ObservableCollection<LuaDataRow> Rows { get; } = new();
        public LuaDataRow? SelectedRow { get; set; }

        public LuaDataTableWindow()
        {
            InitializeComponent();
            DataContext = this;
            Rows.Add(new LuaDataRow());
        }

        private static string TableDirectory => Path.Combine(ProjectPathService.AssetPath, "Table");

        private void AddRow_Click(object sender, RoutedEventArgs e)
        {
            var row = new LuaDataRow();
            int index = SelectedRow == null ? Rows.Count : Rows.IndexOf(SelectedRow) + 1;
            Rows.Insert(index, row);
            TableGrid.SelectedItem = row;
            TableGrid.ScrollIntoView(row);
        }

        private void DeleteRow_Click(object sender, RoutedEventArgs e)
        {
            if (SelectedRow != null) Rows.Remove(SelectedRow);
        }

        private void MoveUp_Click(object sender, RoutedEventArgs e) => MoveSelectedRow(-1);
        private void MoveDown_Click(object sender, RoutedEventArgs e) => MoveSelectedRow(1);

        private void MoveSelectedRow(int offset)
        {
            if (SelectedRow == null) return;
            int oldIndex = Rows.IndexOf(SelectedRow);
            int newIndex = oldIndex + offset;
            if (newIndex < 0 || newIndex >= Rows.Count) return;
            Rows.Move(oldIndex, newIndex);
            TableGrid.SelectedItem = SelectedRow;
        }

        private void NewFile_Click(object sender, RoutedEventArgs e)
        {
            Rows.Clear();
            Rows.Add(new LuaDataRow());
            FileNameTextBox.Clear();
        }

        private void LoadFile_Click(object sender, RoutedEventArgs e)
        {
            Directory.CreateDirectory(TableDirectory);
            var dialog = new OpenFileDialog
            {
                InitialDirectory = TableDirectory,
                Filter = "Lua data table (*.lua)|*.lua",
                Multiselect = false
            };
            if (dialog.ShowDialog(this) != true) return;

            try
            {
                LoadRows(dialog.FileName);
                FileNameTextBox.Text = Path.GetFileNameWithoutExtension(dialog.FileName);
            }
            catch (Exception exception)
            {
                PixelMessageBox.Show(exception.Message, "Lua Data Table", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void SaveFile_Click(object sender, RoutedEventArgs e)
        {
            TableGrid.CommitEdit();
            TableGrid.CommitEdit();

            string fileName = Path.GetFileNameWithoutExtension(FileNameTextBox.Text.Trim());
            if (string.IsNullOrWhiteSpace(fileName))
            {
                PixelMessageBox.Show("Enter a file name.", "Lua Data Table", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            try
            {
                ValidateRows();
                Directory.CreateDirectory(TableDirectory);
                string filePath = Path.Combine(TableDirectory, fileName + ".lua");
                File.WriteAllText(filePath, BuildLua(), new UTF8Encoding(false));
                FileNameTextBox.Text = fileName;
                GlobalFunction.GetDockedWindow<AssetWindow>()?.Refresh();
                PixelMessageBox.Show($"Saved to Asset/Table/{fileName}.lua", "Lua Data Table");
            }
            catch (Exception exception)
            {
                PixelMessageBox.Show(exception.Message, "Lua Data Table", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void ValidateRows()
        {
            if (Rows.Count == 0) throw new InvalidDataException("Add at least one row.");
            var keys = new HashSet<string>(StringComparer.Ordinal);

            foreach (LuaDataRow row in Rows)
            {
                row.Group = row.Group.Trim();
                row.Key = row.Key.Trim();
                if (row.Group.Length == 0 || row.Key.Length == 0) throw new InvalidDataException("Group and Key cannot be empty.");
                if (!keys.Add(row.Group + "\0" + row.Key)) throw new InvalidDataException($"Duplicate entry: {row.Group} / {row.Key}");

                if (row.Type == "Number" && !double.TryParse(row.Value, NumberStyles.Float, CultureInfo.InvariantCulture, out _))
                    throw new InvalidDataException($"{row.Group} / {row.Key} is not a valid number.");
                if (row.Type == "Boolean" && !bool.TryParse(row.Value, out _))
                    throw new InvalidDataException($"{row.Group} / {row.Key} must be true or false.");
            }
        }

        private string BuildLua()
        {
            var builder = new StringBuilder();
            builder.AppendLine("-- PixelEngine Data Table");
            builder.AppendLine("return {");

            foreach (IGrouping<string, LuaDataRow> group in Rows.GroupBy(row => row.Group))
            {
                builder.AppendLine($"    [\"{Escape(group.Key)}\"] = {{");
                foreach (LuaDataRow row in group)
                    builder.AppendLine($"        [\"{Escape(row.Key)}\"] = {FormatValue(row)},");
                builder.AppendLine("    },");
            }

            builder.AppendLine("}");
            return builder.ToString();
        }

        private static string FormatValue(LuaDataRow row) => row.Type switch
        {
            "Number" => double.Parse(row.Value, CultureInfo.InvariantCulture).ToString("R", CultureInfo.InvariantCulture),
            "Boolean" => bool.Parse(row.Value) ? "true" : "false",
            _ => $"\"{Escape(row.Value)}\""
        };

        private void LoadRows(string filePath)
        {
            Rows.Clear();
            string? currentGroup = null;

            foreach (string line in File.ReadLines(filePath))
            {
                Match groupMatch = GroupPattern.Match(line);
                if (groupMatch.Success)
                {
                    currentGroup = Unescape(groupMatch.Groups["name"].Value);
                    continue;
                }

                if (currentGroup == null) continue;
                Match valueMatch = ValuePattern.Match(line);
                if (!valueMatch.Success) continue;

                string rawValue = valueMatch.Groups["value"].Value.Trim();
                Rows.Add(ParseRow(currentGroup, Unescape(valueMatch.Groups["key"].Value), rawValue));
            }

            if (Rows.Count == 0) throw new InvalidDataException("This file does not contain a supported PixelEngine data table.");
        }

        private static LuaDataRow ParseRow(string group, string key, string rawValue)
        {
            if (rawValue.Length >= 2 && rawValue[0] == '"' && rawValue[^1] == '"')
                return new LuaDataRow { Group = group, Key = key, Type = "String", Value = Unescape(rawValue[1..^1]) };
            if (rawValue is "true" or "false")
                return new LuaDataRow { Group = group, Key = key, Type = "Boolean", Value = rawValue };
            if (double.TryParse(rawValue, NumberStyles.Float, CultureInfo.InvariantCulture, out _))
                return new LuaDataRow { Group = group, Key = key, Type = "Number", Value = rawValue };
            throw new InvalidDataException($"Unsupported value at {group} / {key}: {rawValue}");
        }

        private static string Escape(string value) => value.Replace("\\", "\\\\").Replace("\"", "\\\"").Replace("\r", "\\r").Replace("\n", "\\n");
        private static string Unescape(string value) => Regex.Unescape(value);
    }

    public sealed class LuaDataRow
    {
        public string Group { get; set; } = "Group";
        public string Key { get; set; } = "Key";
        public string Type { get; set; } = "String";
        public string Value { get; set; } = string.Empty;
    }
}
