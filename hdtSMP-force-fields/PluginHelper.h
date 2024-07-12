#pragma once

#include "ForceFieldManager.h"
#include "Settings.h"

namespace hdt
{
	constexpr bool operator<(const PluginInterface::Version& lhs, const PluginInterface::Version& rhs)
	{
		bool result = false;

		if (lhs.major < rhs.major) {
			result = true;
		}
		else if (lhs.major == rhs.major) {
			if (lhs.minor < rhs.minor) {
				result = true;
			}
			else if (lhs.minor == rhs.minor) {
				result = lhs.patch < rhs.patch;
			}
		}

		return result;
	}
	constexpr bool operator>(const PluginInterface::Version& lhs, const PluginInterface::Version& rhs)
	{
		return rhs < lhs;
	}
	constexpr bool operator>=(const PluginInterface::Version& lhs, const PluginInterface::Version& rhs)
	{
		return !(lhs < rhs);
	}
	constexpr bool operator<=(const PluginInterface::Version& lhs, const PluginInterface::Version& rhs)
	{
		return !(rhs < lhs);
	}

	class PluginHelper
	{
	public:
		static void tryConnect(const SKSE::LoadInterface*)
		{
			SKSE::GetMessagingInterface()->RegisterListener("SKSE", &skseCallback);
		}
	private:

		inline static hdt::PluginInterface::Version interfaceMin{ 1, 0, 0 };
		inline static hdt::PluginInterface::Version interfaceMax{ 2, 0, 0 };

		inline static hdt::PluginInterface::Version bulletMin{ hdt::PluginInterface::BULLET_VERSION };
		inline static hdt::PluginInterface::Version bulletMax{ hdt::PluginInterface::BULLET_VERSION.major + 1, 0, 0 };

		inline static hdt::PluginInterface* ifc = nullptr;

		static void skseCallback(SKSE::MessagingInterface::Message* msg)
		{
			if (msg && msg->type == SKSE::MessagingInterface::kPostLoad) {
				jg::Settings::GetSingleton()->LoadSettings();
				if (!SKSE::GetMessagingInterface()->RegisterListener("hdtSMP64", &smpCallback)) {
					logger::error("HDT-SMP is not loaded.");
				}
			}
			if (msg && msg->type == SKSE::MessagingInterface::kDataLoaded) {
				if (ifc) {
					logger::info("Connection established.");

					logger::info("Starting ForceFieldManager...");
					ifc->addListener(&jg::ForceFieldManager::get());
					logger::info("ForceFieldManager started.");

					logger::info("Initialisation complete.");
				}
				else {
					logger::error("Failed to connect to HDT-SMP.");
					logger::info("Initialisation failed.");
				}
			}
		}

		static void smpCallback(SKSE::MessagingInterface::Message* msg)
		{
			if (msg && msg->type == hdt::PluginInterface::MSG_STARTUP && msg->data) {
				auto smp = reinterpret_cast<hdt::PluginInterface*>(msg->data);

				auto&& info = smp->getVersionInfo();

				if (info.interfaceVersion >= interfaceMin && info.interfaceVersion < interfaceMax) {
					if (info.bulletVersion >= bulletMin && info.bulletVersion < bulletMax) {
						ifc = smp;
					}
					else {
						logger::error("Incompatible Bullet version.");
					}
				}
				else {
					logger::error("Incompatible HDT-SMP interface.");
				}
			}
		}
	};
}
