#pragma once

#include "obj_traits.h"
#include "utils.h"

namespace jg
{
	template<>
	struct obj_traits<RE::ModelReferenceEffect>
	{
		constexpr static bool hasAge() { return true; }
		static float age(const RE::ModelReferenceEffect& obj) { return obj.age; }

		constexpr static float duration(const RE::ModelReferenceEffect&) { return -1.0f; }
		static bool expired(const RE::ModelReferenceEffect& obj) { return obj.finished; }

		constexpr static float force(const RE::ModelReferenceEffect&) { return 0.0f; }

		constexpr static float length(const RE::ModelReferenceEffect&) { return 0.0f; }
		constexpr static float radius(const RE::ModelReferenceEffect&) { return 0.0f; }

		static btVector3 translate(const RE::ModelReferenceEffect& obj, const btVector3& t)
		{
			return toWorld(obj.Get3D(), t);
		}
		static btMatrix3x3 rotate(const RE::ModelReferenceEffect& obj, const btMatrix3x3& r)
		{
			return toWorld(obj.Get3D(), r);
		}
		static float scale(const RE::ModelReferenceEffect& obj, float s)
		{
			return toWorld(obj.Get3D(), s);
		}

		static bool isAttached(const RE::ModelReferenceEffect& obj);
	};
}
