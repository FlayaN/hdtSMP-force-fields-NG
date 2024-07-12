#pragma once

#define SKSE_SUPPORT_XBYAK
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif

#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/singleton.hpp>

#undef min
#undef max

#include "hdtSMP64/PluginAPI.h"
#include "detours/detours.h"

#define BT_USE_SSE_IN_API
#include "BulletDynamics/Dynamics/btRigidBody.h"

using namespace std::literals;

namespace logger = SKSE::log;

namespace stl
{
	template <class T>
	bool detour_func(REL::RelocationID a_relId)
	{
		T::func = reinterpret_cast<decltype(&T::thunk)>(a_relId.address());

		if (LONG res = DetourAttach((PVOID*)&T::func, T::thunk) != NO_ERROR) {
			logger::error("Detouring {} failed with error code {}", typeid(T).name(), res);
			return false;
		}
		logger::info("Attaching detour to {}", typeid(T).name());
		return true;
	}
}

namespace util
{
	using SKSE::stl::report_and_fail;
}
