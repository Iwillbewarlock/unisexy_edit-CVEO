#pragma once

#include "KeyCodes.h"

enum class Hand
{
	kRight,
	kLeft
};

class Settings
{
public:
	static Settings* GetSingleton()
	{
		static Settings singleton;
		return &singleton;
	}

	void Load();

	// [General]
	bool enabled{ true };
	bool showNotifications{ true };

	// [Controls] - unified key codes (see KeyCodes.h). modifierKey = 0 means "no modifier".
	std::uint32_t modifierKey{ KeyCodes::kLeftShift };
	std::uint32_t nextKey{ KeyCodes::kWheelDown };
	std::uint32_t prevKey{ KeyCodes::kWheelUp };

	// [BareHand] - which hand receives the cycled spell when no staff is equipped.
	Hand bareHandTarget{ Hand::kRight };

private:
	Settings() = default;
};
