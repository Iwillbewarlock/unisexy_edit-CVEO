#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;

using namespace std::literals;

#include "Version.h"
