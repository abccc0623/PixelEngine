using ICSharpCode.AvalonEdit.Rendering;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace PixelTool
{
    public class LuaDarkColorizer : DocumentColorizingTransformer
    {
        JsonDataManager jsonData;
        public LuaDarkColorizer(JsonDataManager apiManager)
        {
            jsonData = apiManager;
        }

        protected override void ColorizeLine(ICSharpCode.AvalonEdit.Document.DocumentLine line)
        {
            int lineStartOffset = line.Offset;
            string text = CurrentContext.Document.GetText(line);

            // 1. 키워드 (파란색 #569CD6)
            string[] keywords = { "local", "function", "end", "if", "then", "else", "return", "print", "self" };
            foreach (string keyword in keywords)
            {
                // 단어 단위로 정확히 찾기 위해 정규식(\b) 사용
                foreach (Match m in Regex.Matches(text, @"\b" + keyword + @"\b"))
                {
                    ChangeLinePart(
                        lineStartOffset + m.Index,
                        lineStartOffset + m.Index + m.Length,
                        (VisualLineElement element) =>
                        {
                            element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(86, 156, 214)));
                            element.TextRunProperties.SetTypeface(new Typeface(element.TextRunProperties.Typeface.FontFamily, FontStyles.Normal, FontWeights.Bold, FontStretches.Normal));
                        });
                }
            }


            if (jsonData?.LoadedApi?.Types != null)
            {
                foreach (var type in jsonData.LoadedApi.Types)
                {
                    // 엔진에 등록된 모든 클래스/네임스페이스 이름을 찾습니다.
                    foreach (Match m in Regex.Matches(text, @"\b" + type.Name + @"\b"))
                    {

                        ChangeLinePart(
                    lineStartOffset + m.Index,
                    lineStartOffset + m.Index + m.Length,
                    (VisualLineElement element) =>
                    {
                        element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(78, 201, 176)));
                        element.TextRunProperties.SetTypeface(new Typeface(element.TextRunProperties.Typeface.FontFamily, FontStyles.Normal, FontWeights.Bold, FontStretches.Normal));
                    });
                    }
                }
            }


            // 2. 문자열 (" " 또는 ' ') (주황색 #CE9178)
            foreach (Match m in Regex.Matches(text, "\".*?\"|'.*?'"))
            {
                ChangeLinePart(
                    lineStartOffset + m.Index,
                    lineStartOffset + m.Index + m.Length,
                    (VisualLineElement element) =>
                    {
                        element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(206, 145, 120)));
                    });
            }

            // 3. 주석 (--) (초록색 #6A9955)
            int commentIndex = text.IndexOf("--");
            if (commentIndex >= 0)
            {
                ChangeLinePart(
                    lineStartOffset + commentIndex,
                    lineStartOffset + text.Length,
                    (VisualLineElement element) =>
                    {
                        element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(106, 153, 85)));
                    });
            }
        }
    }

}
