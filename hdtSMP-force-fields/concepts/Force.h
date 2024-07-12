#pragma once

#include "Settings.h"
#include "IForceField.h"
#include "Objects.h"
#include "utils.h"

namespace jg
{
	class Force
	{
	public:
		struct ParamMapping
		{
			std::string fForce{ "fForce" };

			template<typename T>
			void operator()(Force& target, T* owner, const ParamMap& params) const
			{
				if (auto it = params.find(fForce); it != params.end()) {
					target.m_intensity = it->second.f;
				}
				else {
					assert(owner);
					target.m_intensity = obj_traits<T>::force(*owner);
				}

				logger::debug("Force: {}", target.m_intensity);
			}
		};

	public:
		Force() = default;
		~Force() = default;

		float getForce() const { return Settings::GetSingleton()->fForceMultiplier * m_intensity; }

		template<typename T>
		void update(T*, float) {}

	protected:
		float m_intensity;
	};

	class Decaying
	{
	public:
		struct ParamMapping
		{
			std::string fRange{ "fRange" };

			template<typename T>
			void operator()(Decaying& target, T*, const ParamMap& params) const
			{
				if (auto it = params.find(fRange); it != params.end() && it->second.f > 0.0f) {
					//interpret range as half-life, in some user-defined units of the geometry size
					target.m_decay = 0.6931472f / it->second.f;
				}
				else {
					target.m_decay = 0.0f;
				}

				logger::debug("Decay: {}", target.m_decay);
			}
		};

	public:
		Decaying() = default;
		~Decaying() = default;

		float getDecay(float x) const { return std::exp(-m_decay * x); }
		bool hasDecay() const { return m_decay > 0.0f; }

		template<typename T>
		void update(T*, float) {}

	private:
		float m_decay;
	};


	class Noisy
	{
	public:
		struct ParamMapping
		{
			std::string fNoise{ "fNoise" };

			template<typename T>
			void operator()(Noisy& target, T*, const ParamMap& params) const
			{
				if (auto it = params.find(fNoise); it != params.end() && it->second.f >= 0.0f) {
					target.m_noise = it->second.f;
				}
				else {
					target.m_noise = 0.0f;
				}

				logger::debug("Noise: {}", target.m_noise);
			}
		};

	public:
		Noisy() = default;
		~Noisy() = default;

		bool hasNoise() const { return m_noise != 0.0f; }

		btVector3 noiseVector() const
		{
			if (m_noise != 0.0f) {
				return btVector3(randf(-m_noise, m_noise), randf(-m_noise, m_noise), randf(-m_noise, m_noise));
			}
			else {
				return btVector3(0.0f, 0.0f, 0.0f);
			}
		}
		
		template<typename T>
		void update(T*, float) {}

	private:
		float m_noise;
	};
}
