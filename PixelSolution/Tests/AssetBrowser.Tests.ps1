$ErrorActionPreference = 'Stop'
$source = Get-Content (Join-Path $PSScriptRoot '../PixelTool/Tool/AssetWindow/AssetBrowserSupport.cs') -Raw
$checks = @'
public static class AssetBrowserChecks
{
    static void Assert(bool condition, string message)
    {
        if (!condition) throw new System.Exception(message);
    }
    static void Reject(System.Action action)
    {
        bool rejected = false;
        try { action(); } catch (System.IO.IOException) { rejected = true; }
        Assert(rejected,"Unsafe operation accepted");
    }
    public static void Run(string workspace)
    {
        string temporary = System.IO.Path.Combine(workspace,".tmp","asset-browser-test-" + System.Guid.NewGuid().ToString("N"));
        System.IO.Directory.CreateDirectory(temporary);
        try
        {
            string asset = System.IO.Path.Combine(temporary,"Asset");
            string a = System.IO.Path.Combine(asset,"A.version");
            string b = System.IO.Path.Combine(asset,"B");
            string external = System.IO.Path.Combine(temporary,"External");
            System.IO.Directory.CreateDirectory(a);
            System.IO.Directory.CreateDirectory(b);
            System.IO.Directory.CreateDirectory(external);
            string original = System.IO.Path.Combine(a,"test.lua");
            System.IO.File.WriteAllText(original,"original");
            PixelTool.AssetFileOperations.Transfer(new[]{original},b,false,asset);
            Assert(System.IO.File.ReadAllText(System.IO.Path.Combine(b,"test.lua")) == "original","Copy failed");
            PixelTool.AssetFileOperations.Transfer(new[]{original},b,false,asset);
            Assert(System.IO.File.Exists(System.IO.Path.Combine(b,"test_1.lua")),"Collision overwrote file");
            int notifications = 0;
            PixelTool.AssetFileOperations.Transfer(new[]{original},b,true,asset,(_,_) => notifications++);
            Assert(!System.IO.File.Exists(original) && notifications == 1,"Move callback failed");
            string nested = System.IO.Path.Combine(a,"Child");
            System.IO.Directory.CreateDirectory(nested);
            System.IO.File.WriteAllText(System.IO.Path.Combine(nested,"value.txt"),"data");
            PixelTool.AssetFileOperations.Transfer(new[]{a,nested},b,false,asset);
            Assert(System.IO.File.Exists(System.IO.Path.Combine(b,"A.version","Child","value.txt")),"Recursive folder copy failed");
            Assert(!System.IO.Directory.Exists(System.IO.Path.Combine(b,"Child")),"Selected child transferred twice");
            PixelTool.AssetFileOperations.Transfer(new[]{a},b,true,asset);
            Assert(!System.IO.Directory.Exists(a) && System.IO.Directory.Exists(System.IO.Path.Combine(b,"A.version_1")),"Dotted folder move failed");
            string imported = System.IO.Path.Combine(external,"import.png");
            System.IO.File.WriteAllText(imported,"external");
            PixelTool.AssetFileOperations.Transfer(new[]{imported},b,true,asset);
            Assert(System.IO.File.Exists(imported) && System.IO.File.Exists(System.IO.Path.Combine(b,"import.png")),"External import moved original");
            Reject(() => PixelTool.AssetFileOperations.Transfer(new[]{b},b,false,asset));
            Reject(() => PixelTool.AssetFileOperations.Transfer(new[]{asset},external,true,asset));
            Reject(() => PixelTool.AssetFileOperations.RequireAssetPath(asset + System.IO.Path.DirectorySeparatorChar,asset));
            Reject(() => PixelTool.AssetFileOperations.NamedPath(b,"../escape"));
            Assert(!PixelTool.AssetFileOperations.IsInside(asset + "Other",asset),"Root-prefix escape allowed");
            var state = new PixelTool.AssetBrowserState();
            state.Expanded["."] = true;
            state.Expanded["B"] = false;
            state.Expanded["B/Child"] = true;
            state.Selected = "B";
            string stateFile = System.IO.Path.Combine(temporary,"Editor","AssetBrowser.json");
            state.Save(stateFile);
            var restored = PixelTool.AssetBrowserState.Load(stateFile);
            Assert(restored.Expanded["."] && !restored.Expanded["b"] && restored.Expanded["B/Child"] && restored.Selected == "B","Expansion state did not persist");
            restored.Save(stateFile);
            Assert(!System.IO.File.Exists(stateFile + ".tmp"),"State save left temporary file");
        }
        finally
        {
            // Only the GUID directory created by this test may be removed.
            Assert(PixelTool.AssetFileOperations.IsInside(temporary,System.IO.Path.Combine(workspace,".tmp")),"Unexpected cleanup target");
            System.IO.Directory.Delete(temporary,true);
        }
    }
}
'@
Add-Type -TypeDefinition ("#nullable enable`n" + $source + $checks)
[AssetBrowserChecks]::Run((Resolve-Path (Join-Path $PSScriptRoot '../..')).Path)
Write-Output 'PASS: copy, cut/move, collision-safe names, nested folders, imports, path guards and persisted expansion state'
