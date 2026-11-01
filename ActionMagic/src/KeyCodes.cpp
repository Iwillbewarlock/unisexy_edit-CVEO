#include "KeyCodes.h"

namespace
{
	std::uint32_t GamepadMaskToIndex(std::uint32_t a_mask)
	{
		using Key = RE::BSWin32GamepadDevice::Key;

		switch (a_mask) {
		case Key::kUp:
			return 0;
		case Key::kDown:
			return 1;
		case Key::kLeft:
			return 2;
		case Key::kRight:
			return 3;
		case Key::kStart:
			return 4;
		case Key::kBack:
			return 5;
		case Key::kLeftThumb:
			return 6;
		case Key::kRightThumb:
			return 7;
		case Key::kLeftShoulder:
			return 8;
		case Key::kRightShoulder:
			return 9;
		case Key::kA:
			return 10;
		case Key::kB:
			return 11;
		case Key::kX:
			return 12;
		case Key::kY:
			return 13;
		case Key::kLeftTrigger:
			return 14;
		case Key::kRightTrigger:
			return 15;
		default:
			return KeyCodes::kInvalid;
		}
	}
}

namespace KeyCodes
{
	std::uint32_t ToUnified(const RE::ButtonEvent* a_event)
	{
		if (!a_event) {
			return kInvalid;
		}

		const auto idCode = a_event->GetIDCode();

		switch (a_event->GetDevice()) {
		case RE::INPUT_DEVICE::kKeyboard:
			return idCode;
		case RE::INPUT_DEVICE::kMouse:
			return kMouseOffset + idCode;
		case RE::INPUT_DEVICE::kGamepad:
			{
				const auto index = GamepadMaskToIndex(idCode);
				return index == kInvalid ? kInvalid : kGamepadOffset + index;
			}
		default:
			return kInvalid;
		}
	}
}
