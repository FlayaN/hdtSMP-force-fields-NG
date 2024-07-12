#pragma once

constexpr float PI = 3.1415927f;

namespace jg
{
	float randf(float min = 0.0f, float max = 1.0f);

	btVector3 toWorld(RE::NiAVObject* parent, const btVector3& translation);
	btMatrix3x3 toWorld(RE::NiAVObject* parent, const btMatrix3x3& rotation);
	float toWorld(RE::NiAVObject* parent, float scale);

	struct CaseInsensitiveComparator
	{
		bool operator()(const std::string& a, const std::string& b) const noexcept
		{
			return stricmp(a.c_str(), b.c_str()) < 0;
		}
	};
}
