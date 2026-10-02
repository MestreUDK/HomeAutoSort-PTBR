#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <optional>
#include <thread>
#include <utility>
#include <mutex>
#include <numeric>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include "SimpleIni.h"
#include "SKSEMenuFramework.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

extern std::atomic<bool> g_operationRunning;

inline bool IsPhantomItem(RE::TESBoundObject* a_item)
{
    if (!a_item) return true;
    if (a_item->GetFormType() == RE::FormType::LeveledItem) return true;
    const char* name = a_item->GetName();
    if (!name || name[0] == '\0') return true;
    if (!a_item->GetPlayable()) return true;
    return false;
}
