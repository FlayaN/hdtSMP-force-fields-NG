#pragma once

#include "obj_traits.h"
#include "utils.h"

namespace jg
{
	template<>
	struct obj_traits<RE::TESObjectREFR>
	{
		constexpr static bool hasAge() { return false; }
		constexpr static float age(const RE::TESObjectREFR&) { return 0.0f; }

		constexpr static float duration(const RE::TESObjectREFR&) { return -1.0f; }
		constexpr static bool expired(const RE::TESObjectREFR&) { return false; }

		constexpr static float force(const RE::TESObjectREFR&) { return 0.0f; }

		constexpr static float length(const RE::TESObjectREFR&) { return 0.0f; }
		constexpr static float radius(const RE::TESObjectREFR&) { return 0.0f; }

		static btVector3 translate(const RE::TESObjectREFR& obj, const btVector3& t)
		{
			return toWorld(obj.Get3D(), t);
		}
		static btMatrix3x3 rotate(const RE::TESObjectREFR& obj, const btMatrix3x3& r)
		{
			return toWorld(obj.Get3D(), r);
		}
		static float scale(const RE::TESObjectREFR& obj, float s)
		{
			return toWorld(obj.Get3D(), s);
		}

		static bool isAttached(const RE::TESObjectREFR& obj);
	};
}
