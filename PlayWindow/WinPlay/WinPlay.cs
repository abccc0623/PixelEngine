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
            }
        }
    }
}
