#pragma once

namespace jg
{
	class Settings : public clib_util::singleton::ISingleton<Settings>
	{
	public:
		bool LoadSettings();

		bool bLogStatistics{ false };
		float fForceMultiplier{ 1.0f };
		int iThreads{ 8 };
	};
}
