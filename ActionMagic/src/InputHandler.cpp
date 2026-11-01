#include "InputHandler.h"

#include "KeyCodes.h"
#include "Settings.h"
#include "SpellCycler.h"

void InputHandler::Register()
{
	const auto manager = RE::BSInputDeviceManager::GetSingleton();
	if (!manager) {
		logger::error("BSInputDeviceManager not available, input handler not registered");
		return;
	}
	manager->AddEventSink(GetSingleton());
	logger::info("Input handler registered");
}

bool InputHandler::CanCycleNow()
{
	const auto ui = RE::UI::GetSingleton();
	if (!ui || ui->GameIsPaused() || ui->IsMenuOpen(RE::Console::MENU_NAME)) {
		return false;
	}
	return RE::PlayerCharacter::GetSingleton() != nullptr;
}

RE::BSEventNotifyControl InputHandler::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
{
	const auto settings = Settings::GetSingleton();
	if (!a_event || !settings->enabled) {
		return RE::BSEventNotifyControl::kContinue;
	}

	for (auto event = *a_event; event; event = event->next) {
		const auto button = event->AsButtonEvent();
		if (!button) {
			continue;
		}

		const auto key = KeyCodes::ToUnified(button);
		if (key == KeyCodes::kInvalid) {
			continue;
		}

		if (settings->modifierKey != 0 && key == settings->modifierKey) {
			modifierHeld_ = button->IsPressed();
			continue;
		}

		if (!button->IsDown()) {
			continue;
		}

		int direction = 0;
		if (key == settings->nextKey) {
			direction = 1;
		} else if (key == settings->prevKey) {
			direction = -1;
		} else {
			continue;
		}

		if (settings->modifierKey != 0 && !modifierHeld_) {
			continue;
		}
		if (!CanCycleNow()) {
			continue;
		}

		// Equipping must happen on the main thread.
		SKSE::GetTaskInterface()->AddTask([direction]() {
			SpellCycler::GetSingleton()->Cycle(direction);
		});
	}

	return RE::BSEventNotifyControl::kContinue;
}
