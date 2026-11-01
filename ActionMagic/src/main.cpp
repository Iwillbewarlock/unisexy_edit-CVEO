#include "InputHandler.h"
#include "Settings.h"

namespace
{
	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kInputLoaded:
			InputHandler::Register();
			break;
		default:
			break;
		}
	}

	void InitializeLog()
	{
		auto path = logger::log_directory();
		if (!path) {
			SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log", Version::PROJECT);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%H:%M:%S] %v"s);

		logger::info("{} v{}", Version::PROJECT, Version::NAME);
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion({ Version::MAJOR, Version::MINOR, Version::PATCH });
	v.PluginName("ActionMagic");
	v.AuthorName("Iwillbewarlock");
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	SKSE::Init(a_skse);

	Settings::GetSingleton()->Load();

	const auto messaging = SKSE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(OnMessage)) {
		logger::error("Failed to register messaging listener");
		return false;
	}

	return true;
}
