#pragma once

#include "ObjRef_traits.h"

namespace jg
{
	template<>
	struct obj_traits<RE::Projectile> : obj_traits<RE::TESObjectREFR>
	{
		constexpr static RE::FormType BASE_TYPE = RE::FormType::Projectile;

		constexpr static bool hasAge() { return true; }
		static float age(const RE::Projectile& obj) { return obj.GetProjectileRuntimeData().livingTime; }

		static float force(const RE::Projectile& obj)
		{
			auto base = obj.GetProjectileBase();
			return base ? base->data.force : 0.0f;
		}

		static float length(const RE::Projectile& obj) { return radius(obj); }
		static float radius(const RE::Projectile& obj)
		{
			auto base = obj.GetProjectileBase();
			return base ? base->data.collisionRadius : 0.0f;
		}

		static float scale(const RE::Projectile& obj, float s)
		{
			return obj_traits<RE::TESObjectREFR>::scale(obj, s) * obj.GetProjectileRuntimeData().scale;
		}
	};

	template<>
	struct obj_traits<RE::ConeProjectile> : obj_traits<RE::Projectile>
	{
		static float getSpread(const RE::ConeProjectile& obj)
		{
			if (auto base = obj.GetProjectileBase()) {
				auto& coneRunetimeData = obj.GetConeRuntimeData();
				auto& projectileRuntimeData = obj.GetProjectileRuntimeData();
				return 1.0f + coneRunetimeData.coneAngleTangent * projectileRuntimeData.livingTime * base->data.speed * projectileRuntimeData.speedMult / coneRunetimeData.initialCollisionSphereRadius;
			}
			else {
				return 1.0f;
			}
		}

		static float scale(const RE::ConeProjectile& obj, float s)
		{
			return obj_traits<RE::Projectile>::scale(obj, s) * getSpread(obj);
		}
	};
}
