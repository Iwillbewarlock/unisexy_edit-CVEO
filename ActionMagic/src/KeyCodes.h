#pragma once

// Unified key codes, using the same numbering as SKSE / SkyUI MCM key maps:
//   0-255   keyboard (DirectInput scan codes)
//   256-265 mouse (264 = wheel up, 265 = wheel down)
//   266-281 gamepad
namespace KeyCodes
{
	inline constexpr std::uint32_t kMouseOffset = 256;
	inline constexpr std::uint32_t kGamepadOffset = 266;
	inline constexpr std::uint32_t kInvalid = static_cast<std::uint32_t>(-1);

	inline constexpr std::uint32_t kLeftShift = 0x2A;
	inline constexpr std::uint32_t kWheelUp = kMouseOffset + 8;
	inline constexpr std::uint32_t kWheelDown = kMouseOffset + 9;

	std::uint32_t ToUnified(const RE::ButtonEvent* a_event);
}
