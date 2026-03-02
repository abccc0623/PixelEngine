using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;
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
using static System.Formats.Asn1.AsnWriter;

namespace PixelTool
{
    /// <summary>
    /// Transform.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class TransformView : UserControl
    {
        public TransformView()
        {
            InitializeComponent();
        }

        private void NumberOnly_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            // 정규표현식: 숫자, 소수점(.), 마이너스(-)만 허용
            Regex regex = new Regex(@"^[0-9.-]+$");
            e.Handled = !regex.IsMatch(e.Text);
        }

       
    }


    public class TransformData : INotifyPropertyChanged
    {
        private Transform _coreTransform;
        public TransformData(Transform coreTransform)
        {
            _coreTransform = coreTransform;
        }

        public void RefreshFromCore()
        {
            OnPropertyChanged(nameof(PositionX));
            OnPropertyChanged(nameof(PositionY));
            OnPropertyChanged(nameof(PositionZ));

            OnPropertyChanged(nameof(RotationX));
            OnPropertyChanged(nameof(RotationY));
            OnPropertyChanged(nameof(RotationZ));

            OnPropertyChanged(nameof(ScaleX));
            OnPropertyChanged(nameof(ScaleY));
            OnPropertyChanged(nameof(ScaleZ));
        }


        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string name = null)
        {
            if (Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(name);
            return true;
        }



        public float PositionX
        {
            get => _coreTransform.GetPosition().X;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetPosition();
                _coreTransform.SetPosition(value,vec.Y,vec.Z);
            }
        }
        public float PositionY
        {
            get => _coreTransform.GetPosition().Y;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetPosition();
                _coreTransform.SetPosition(vec.X, value, vec.Z);
            }
        }
        public float PositionZ
        {
            get => _coreTransform.GetPosition().Z;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetPosition();
                _coreTransform.SetPosition(vec.X, vec.Y, value);
            }
        }

        public float RotationX
        {
            get => _coreTransform.GetRotation().X;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetRotation();
                _coreTransform.SetRotation(value, vec.Y, vec.Z);
            }
        }
        public float RotationY
        {
            get => _coreTransform.GetRotation().Y;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetRotation();
                _coreTransform.SetRotation(vec.X, value, vec.Z);
            }
        }
        public float RotationZ
        {
            get => _coreTransform.GetRotation().Z;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetRotation();
                _coreTransform.SetRotation(vec.X, vec.Y, value);
            }
        }

        public float ScaleX
        {
            get => _coreTransform.GetScale().X;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetScale();
                _coreTransform.SetScale(value, vec.Y, vec.Z);
            }
        }
        public float ScaleY
        {
            get => _coreTransform.GetScale().Y;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetScale();
                _coreTransform.SetScale(vec.X, value, vec.Z);
            }
        }
        public float ScaleZ
        {
            get => _coreTransform.GetScale().Z;
            set
            {
                if (float.IsNaN(value) || float.IsInfinity(value)) return;
                var vec = _coreTransform.GetScale();
                _coreTransform.SetScale(vec.X, vec.Y, value);
            }
        }
    }
}
