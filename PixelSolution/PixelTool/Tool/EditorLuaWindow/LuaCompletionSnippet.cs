using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace PixelTool
{
    // Parse LSP tab stops without rewriting ordinary Lua expressions or strings.
    internal static class LuaCompletionSnippet
    {
        internal sealed class Part
        {
            public string Text;
            public int Index = -1;
        }

        internal static List<Part> Parse(string source, string detail, string selectedText)
        {
            var parts = new List<Part>();
            int offset = 0;
            var text = new StringBuilder();
            while (offset < source.Length)
            {
                char ch = source[offset++];
                if (ch == '\\' && offset < source.Length && "\\$},|".IndexOf(source[offset]) >= 0)
                {
                    text.Append(source[offset++]);
                    continue;
                }
                if (ch != '$' || offset >= source.Length ||
                    (source[offset] != '{' && !char.IsDigit(source[offset]) && !char.IsLetter(source[offset]) && source[offset] != '_'))
                {
                    text.Append(ch);
                    continue;
                }
                if (text.Length > 0) { parts.Add(new Part { Text = text.ToString() }); text.Clear(); }
                bool braced = source[offset] == '{';
                if (braced) offset++;
                int start = offset;
                bool numeric = offset < source.Length && char.IsDigit(source[offset]);
                while (offset < source.Length && (numeric ? char.IsDigit(source[offset]) : char.IsLetterOrDigit(source[offset]) || source[offset] == '_')) offset++;
                string name = source.Substring(start, offset - start);
                if (name.Length == 0) throw new FormatException("Empty snippet placeholder.");
                string value = numeric ? "" : name == "TM_SELECTED_TEXT" ? selectedText : name;
                if (braced)
                {
                    if (offset < source.Length && source[offset] == ':')
                    {
                        offset++;
                        int bodyStart = offset, depth = 1;
                        while (offset < source.Length && depth > 0)
                        {
                            if (source[offset] == '\\' && offset + 1 < source.Length) { offset += 2; continue; }
                            if (source[offset] == '{') depth++;
                            if (source[offset] == '}') depth--;
                            if (depth > 0) offset++;
                        }
                        if (depth != 0) throw new FormatException("Unclosed snippet placeholder.");
                        value = Flatten(Parse(source.Substring(bodyStart, offset - bodyStart), null, selectedText));
                    }
                    else if (offset < source.Length && source[offset] == '|')
                    {
                        offset++;
                        var choice = new StringBuilder();
                        bool first = true;
                        while (offset < source.Length)
                        {
                            char next = source[offset++];
                            if (next == '\\' && offset < source.Length) { if (first) choice.Append(source[offset]); offset++; continue; }
                            if (next == '|' && offset < source.Length && source[offset] == '}') break;
                            if (next == ',') first = false;
                            else if (first) choice.Append(next);
                        }
                        value = choice.ToString();
                    }
                    if (offset >= source.Length || source[offset] != '}')
                        throw new FormatException("Unsupported or unclosed snippet placeholder.");
                    offset++;
                }
                int index;
                if (numeric && int.TryParse(name,out index))
                    parts.Add(new Part { Index = index, Text = DefaultArgument(value,detail) });
                else
                    parts.Add(new Part { Text = value });
            }
            if (text.Length > 0) parts.Add(new Part { Text = text.ToString() });
            // A forward reference may precede its default: $1 ... ${1:name}.
            var defaults = new Dictionary<int,string>();
            foreach (var part in parts)
                if (part.Index >= 0 && !string.IsNullOrEmpty(part.Text) && !defaults.ContainsKey(part.Index)) defaults[part.Index] = part.Text;
            foreach (var part in parts)
                if (part.Index >= 0 && defaults.ContainsKey(part.Index)) part.Text = defaults[part.Index];
            return parts;
        }

        internal static string Flatten(List<Part> parts)
        {
            var result = new StringBuilder();
            foreach (var part in parts) result.Append(part.Text);
            return result.ToString();
        }

        private static string DefaultArgument(string value, string detail)
        {
            if (!Regex.IsMatch(value,@"^[A-Za-z_][A-Za-z_0-9]*$")) return value;
            string type = value;
            if (!string.IsNullOrEmpty(detail))
            {
                var match = Regex.Match(detail,@"\b" + Regex.Escape(value) + @"\??\s*:\s*(string|number|integer|boolean)\b");
                if (match.Success) type = match.Groups[1].Value;
            }
            switch (Regex.Replace(type,@"\d+$", ""))
            {
                case "string": return "\"\"";
                case "number":
                case "integer": return "0";
                case "boolean": return "false";
                default: return value;
            }
        }
    }
}
