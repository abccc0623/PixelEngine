using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;

namespace PixelTool;

/// <summary>Editor palette. Change the default colors here; resource keys remain stable.</summary>
public sealed class EditorColors : ResourceDictionary
{
    public static readonly Dictionary<string, string> Palette = new()
    {
        ["PixelBackgroundBrush"] = "#202630",
        ["PixelPanelBrush"] = "#29323E",
        ["PixelSurfaceBrush"] = "#343F4D",
        ["PixelAccentBrush"] = "#FFD600",
        ["PixelAccentMutedBrush"] = "#544B16",
        ["PixelBorderBrush"] = "#3C4755",
        ["PixelTextBrush"] = "#E6EAF0",
        ["PixelMutedTextBrush"] = "#A7AFBA",
        ["PixelWhiteBrush"] = "#FFFFFF",
        ["PixelDockBorderBrush"] = "#323C49",
        ["PixelButtonBrush"] = "#242C36",
        ["PixelButtonHoverBrush"] = "#35404C",
        ["PixelButtonPressedBrush"] = "#FFD600",
        ["PixelButtonTextBrush"] = "#E1E1F0",
        ["PixelTitleBarBrush"] = "#202630",
        ["PixelTitleTextBrush"] = "#E6EAF0",
        ["PixelTabBackgroundBrush"] = "#202630",
        ["PixelTabSelectedBrush"] = "#29323E",
        ["PixelTabBorderBrush"] = "#323C49",
        ["PixelTabSelectedBorderBrush"] = "#FFD600",
        ["PixelTabTextBrush"] = "#A7AFBA",
        ["PixelTabSelectedTextBrush"] = "#FFD600",
        ["PixelFileTabBackgroundBrush"] = "#1B2029",
        ["PixelFileTabHoverBrush"] = "#303C4E",
        ["PixelFileTabSelectedBrush"] = "#254A70",
        ["PixelFileTabAccentBrush"] = "#65B9FF",
        ["PixelFileTabTextBrush"] = "#A8B3C5",
    };

    public EditorColors()
    {
        foreach (var entry in Palette)
            this[entry.Key] = new SolidColorBrush((Color)ColorConverter.ConvertFromString(entry.Value));
        this["PixelBackgroundColor"] = ((SolidColorBrush)this["PixelBackgroundBrush"]).Color;
        this["PixelBorderColor"] = ((SolidColorBrush)this["PixelBorderBrush"]).Color;
    }

    /// <summary>Apply native window chrome colors when the window handle is ready.</summary>
    public static void AttachTitleBar(Window window)
    {
        window.SourceInitialized += (_, _) =>
        {
            // Custom caption colors require Windows 11; older Windows keeps its system caption.
            if (!OperatingSystem.IsWindowsVersionAtLeast(10, 0, 22000)) return;
            var handle = new WindowInteropHelper(window).Handle;
            SetCaptionColor(handle, 35, "PixelTitleBarBrush");
            SetCaptionColor(handle, 36, "PixelTitleTextBrush");
            SetCaptionColor(handle, 34, "PixelDockBorderBrush");
        };
    }

    private static void SetCaptionColor(IntPtr handle, int attribute, string key)
    {
        var color = ((SolidColorBrush)Application.Current.FindResource(key)).Color;
        int value = color.R | color.G << 8 | color.B << 16;
        DwmSetWindowAttribute(handle, attribute, ref value, sizeof(int));
    }

    [DllImport("dwmapi.dll")]
    private static extern int DwmSetWindowAttribute(IntPtr window, int attribute, ref int value, int size);
}
