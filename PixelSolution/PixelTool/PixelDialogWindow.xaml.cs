using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace PixelTool
{
    public partial class PixelDialogWindow : Window
    {
        private readonly MessageBoxButton _buttons;

        public MessageBoxResult Result { get; private set; } = MessageBoxResult.None;
        public string InputText => InputTextBox.Text;

        public PixelDialogWindow(string message, string title, MessageBoxButton buttons,
            MessageBoxImage image, string? defaultInput = null)
        {
            InitializeComponent();
            _buttons = buttons;
            TitleText.Text = string.IsNullOrWhiteSpace(title) ? "PixelTool" : title;
            MessageText.Text = message;
            ConfigureIcon(image);
            ConfigureButtons(buttons);

            if (defaultInput != null)
            {
                InputTextBox.Visibility = Visibility.Visible;
                InputTextBox.Text = defaultInput;
                Loaded += (_, _) =>
                {
                    InputTextBox.Focus();
                    InputTextBox.SelectAll();
                };
            }
        }

        private void ConfigureIcon(MessageBoxImage image)
        {
            IconText.Text = image switch
            {
                MessageBoxImage.Warning => "!",
                MessageBoxImage.Error => "x",
                MessageBoxImage.Question => "?",
                _ => "i"
            };

            if (image == MessageBoxImage.Error)
                IconText.Foreground = new SolidColorBrush(Color.FromRgb(255, 115, 115));
        }

        private void ConfigureButtons(MessageBoxButton buttons)
        {
            switch (buttons)
            {
                case MessageBoxButton.OKCancel:
                    AddButton("OK", MessageBoxResult.OK, true);
                    AddButton("Cancel", MessageBoxResult.Cancel);
                    break;
                case MessageBoxButton.YesNo:
                    AddButton("Yes", MessageBoxResult.Yes, true);
                    AddButton("No", MessageBoxResult.No);
                    break;
                case MessageBoxButton.YesNoCancel:
                    AddButton("Cancel", MessageBoxResult.Cancel);
                    AddButton("Yes", MessageBoxResult.Yes, true);
                    AddButton("No", MessageBoxResult.No);
                    break;
                default:
                    AddButton("OK", MessageBoxResult.OK, true);
                    break;
            }
        }

        private void AddButton(string text, MessageBoxResult result, bool isDefault = false)
        {
            var button = new Button
            {
                Content = text,
                MinWidth = 88,
                MinHeight = 34,
                Margin = new Thickness(8, 0, 0, 0),
                IsDefault = isDefault,
                Style = (Style)FindResource("PixelToolButton")
            };
            button.Click += (_, _) => Complete(result);
            ButtonPanel.Children.Add(button);
        }

        private void Complete(MessageBoxResult result)
        {
            Result = result;
            DialogResult = result is MessageBoxResult.OK or MessageBoxResult.Yes;
        }

        private void Close_Click(object sender, RoutedEventArgs e) => CloseWithCancel();

        private void CloseWithCancel()
        {
            Result = _buttons == MessageBoxButton.YesNo ? MessageBoxResult.No : MessageBoxResult.Cancel;
            DialogResult = false;
        }

        private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ButtonState == MouseButtonState.Pressed) DragMove();
        }

        private void InputTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Enter) return;
            Complete(MessageBoxResult.OK);
            e.Handled = true;
        }
    }

    public static class PixelMessageBox
    {
        public static MessageBoxResult Show(string message) =>
            Show(message, "PixelTool", MessageBoxButton.OK, MessageBoxImage.Information);

        public static MessageBoxResult Show(string message, string title) =>
            Show(message, title, MessageBoxButton.OK, MessageBoxImage.Information);

        public static MessageBoxResult Show(string message, string title,
            MessageBoxButton buttons, MessageBoxImage image)
        {
            var dialog = new PixelDialogWindow(message, title, buttons, image)
            {
                Owner = GetOwner()
            };
            dialog.ShowDialog();
            return dialog.Result;
        }

        private static Window? GetOwner() =>
            Application.Current?.Windows.OfType<Window>().FirstOrDefault(window => window.IsActive)
            ?? Application.Current?.MainWindow;
    }

    public static class PixelPromptDialog
    {
        public static string Show(string message, string title, string defaultValue = "")
        {
            var owner = Application.Current?.Windows.OfType<Window>()
                .FirstOrDefault(window => window.IsActive) ?? Application.Current?.MainWindow;
            var dialog = new PixelDialogWindow(message, title, MessageBoxButton.OKCancel,
                MessageBoxImage.Question, defaultValue) { Owner = owner };
            return dialog.ShowDialog() == true ? dialog.InputText.Trim() : string.Empty;
        }
    }
}
