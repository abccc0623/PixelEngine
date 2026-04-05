using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PixelTool
{
    public static class LuaFileManager
    {
        public static string GetFunction(string FileName, string FunctionName)
        {
            string resourceName = $"PixelTool.LuaCode.{FileName}.lua";

            using (var stream = System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
            {
                if (stream == null) return "";

                using (var reader = new System.IO.StreamReader(stream))
                {
                    // 3. 파일의 전체 내용을 읽어옵니다.
                    string fullCode = reader.ReadToEnd();

                    // 4. 전체 코드에서 원하는 함수 부분만 추출합니다.
                    string extractedCode = ExtractFunctionBlock(fullCode, FunctionName);
                    return extractedCode;
                }
            }
        }

        public static string GetFileContent(string FileName)
        {
            string resourceName = $"PixelTool.LuaCode.{FileName}.lua";

            using (var stream = System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
            {
                if (stream == null) return "";

                using (var reader = new System.IO.StreamReader(stream, System.Text.Encoding.UTF8, true))
                {
                    return reader.ReadToEnd();
                }
            }
        }


        static string ExtractFunctionBlock(string fullCode, string functionName)
        {
            string startKeyword = $"function self:{functionName}";
            int startIndex = fullCode.IndexOf(startKeyword);

            // 해당 함수가 없으면 빈 문자열 반환
            if (startIndex == -1) return "";

            // 현재 함수 내용이 끝나는 지점을 찾기 위해, '다음 함수'가 시작되는 위치를 찾습니다.
            int nextFunctionIndex = fullCode.IndexOf("function self:", startIndex + startKeyword.Length);

            if (nextFunctionIndex != -1)
            {
                // 다음 함수가 존재한다면, 현재 함수 시작점부터 다음 함수 시작점 전까지 잘라냅니다.
                return fullCode.Substring(startIndex, nextFunctionIndex - startIndex).TrimEnd();
            }
            else
            {
                // 다음 함수가 없다면 (즉, 파일의 마지막 함수라면) 끝까지 잘라냅니다.
                return fullCode.Substring(startIndex).TrimEnd();
            }
        }
    }
}
