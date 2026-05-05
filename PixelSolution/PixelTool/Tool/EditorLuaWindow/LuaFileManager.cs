using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
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
            string startKeyword = $"function Script:{functionName}";
            int startIndex = fullCode.IndexOf(startKeyword);

            // 해당 함수가 없으면 빈 문자열 반환
            if (startIndex == -1) return "";

            // 현재 함수 내용이 끝나는 지점을 찾기 위해, '다음 함수'가 시작되는 위치를 찾습니다.
            int nextFunctionIndex = fullCode.IndexOf("function Script:", startIndex + startKeyword.Length);

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

        static public void  CreateLuaByJIT(string resourcePath, string exportPath)
        {
            try
            {
                // 1. 디렉토리가 없으면 생성
                string directory = Path.GetDirectoryName(exportPath);
                if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }

                // 2. 어셈블리에서 리소스 스트림 가져오기
                var assembly = Assembly.GetExecutingAssembly();
                using (Stream stream = assembly.GetManifestResourceStream(resourcePath))
                {
                    if (stream == null)
                    {
                        Console.WriteLine($"[Error] 리소스를 찾을 수 없습니다: {resourcePath}");
                        return;
                    }

                    // 3. 파일 스트림으로 복사 (가장 효율적인 방식)
                    using (FileStream fileStream = new FileStream(exportPath, FileMode.Create, FileAccess.Write))
                    {
                        stream.CopyTo(fileStream);
                    }

                    Console.WriteLine($"[Success] 루아 파일 생성 완료: {exportPath}");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Exception] 파일 생성 중 오류 발생: {ex.Message}");
            }
        }

        public static string GetBlockByMarker(string FileName, string MarkerName)
        {
            string resourceName = $"PixelTool.LuaCode.{FileName}.lua";
            string beginMarker = $"--#BEGIN_TABLE {MarkerName}";
            string endMarker = $"--#END_TABLE {MarkerName}";
            using (var stream = System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
            {
                if (stream == null) return "";

                using (var reader = new System.IO.StreamReader(stream, System.Text.Encoding.UTF8, true))
                {
                    string fullCode = reader.ReadToEnd();

                    // 1. 시작 마커 위치 검색
                    int startIndex = fullCode.IndexOf(beginMarker);
                    if (startIndex == -1) return "";

                    // 시작 마커가 끝나는 다음 줄(개행 문자 다음)부터 추출 시작
                    startIndex += beginMarker.Length;

                    // 2. 종료 마커 위치 검색
                    int endIndex = fullCode.IndexOf(endMarker, startIndex);
                    if (endIndex == -1) return "";

                    // 3. 내용 추출 및 앞뒤 공백(개행) 제거
                    return fullCode.Substring(startIndex, endIndex - startIndex).Trim();
                }
            }
        }
    }
}
