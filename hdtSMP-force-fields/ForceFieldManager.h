#pragma once

#include <barrier>
#include <shared_mutex>
#include <semaphore>

#include "IForceField.h"

namespace jg
{
	class ForceFieldManager final : public hdt::IPreStepListener
	{
	private:
		using Bodies = std::remove_const_t<std::remove_reference_t<decltype(hdt::PreStepEvent::objects)>>;
		using Fields = std::vector<std::shared_ptr<IForceField>, Skyrim::allocator<std::shared_ptr<IForceField>>>;

		class WorkerPool
		{
		public:
			WorkerPool(int n);
			~WorkerPool();

			void wait();
			void release(const Fields& fields, const Bodies& bodies);

			explicit operator bool() const { return !m_workers.empty(); }

		private:
			static void barrierComplete() noexcept;
			void work();

		private:
			std::vector<std::thread> m_workers;

			std::counting_semaphore<128> m_startSignal{ 0 };
			std::binary_semaphore m_stopSignal{ 0 };
			std::barrier<void(*)() noexcept> m_barrier;

			const Bodies* m_bodies{ nullptr };
			const Fields* m_fields{ nullptr };

			std::atomic<int> m_jobCounter{ 0 };
		};

	public:
		static ForceFieldManager& get()
		{
			static ForceFieldManager instance;
			return instance;
		}

	public:
		ForceFieldManager();
		~ForceFieldManager() = default;

		virtual void onEvent(const hdt::PreStepEvent& e) override;

		template<typename T> void onAttach(RE::NiAVObject* model, T* owner);
		void onDetach(RE::NiAVObject* model, void* owner);

	private:
		using ObjectAllocator = Skyrim::allocator<std::pair<void* const, IForceField*>>;
		using ObjectMap = std::map<void*, IForceField*, std::less<void*>, ObjectAllocator>;

		ObjectMap::const_iterator	findEntry(void* object) const;
		IForceFieldFactory*			getFactory(RE::NiStringsExtraData* data) const;
		ParamMap					getFieldParameters(RE::NiStringsExtraData* data) const;
		RE::NiStringsExtraData*	getFieldData(RE::NiAVObject* model) const;
		bool						hasEntry(void* object) const;
		void						insertField(void* owner, std::unique_ptr<IForceField>&& field);
		ParamMap::value_type		parseParamStr(const char* str) const;

	private:
		mutable std::shared_mutex m_mutex;

		//Maps objects to their bound force fields.
		//Read and write is protected by m_mutex. Attaching and detaching different objects is thread safe.
		//However, it is assumed that concurrent attachments and detachments of the SAME object will never happen.
		ObjectMap m_fieldMap;

		//The actual fields, for rapid iteration
		Fields m_fields;
		Fields m_active;

		//Never changed outside of init, so this requires no synchronisation
		std::map<std::string, std::unique_ptr<IForceFieldFactory>, CaseInsensitiveComparator> m_factories;

		RE::BSFixedString m_dataName;

		WorkerPool m_workerPool;
	};

	template<typename T>
	inline void jg::ForceFieldManager::onAttach(RE::NiAVObject* model, T* owner)
	{
		//assert(model);
		//logger::debug("Attaching {}", model->name);

		if (auto data = getFieldData(model)) {
			if (!hasEntry(owner)) {
				if (auto factory = getFactory(data)) {
					insertField(owner, factory->create(owner, getFieldParameters(data)));
				}
			}
		}
	}
}
