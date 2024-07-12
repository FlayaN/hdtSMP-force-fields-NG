#pragma once

#include "IForceField.h"
#include "Objects.h"
#include "utils.h"

namespace jg
{
	template<typename... Concepts>
	struct ParamMapping : Concepts::ParamMapping...
	{};

	template<typename... Concepts>
	class TConcepts : public Concepts...
	{
	public:
		using ParamMapping = jg::ParamMapping<Concepts...>;

	public:
		TConcepts() = default;
		~TConcepts() = default;

		template<typename T>
		void mapParams(T* owner, const ParamMap& params, const ParamMapping& mapping)
		{
			int dummy[sizeof...(Concepts)] = { (static_cast<const Concepts::ParamMapping&>(mapping)(*this, owner, params), 0)... };
		}

		template<typename T>
		void update(T* owner, float dt)
		{
			int dummy[sizeof...(Concepts)] = { (static_cast<Concepts&>(*this).update(owner, dt), 0)... };
		}
	};

	template<typename T, typename Impl>
	class TField : public IForceField
	{
	public:
		template<typename PM>
		TField(T* owner, const ParamMap& params, const PM& pm) :
			m_owner{ owner }
		{
			logger::debug("{}<{}>", typeid(Impl).name(), typeid(T).name());
			m_impl.mapParams(owner, params, pm);
		}

		virtual ~TField() override
		{
			logger::debug("~{}<{}>", typeid(Impl).name(), typeid(T).name());
		}

		virtual void actOn(btRigidBody& body) const override
		{
			m_impl.actOn(body);
		}

		virtual bool update(float dt) override
		{
			assert(m_owner);

			if (obj_traits<T>::isAttached(*m_owner)) {
				m_impl.update(m_owner, dt);
				return true;
			}
			else {
				return false;
			}
		}

		Impl m_impl;
		T* m_owner;
	};

	template<template<typename> typename FieldType, typename ParamMapping>
	class TFactory : public IForceFieldFactory
	{
	public:
		TFactory() = default;
		virtual ~TFactory() override = default;

		[[nodiscard]] virtual std::unique_ptr<IForceField> create(RE::ModelReferenceEffect* owner, const ParamMap& params) override
		{
			return createImpl(owner, params);
		}
		[[nodiscard]] virtual std::unique_ptr<IForceField> create(RE::TESObjectREFR* owner, const ParamMap& params) override
		{
			assert(owner);

			switch (owner->GetFormType()) {
			case RE::Hazard::FORMTYPE:
				return createImpl(static_cast<RE::Hazard*>(owner), params);
			case RE::ConeProjectile::FORMTYPE:
				return createImpl(static_cast<RE::ConeProjectile*>(owner), params);
			case RE::FormType::ProjectileMissile:
			case RE::FormType::ProjectileArrow:
			case RE::FormType::ProjectileGrenade:
			case RE::FormType::ProjectileBeam:
			case RE::FormType::ProjectileFlame:
			case RE::FormType::ProjectileBarrier:
				return createImpl(static_cast<RE::Projectile*>(owner), params);
			default:
				if (auto exp = owner->AsExplosion()) {
					return createImpl(exp, params);
				}
				else {
					return createImpl(owner, params);
				}
			}
		}

		template<typename T>
		std::unique_ptr<FieldType<T>> createImpl(T* owner, const ParamMap& params)
		{
			return std::make_unique<FieldType<T>>(owner, params, paramMapping);
		}

		ParamMapping paramMapping;
	};
}
