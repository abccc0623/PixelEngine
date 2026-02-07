using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WinPlay
{
    internal class WinPlay
    {
        static void Main(string[] args)
        {
            //dll 파일이 해당 라이브러리도 찾도록 수정
            string libraryPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Library");
            if (!PixelEngine.SetDllDirectory(libraryPath)) return;
        
            PixelEngine.EngineInitialize();
            while (PixelEngine.RunningEngineCheck())
            {
                if (PixelEngine.GetKeyDown(27))
                {
                    break;
                }
            }
            PixelEngine.ReleaseEngine();
        }
    }
}
