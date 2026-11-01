#include "Settings.h"

namespace
{
	constexpr auto kIniPath = L"Data/SKSE/Plugins/ActionMagic.ini";

	Hand ParseHand(std::string_view a_value, Hand a_default)
	{
		if (a_value == "Right"sv || a_value == "right"sv) {
			return Hand::kRight;
		}
		if (a_value == "Left"sv || a_value == "left"sv) {
			return Hand::kLeft;
		}
		return a_default;
	}

	const char* HandToString(Hand a_hand)
	{
		return a_hand == Hand::kLeft ? "Left" : "Right";
	}
}

void Settings::Load()
{
	CSimpleIniA ini;
	ini.SetUnicode();

	const bool exists = ini.LoadFile(kIniPath) >= 0;
	if (!exists) {
		logger::info("ActionMagic.ini not found, writing defaults");
	}

	enabled = ini.GetBoolValue("General", "bEnabled", enabled);
	showNotifications = ini.GetBoolValue("General", "bShowNotifications", showNotifications);

	modifierKey = static_cast<std::uint32_t>(ini.GetLongValue("Controls", "iModifierKey", static_cast<long>(modifierKey)));
	nextKey = static_cast<std::uint32_t>(ini.GetLongValue("Controls", "iNextKey", static_cast<long>(nextKey)));
	prevKey = static_cast<std::uint32_t>(ini.GetLongValue("Controls", "iPrevKey", static_cast<long>(prevKey)));

	bareHandTarget = ParseHand(ini.GetValue("BareHand", "sTargetHand", HandToString(bareHandTarget)), bareHandTarget);

	if (!exists) {
		ini.SetBoolValue("General", "bEnabled", enabled, "; Master switch for the plugin.");
		ini.SetBoolValue("General", "bShowNotifications", showNotifications, "; Show a top-left notification with the newly selected spell.");
		ini.SetLongValue("Controls", "iModifierKey", static_cast<long>(modifierKey), "; Key that must be held while cycling. 0 = no modifier. Default 42 = Left Shift.");
		ini.SetLongValue("Controls", "iNextKey", static_cast<long>(nextKey), "; Next spell. Default 265 = mouse wheel down.");
		ini.SetLongValue("Controls", "iPrevKey", static_cast<long>(prevKey), "; Previous spell. Default 264 = mouse wheel up.");
		ini.SetValue("BareHand", "sTargetHand", HandToString(bareHandTarget), "; Preferred hand when no staff is equipped: Right or Left.\n; If that hand holds a weapon, shield or torch, the other hand is used instead.");
		ini.SaveFile(kIniPath);
	}

	logger::info("Settings: enabled={} notify={} modifier={} next={} prev={} bareHand={}",
		enabled, showNotifications, modifierKey, nextKey, prevKey, HandToString(bareHandTarget));
}
