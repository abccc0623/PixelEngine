$ErrorActionPreference = 'Stop'
$source = Get-Content (Join-Path $PSScriptRoot '../PixelTool/Tool/EditorLuaWindow/LuaCompletionSnippet.cs') -Raw
$checks = @'
public static class LuaSnippetChecks
{
    public static void Run()
    {
        Check("Set(${1:name}, ${2:amount}, ${3:enabled})$0", "name: string, amount: number, enabled: boolean", "Set(\"\", 0, false)");
        Check("${1:foo} + $1", null, "foo + foo");
        Check("$1 / ${1:value}", null, "value / value");
        Check("${1|left,right|}", null, "left");
        Check("${1:${2:value}}", null, "value");
        Check("\\$value ${1:a\\}b}", null, "$value a}b");
        Check("f(\"number\", math.max(1,2), ${1:count})", "count: integer", "f(\"number\", math.max(1,2), 0)");
        Check("${TM_SELECTED_TEXT}", null, "selection");
        var parts = PixelTool.LuaCompletionSnippet.Parse("${1:name}$0",null,"");
        if (parts[0].Index != 1 || parts[1].Index != 0) throw new System.Exception("Tab stops lost");
        bool rejected = false;
        try { PixelTool.LuaCompletionSnippet.Parse("${1/foo/bar/}",null,""); }
        catch (System.FormatException) { rejected = true; }
        if (!rejected) throw new System.Exception("Unsupported transform must not corrupt Lua");
    }
    static void Check(string input,string detail,string expected)
    {
        string actual = PixelTool.LuaCompletionSnippet.Flatten(PixelTool.LuaCompletionSnippet.Parse(input,detail,"selection"));
        if (actual != expected) throw new System.Exception(input + " -> " + actual);
    }
}
'@
Add-Type -TypeDefinition ($source + $checks)
[LuaSnippetChecks]::Run()
Write-Output 'PASS: typed defaults, mirrors, forward references, choices, nested defaults, escaping, variables, tab stops and transform rejection'
