#pragma once
#include "ObjRef_traits.h"

namespace jg
{
	template<>
	struct obj_traits<RE::Explosion> : obj_traits<RE::TESObjectREFR>
	{
		constexpr static bool hasAge() { return true; }
		static float age(const RE::Explosion& obj) { return obj.startKeyTime; }

		//static float duration(const Explosion& obj) { return obj.getDuration(); }//explosion duration is not used?

		static float force(const RE::Explosion& obj)
		{
			auto* objRef = obj.GetObjectReference();
			auto* base = objRef ? objRef->As<RE::BGSExplosion>() : nullptr;
			return base ? base->data.force : 0.0f;
		}

		static float length(const RE::Explosion& obj) { return radius(obj); }
		static float radius(const RE::Explosion& obj) { return obj.radius; }
	};
}
