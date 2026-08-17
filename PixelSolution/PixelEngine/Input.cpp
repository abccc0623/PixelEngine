#include "pch.h"
#include "Input.h"
#include "PixelEngine.h"
#include "KeyInputManager.h"
#include "GlobalEnum.h"
extern PixelEngine* Engine;
namespace
{
	byte ToVirtualKey(Keycode key)
	{
		switch (key)
		{
		case Keycode::MouseLeft: return VK_LBUTTON;
		case Keycode::MouseRight: return VK_RBUTTON;
		case Keycode::MouseMiddle: return VK_MBUTTON;
		case Keycode::Backspace: return VK_BACK;
		case Keycode::Tab: return VK_TAB;
		case Keycode::Enter: return VK_RETURN;
		case Keycode::Shift: return VK_SHIFT;
		case Keycode::Control: return VK_CONTROL;
		case Keycode::Alt: return VK_MENU;
		case Keycode::Pause: return VK_PAUSE;
		case Keycode::CapsLock: return VK_CAPITAL;
		case Keycode::Escape: return VK_ESCAPE;
		case Keycode::Space: return VK_SPACE;
		case Keycode::PageUp: return VK_PRIOR;
		case Keycode::PageDown: return VK_NEXT;
		case Keycode::End: return VK_END;
		case Keycode::Home: return VK_HOME;
		case Keycode::Left: return VK_LEFT;
		case Keycode::Up: return VK_UP;
		case Keycode::Right: return VK_RIGHT;
		case Keycode::Down: return VK_DOWN;
		case Keycode::Insert: return VK_INSERT;
		case Keycode::Delete: return VK_DELETE;
		case Keycode::D0: case Keycode::D1: case Keycode::D2: case Keycode::D3: case Keycode::D4:
		case Keycode::D5: case Keycode::D6: case Keycode::D7: case Keycode::D8: case Keycode::D9:
			return static_cast<byte>('0' + (static_cast<int>(key) - static_cast<int>(Keycode::D0)));
		case Keycode::A: case Keycode::B: case Keycode::C: case Keycode::D: case Keycode::E:
		case Keycode::F: case Keycode::G: case Keycode::H: case Keycode::I: case Keycode::J:
		case Keycode::K: case Keycode::L: case Keycode::M: case Keycode::N: case Keycode::O:
		case Keycode::P: case Keycode::Q: case Keycode::R: case Keycode::S: case Keycode::T:
		case Keycode::U: case Keycode::V: case Keycode::W: case Keycode::X: case Keycode::Y: case Keycode::Z:
			return static_cast<byte>('A' + (static_cast<int>(key) - static_cast<int>(Keycode::A)));
		case Keycode::Numpad0: case Keycode::Numpad1: case Keycode::Numpad2: case Keycode::Numpad3: case Keycode::Numpad4:
		case Keycode::Numpad5: case Keycode::Numpad6: case Keycode::Numpad7: case Keycode::Numpad8: case Keycode::Numpad9:
			return static_cast<byte>(VK_NUMPAD0 + (static_cast<int>(key) - static_cast<int>(Keycode::Numpad0)));
		case Keycode::Multiply: return VK_MULTIPLY;
		case Keycode::Add: return VK_ADD;
		case Keycode::Subtract: return VK_SUBTRACT;
		case Keycode::Decimal: return VK_DECIMAL;
		case Keycode::Divide: return VK_DIVIDE;
		case Keycode::F1: case Keycode::F2: case Keycode::F3: case Keycode::F4: case Keycode::F5: case Keycode::F6:
		case Keycode::F7: case Keycode::F8: case Keycode::F9: case Keycode::F10: case Keycode::F11: case Keycode::F12:
			return static_cast<byte>(VK_F1 + (static_cast<int>(key) - static_cast<int>(Keycode::F1)));
		case Keycode::NumLock: return VK_NUMLOCK;
		case Keycode::ScrollLock: return VK_SCROLL;
		case Keycode::LeftShift: return VK_LSHIFT;
		case Keycode::RightShift: return VK_RSHIFT;
		case Keycode::LeftControl: return VK_LCONTROL;
		case Keycode::RightControl: return VK_RCONTROL;
		case Keycode::LeftAlt: return VK_LMENU;
		case Keycode::RightAlt: return VK_RMENU;
		case Keycode::None: default: return 0;
		}
	}
}

bool Input_Key(Keycode key)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return key != Keycode::None && input->GetKey(ToVirtualKey(key));
}

bool Input_KeyDown(Keycode key)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return key != Keycode::None && input->GetKeyDown(ToVirtualKey(key));
}

bool Input_KeyUp(Keycode key)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return key != Keycode::None && input->GetKeyUp(ToVirtualKey(key));
}

int Input_GetMousePositionX()
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->MousePosition_X();
}

int Input_GetMousePositionY()
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->MousePosition_Y();
}
