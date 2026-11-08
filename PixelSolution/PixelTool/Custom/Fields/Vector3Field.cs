namespace PixelTool;

/// <summary>Label + draggable X/Y/Z numeric fields for editor forms.</summary>
public sealed class Vector3Field : VectorField
{
    public Vector3Field() : base(3) { Label = "Vector3"; }
}
