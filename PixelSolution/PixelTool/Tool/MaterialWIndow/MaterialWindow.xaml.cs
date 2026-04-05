using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace PixelTool
{
    /// <summary>
    /// MaterialWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MaterialWindow : System.Windows.Controls.UserControl
    {
        public Action Close;

        string TextureFullPath = "";
        public MaterialWindow()
        {
            InitializeComponent();
            FileName.Text = "New Material";
            TexturePath.Text = "Asset/";
            OffsetX.Text = "1";
            OffsetY.Text = "1";
            TilingX.Text = "1";
            TilingY.Text = "1";
        }

        private void CreateButton(object sender, RoutedEventArgs e)
        {
            string filePath = $"./Asset/{FileName.Text}.mat";
            if (!File.Exists(filePath))
            {
            }
            
            string MaterialContent = "";
            if(TexturePath.Text == "Asset/" || string.IsNullOrEmpty(TexturePath.Text))
            {
                TexturePath.Text = "";
            }
            MaterialContent += $"@Path {TexturePath.Text}\n";
            MaterialContent += $"@OffsetX {OffsetX.Text}\n";
            MaterialContent += $"@OffsetY {OffsetY.Text}\n";
            MaterialContent += $"@TilingX {TilingX.Text}\n";
            MaterialContent += $"@TilingY {TilingY.Text}\n";
            File.WriteAllText(filePath, MaterialContent);

            var window = GlobalFunction.GetDockedWindow<AssetWindow>();
            window.Refresh();
            Close.Invoke();
        }


        private void FindTextureButton(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog openFileDialog = new System.Windows.Forms.OpenFileDialog();
            openFileDialog.Title = "Import Texture";
            openFileDialog.Filter = "Texture Files (*.png;*.jpg;*.tga)|*.png;*.jpg;*.tga|All Files (*.*)|*.*";

            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            openFileDialog.InitialDirectory = $"{baseDir}/Asset";

            var result = openFileDialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                string selectedFilePath = openFileDialog.FileName;
                TextureFullPath  = selectedFilePath;
                selectedFilePath = selectedFilePath.Replace(baseDir, "");
                TexturePath.Text = selectedFilePath;

                BitmapImage bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.UriSource = new Uri(TextureFullPath);
                bitmap.EndInit();
                PreviewBrush.ImageSource = bitmap;
                UpdateMaterialPreview();
            }
        }

        private void UpdateMaterialPreview()
        {
            if (PreviewBrush.ImageSource == null) return;

            double.TryParse(OffsetX.Text, out double offsetX);
            double.TryParse(OffsetY.Text, out double offsetY);
            if (!double.TryParse(TilingX.Text, out double tilingX) || tilingX == 0) tilingX = 1.0;
            if (!double.TryParse(TilingY.Text, out double tilingY) || tilingY == 0) tilingY = 1.0;

            // 1. Viewport는 기본값(전체 채우기)으로 고정합니다.
            PreviewBrush.Viewport = new System.Windows.Rect(0, 0, 1, 1);

            // 2. 엔진의 Matrix 변환과 동일한 TransformGroup 생성
            var transformGroup = new System.Windows.Media.TransformGroup();

            // 3. 타일링 (Scale) 적용
            transformGroup.Children.Add(new System.Windows.Media.ScaleTransform(tilingX, tilingY));

            // 4. 오프셋 (Translate) 적용
            // UV 좌표계에서는 값이 양수일 때 텍스처가 반대로 밀려야 직관적이므로 마이너스(-)를 붙이는 것이 좋습니다.
            transformGroup.Children.Add(new System.Windows.Media.TranslateTransform(-offsetX, -offsetY));

            // 5. 브러시에 최종 행렬(Matrix) 적용
            PreviewBrush.RelativeTransform = transformGroup;
        }


        private void OffsetXChange(object sender, RoutedEventArgs e)
        {
            if (!double.TryParse(OffsetX.Text, out double offsetX))
            {
                OffsetX.Text = "1";
            }
            UpdateMaterialPreview();
        }

        private void OffsetYChange(object sender, RoutedEventArgs e)
        {
            if (!double.TryParse(OffsetY.Text, out double offsetY))
            {
                OffsetY.Text = "1";
            }
            UpdateMaterialPreview();
        }

        private void TilingXChange(object sender, RoutedEventArgs e)
        {
            if (!double.TryParse(TilingX.Text, out double tilingX))
            {
                TilingX.Text = "1";
            }
            UpdateMaterialPreview();
        }

        private void TilingYChange(object sender, RoutedEventArgs e)
        {
            if (!double.TryParse(TilingY.Text, out double tilingY))
            {
                TilingY.Text = "1";
            }
            UpdateMaterialPreview();
        }
    }
}
