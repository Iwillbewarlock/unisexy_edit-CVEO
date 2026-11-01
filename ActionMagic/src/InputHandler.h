#pragma once

class InputHandler : public RE::BSTEventSink<RE::InputEvent*>
{
public:
	static InputHandler* GetSingleton()
	{
		static InputHandler singleton;
		return &singleton;
	}

	static void Register();

	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

private:
	InputHandler() = default;

	static bool CanCycleNow();

	bool modifierHeld_{ false };
};
