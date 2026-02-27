using System;
using System.Collections.Generic;
using System.Diagnostics;
  using System.Diagnostics; // 프로세스 실행을 위해 필수!
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace PixelTool
{
    public partial class FolderWindow : UserControl
    {
        public FolderWindow()
        {
            InitializeComponent(); // 주석 해제 필수!
            InitializeFolderView("./Asset");
        }

        // 외부(MainWindow 등)에서 루트 경로를 설정할 때 호출
        public void InitializeFolderView(string rootPath)
        {
            if (!Directory.Exists(rootPath)) return;

            var rootItem = new FileItem
            {
                Name = System.IO.Path.GetFileName(rootPath),
                FullPath = rootPath,
                IsDirectory = true
            };

            LoadDirectories(rootItem);
            FolderTreeView.ItemsSource = new List<FileItem> { rootItem };
        }

        private void LoadDirectories(FileItem parentItem)
        {
            try
            {
                parentItem.Children.Clear();
                var dirInfo = new DirectoryInfo(parentItem.FullPath);

                foreach (var dir in dirInfo.GetDirectories())
                {
                    var item = new FileItem
                    {
                        Name = dir.Name,
                        FullPath = dir.FullName,
                        IsDirectory = true
                    };

                    if (dir.GetDirectories().Length > 0)
                    {
                        item.Children.Add(new FileItem { Name = "Loading..." });
                    }
                    parentItem.Children.Add(item);
                }
            }
            catch { /* 접근 권한 오류 등 예외 처리 */ }
        }


        private void FolderTreeView_Expanded(object sender, RoutedEventArgs e)
        {
            // 이벤트가 발생한 TreeViewItem을 가져옴
            var item = e.OriginalSource as TreeViewItem;

            // 이 아이템에 바인딩된 데이터(FileItem)가 있는지 확인
            if (item != null && item.Header is FileItem fileItem)
            {
                // 1. 이미 자식이 로드되었는지 확인 (더미 "Loading..."만 있는 상태인지)
                if (fileItem.IsDirectory && fileItem.Children.Count == 1 && fileItem.Children[0].Name == "Loading...")
                {
                    // 2. 가짜 자식(더미)을 비우고 진짜 폴더 목록을 읽어옴
                    LoadDirectories(fileItem);
                }
            }
        }
        private void FolderTreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            var selectedItem = e.NewValue as FileItem;
            if (selectedItem != null && selectedItem.IsDirectory)
            {
                UpdateFileListView(selectedItem.FullPath);
            }
        }
        private void UpdateFileListView(string DirectoryPath)
        {
            FileListView.Items.Clear();
            DirectoryInfo dir = new DirectoryInfo(DirectoryPath);

            foreach (FileInfo file in dir.GetFiles())
            {
                var item = new FileDisplayItem { Name = file.Name };
                string ext = file.Extension.ToLower();
                if (ext == ".lua")
                {
                    item.Icon = "📜"; // 루아는 스크립트 모양
                    item.IconColor = "#569CD6"; // 하늘색
                }
                else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                {
                    item.IsImage = true; // 이미지 모드 활성화
                    item.FullPath = file.FullName;
                }
                else
                {
                    item.Icon = "📄"; // 나머지는 일반 문서
                    item.IconColor = "#DA34AE"; // 우리 핑크색
                }
                FileListView.Items.Add(item);
            }
        }

        private void FileListViewItem_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            // 왼쪽 버튼 더블 클릭인지 확인
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                var listBoxItem = sender as ListViewItem;
                if (listBoxItem == null) return;

                var fileData = listBoxItem.Content as FileDisplayItem;
                if (fileData == null) return;

                var selectedFolder = FolderTreeView.SelectedItem as FileItem;

                if (selectedFolder != null && !string.IsNullOrEmpty(fileData.Name))
                {
                    string fullPath = System.IO.Path.Combine(selectedFolder.FullPath, fileData.Name);

                    // 루아 파일이면 VS Code로 열기
                    if (System.IO.Path.GetExtension(fullPath).ToLower() == ".lua")
                    {
                        OpenWithVsCode(fullPath);
                    }
                }
            }
        }



        private void OpenWithVsCode(string filePath)
        {
            try
            {
                // 1. 실행할 프로세스 설정
                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    // VS Code가 환경 변수에 등록되어 있다면 "code"만으로 실행 가능합니다.
                    FileName = "code",
                    // 인자로 파일 경로를 넘깁니다. 경로에 공백이 있을 수 있으므로 따옴표(\")로 감쌉니다.
                    Arguments = $"\"{filePath}\"",
                    // 쉘 실행을 사용하도록 설정 (환경 변수 인식을 위함)
                    UseShellExecute = true,
                    // 창을 새로 띄우지 않고 기존 VS Code 인스턴스가 있다면 그곳에서 엽니다.
                    WindowStyle = ProcessWindowStyle.Hidden
                };
                // 2. 프로세스 시작
                Process.Start(startInfo);
            }
            catch (System.ComponentModel.Win32Exception)
            {
                // 만약 VS Code가 환경 변수에 등록되지 않아 "code" 명령어를 못 찾을 경우
                MessageBox.Show("VS Code('code') 명령어를 찾을 수 없습니다.\nVS Code 설치 시 'PATH에 추가' 옵션을 선택했는지 확인해 주세요.",
                                "실행 실패", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"파일을 여는 중 오류가 발생했습니다: {ex.Message}",
                                "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void ListViewItem_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                if (sender is ListViewItem item)
                {
                    // 아이템 선택 및 포커스
                    item.IsSelected = true;
                    item.Focus();

                    // 메뉴 강제 열기
                    if (item.ContextMenu != null)
                    {
                        item.ContextMenu.PlacementTarget = item;
                        // 메뉴가 나타날 위치 설정 (선택 사항: 아이템 바로 아래)
                        item.ContextMenu.Placement = System.Windows.Controls.Primitives.PlacementMode.Bottom;
                        item.ContextMenu.IsOpen = true;
                    }

                    // 왼쪽 클릭의 기본 동작(드래그 등)을 유지하고 싶다면 아래를 주석 처리,
                    // 클릭만으로 메뉴를 띄우고 끝내려면 아래 주석을 해제하세요.
                     e.Handled = true; 
                }
            }
        }

        private void MenuOpen_Click(object sender, RoutedEventArgs e)
        {
            var selectedItem = FileListView.SelectedItem as FileDisplayItem;
            if (selectedItem != null) OpenWithVsCode(selectedItem.FullPath);
        }

        private void MenuDelete_Click(object sender, RoutedEventArgs e)
        {
            var selectedItem = FileListView.SelectedItem as FileDisplayItem;
            if (selectedItem != null)
            {
                var result = MessageBox.Show($"{selectedItem.Name}을(를) 삭제하시겠습니까?", "삭제 확인", MessageBoxButton.YesNo);
                if (result == MessageBoxResult.Yes)
                {
                    File.Delete(selectedItem.FullPath);
                    // 삭제 후 목록 새로고침 (현재 트리뷰 선택된 경로로)
                    var folder = FolderTreeView.SelectedItem as FileItem;
                    UpdateFileListView(folder.FullPath);
                }
            }
        }
        private void MenuShowInExplorer_Click(object sender, RoutedEventArgs e)
        {
            var selectedItem = FileListView.SelectedItem as FileDisplayItem;
            if (selectedItem != null)
            {
                Process.Start("explorer.exe", $"/select,\"{selectedItem.FullPath}\"");
            }
        }

        private void CreateObject_Click(object sender, RoutedEventArgs e)
        {
            PixelEngine.CreateScene("./Asset/Scene/StartScene.lua");
            PixelEngine.ChangeScene("StartScene");
        }

        private void CreateScene_Click(object sender, RoutedEventArgs e)
        {
            PixelEngine.SaveScene();
        }
    }
}
