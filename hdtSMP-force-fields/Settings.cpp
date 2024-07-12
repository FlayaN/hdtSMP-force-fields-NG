#include "Settings.h"

bool jg::Settings::LoadSettings()
{
	const auto plugin{ SKSE::PluginDeclaration::GetSingleton() };

	const auto path = std::format("Data/SKSE/Plugins/{}.ini", plugin->GetName());

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path.c_str());

	clib_util::ini::get_value(ini, bLogStatistics, "General", "bLogStatistics", nullptr);

	clib_util::ini::get_value(ini, fForceMultiplier, "ForceFields", "fForceMultiplier", nullptr);

	clib_util::ini::get_value(ini, iThreads, "Performance", "iThreads", nullptr);

	(void)ini.SaveFile(path.c_str());

	return true;
}
