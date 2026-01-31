using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WinPlay
{
    internal class WinPlay
    {
        static void Main(string[] args)
        {
            PixelEngine.EngineInitialize();

            while (true)
            {
                PixelEngine.UpdateEngine();
                if (PixelEngine.GetKeyDown((byte)'W'))
                {
                    var data = PixelEngine.GetTotalTime();
                    Console.WriteLine(data.ToString());
                }
                if (PixelEngine.GetKey((byte)'A'))
                {
                    Console.WriteLine("A");
                }
                if (PixelEngine.GetKey((byte)'S'))
                {
                    Console.WriteLine("S");
                }
                if (PixelEngine.GetKey((byte)'D'))
                {
                    Console.WriteLine("D");
                }
                if (PixelEngine.GetKeyDown(27))
                {
                    PixelEngine.ReleaseEngine();
                    break;
                }
            }
        }
    }
}
