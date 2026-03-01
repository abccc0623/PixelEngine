using System.Globalization;
using System.Windows.Data;

namespace PixelTool
{
    internal class ObjectTypeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is GameObject target)
            {
                if (target.HasModule(MODULE_TYPE.Camera) || target.HasModule(MODULE_TYPE.DebugCamera))
                {
                    return "🎥";
                }
                else
                {
                    return "📦";
                }
            }
            return "❓";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw null;
        }
    }
}
