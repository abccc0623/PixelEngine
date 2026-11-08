namespace PixelTool;

/// <summary>Label + draggable X/Y numeric fields for editor forms.</summary>
public sealed class Vector2Field : VectorField
{
    public Vector2Field() : base(2) { Label = "Vector2"; }
}
