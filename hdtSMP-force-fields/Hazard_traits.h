#pragma once

#include "ObjRef_traits.h"

namespace jg
{
	template<>
	struct obj_traits<RE::Hazard> : obj_traits<RE::TESObjectREFR>
	{
		constexpr static bool hasAge() { return true; }
		static float age(const RE::Hazard& obj) { return obj.GetHazardRuntimeData().age; }

		static float duration(const RE::Hazard& obj) { return obj.GetHazardRuntimeData().lifetime; }

		static float length(const RE::Hazard& obj) { return radius(obj); }

		//Hazard radius uses UI units, not game units (inconsistent with Explosion and Projectile). 
		//The conversion factor appears to be 22.
		static float radius(const RE::Hazard& obj) { return 22.0f * obj.GetHazardRuntimeData().radius; }
	};
}
