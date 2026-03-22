using System.IO;
using System.Reflection;
using System.Text.Json;

namespace PixelTool
{
    public class EngineApiRoot
    {
        public string ApiVersion { get; set; }
        public List<EngineType> Types { get; set; }
    }

    public class EngineType
    {
        public string Name { get; set; }
        public string Kind { get; set; } // "Class" 또는 "Namespace"
        public List<EngineFunction> Functions { get; set; }
        public List<EngineField> Fields { get; set; }
    }
    public class EngineFunction
    {
        public string Name { get; set; }
        public string Description { get; set; }
    }
    public class EngineField
    {
        public string Name { get; set; }
        public string Type { get; set; }
    }

    public class JsonDataManager
    {
        public EngineApiRoot LoadedApi { get; private set; }

        public void LoadEmbeddedJson()
        {
            // 1. 현재 에디터 프로그램(.exe)의 메모리를 가리킵니다.
            var assembly = Assembly.GetExecutingAssembly();

            // 2. 리소스 이름의 규칙: "프로젝트_기본네임스페이스.폴더명(있다면).파일명.확장자"
            // (예시: 프로젝트 네임스페이스가 HyungSunEditor 이고 최상위 경로에 넣었다면)
            string resourceName = "PixelTool.Generated.GenerateLuaAPI.json";

            // 3. exe 파일 뱃속에서 스트림(Stream) 형태로 데이터를 뽑아냅니다.
            using (Stream stream = assembly.GetManifestResourceStream(resourceName))
            {
                if (stream == null)
                {
                    Console.WriteLine("에러: 내장된 JSON 리소스를 찾을 수 없습니다. 이름을 확인하세요.");
                    return;
                }

                // 4. 스트림을 텍스트로 읽어서 변환합니다.
                using (StreamReader reader = new StreamReader(stream))
                {
                    string jsonString = reader.ReadToEnd();

                    LoadedApi = JsonSerializer.Deserialize<EngineApiRoot>(jsonString);
                }
            }
        }
    }
}
