using System;
using System.IO;

namespace PixelTool
{
    public static class ProjectPathService
    {
        private static readonly string SettingsDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "PixelTool");

        private static readonly string LastProjectPathFile = Path.Combine(SettingsDirectory, "LastProjectPath.txt");
        private static readonly string LegacyLastAssetPathFile = Path.Combine(SettingsDirectory, "LastAssetPath.txt");

        public static string ProjectRootPath { get; private set; } = string.Empty;

        public static string AssetPath => GetProjectFolderPath("Asset");
        public static string EnginePath => GetProjectFolderPath("Engine");
        public static string EditorPath => GetProjectFolderPath("Editor");

        public static bool GenerateProjectFilesPending { get; set; }

        public static string LoadPreviousProjectPath()
        {
            try
            {
                if (File.Exists(LastProjectPathFile))
                {
                    string savedRootPath = File.ReadAllText(LastProjectPathFile).Trim();
                    return IsProjectRoot(savedRootPath) ? Path.GetFullPath(savedRootPath) : string.Empty;
                }

                if (!File.Exists(LegacyLastAssetPathFile)) return string.Empty;

                string legacyAssetPath = File.ReadAllText(LegacyLastAssetPathFile).Trim();
                if (!Directory.Exists(legacyAssetPath) ||
                    !string.Equals(Path.GetFileName(legacyAssetPath), "Asset", StringComparison.OrdinalIgnoreCase))
                {
                    return string.Empty;
                }

                string? projectRootPath = Directory.GetParent(legacyAssetPath)?.FullName;
                return IsProjectRoot(projectRootPath) ? projectRootPath! : string.Empty;
            }
            catch
            {
                return string.Empty;
            }
        }

        public static void CreateProject(string projectRootPath)
        {
            string rootPath = Path.GetFullPath(projectRootPath);
            Directory.CreateDirectory(rootPath);
            Directory.CreateDirectory(Path.Combine(rootPath, "Asset"));
            Directory.CreateDirectory(Path.Combine(rootPath, "Engine"));
            Directory.CreateDirectory(Path.Combine(rootPath, "Editor"));

            GenerateProjectFilesPending = true;
        }

        public static void UseProjectPath(string projectRootPath)
        {
            string rootPath = Path.GetFullPath(projectRootPath);
            if (!IsProjectRoot(rootPath))
            {
                throw new DirectoryNotFoundException("The selected folder is not a valid PixelTool project.");
            }

            ProjectRootPath = rootPath;
            Directory.SetCurrentDirectory(ProjectRootPath);

            Directory.CreateDirectory(SettingsDirectory);
            File.WriteAllText(LastProjectPathFile, ProjectRootPath);
        }

        public static string GetAssetFilePath(params string[] relativeParts) =>
            CombineProjectPath(AssetPath, relativeParts, "Asset path is not configured.");

        public static string GetEngineFilePath(params string[] relativeParts) =>
            CombineProjectPath(EnginePath, relativeParts, "Engine path is not configured.");

        public static string GetEditorFilePath(params string[] relativeParts) =>
            CombineProjectPath(EditorPath, relativeParts, "Editor path is not configured.");

        public static void StartNewProject()
        {
            if (!GenerateProjectFilesPending) return;

            string targetPath = EnginePath.Replace("\\", "/");
            PixelEngineNative.EditorNotify(EditorEventType.GenerateEngineFile, targetPath);

            GenerateProjectFilesPending = false;
        }

        private static string GetProjectFolderPath(string folderName) =>
            string.IsNullOrEmpty(ProjectRootPath)
                ? string.Empty
                : Path.Combine(ProjectRootPath, folderName);

        private static string CombineProjectPath(string basePath, string[] relativeParts, string errorMessage)
        {
            if (string.IsNullOrEmpty(basePath)) throw new InvalidOperationException(errorMessage);

            string path = basePath;
            foreach (string part in relativeParts) path = Path.Combine(path, part);
            return path;
        }

        public static bool IsProjectRoot(string? path) =>
            !string.IsNullOrWhiteSpace(path) &&
            Directory.Exists(path) &&
            Directory.Exists(Path.Combine(path, "Asset")) &&
            Directory.Exists(Path.Combine(path, "Engine")) &&
            Directory.Exists(Path.Combine(path, "Editor"));
    }
}
