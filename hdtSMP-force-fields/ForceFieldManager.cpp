#include "ForceFieldManager.h"

#include "Settings.h"
#include "Timer.h"
#include "Bomb.h"
#include "Cylinder.h"
#include "Plane.h"
#include "Sphere.h"
#include "Vortex.h"

#define LOG_TIMER

jg::ForceFieldManager::ForceFieldManager() :
	m_workerPool(Settings::GetSingleton()->iThreads)
{
	m_dataName = "HDT-SMP Force Field";

	m_factories["CYLINDRICAL"] = std::make_unique<CylinderFactory>();
	m_factories["PLANAR_BOX"] = std::make_unique<BoxFactory>();
	m_factories["PLANAR_CYL"] = std::make_unique<RodFactory>();
	m_factories["PLANAR_SPH"] = std::make_unique<BallFactory>();
	m_factories["SPHERICAL"] = std::make_unique<SphereFactory>();
	m_factories["VORTEX"] = std::make_unique<VortexFactory>();

	//Bomb is unfinished and janky, leave it out for now
	//m_factories["BOMB"] = std::make_unique<BombFactory>();

	logger::info("Field types:");
	for (auto&& item : m_factories) {
		logger::info("{}", item.first.c_str());
	}
}

void jg::ForceFieldManager::onEvent(const hdt::PreStepEvent& e)
{
#ifdef LOG_TIMER
	static std::size_t fields = 0;
	static int objects = 0;
	static int steps = 0;
	static long long time = 0;
	static int multiCount = 0;

	Timer<long long, std::micro> timer;
#endif

	static float lastSingle = 1.0f;
	static float lastMulti = 1.0f;

	{
		//Update and make a working copy of the active fields, then release lock immediately
		std::shared_lock<decltype(m_mutex)> lock(m_mutex);

		for (auto&& field : m_fields) {
			if (field->update(e.timeStep)) {
				m_active.push_back(field);
			}
		}
	}

	Timer<float, std::micro> newTimer;

	if (!m_active.empty() && e.objects.size() != 0) {
		//We dynamically select the fastest threading policy, based on previous performance.
		if (m_workerPool && (lastMulti || lastSingle) && randf() < lastSingle / (lastMulti + lastSingle)) {
			m_workerPool.release(m_active, e.objects);
			m_workerPool.wait();

			//Some arbitrary power to push towards the faster method
			lastMulti = std::pow(newTimer.elapsed(), 3.0f);

			//If any one frame takes extremely long, we may become permanently locked out of that method.
			//Decaying the time for the unused method guards against this.
			//This factor should force reevaluation within seconds.
			lastSingle *= 0.99f;

#ifdef LOG_TIMER
			multiCount++;
#endif
		}
		else {
			for (int i = 0; i < e.objects.size(); i++) {
				btRigidBody* body = btRigidBody::upcast(e.objects[i]);
				if (body && !body->isStaticOrKinematicObject()) {
					for (auto&& field : m_active) {
						field->actOn(*body);
					}
				}
			}
			lastSingle = std::pow(newTimer.elapsed(), 3.0f);

			lastMulti *= 0.99f;
		}
	}

#ifdef LOG_TIMER
	if (Settings::GetSingleton()->bLogStatistics) {
		fields += m_active.size();
		objects += e.objects.size();
		time += timer.elapsed();
		steps++;

		if (steps == 120) {
			logger::info("Active fields: {}\tCollision objects: {}\tFraction multithreaded: {:.2f}\tUpdate time: {} microseconds",
				fields / 120, objects / 120, (float)multiCount / 120, time / 120);
			fields = 0;
			objects = 0;
			steps = 0;
			time = 0;
			multiCount = 0;
		}
	}
#endif

	m_active.clear();
}

void jg::ForceFieldManager::onDetach(RE::NiAVObject* model, void* owner)
{
	// assert(model);
	// logger::info("Detaching {}", model->name);

	if (auto entry = findEntry(owner); entry != m_fieldMap.end()) {
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (entry->second) {
			auto field = std::find_if(m_fields.begin(), m_fields.end(),
				[=](const std::shared_ptr<IForceField>& ptr) { return ptr.get() == entry->second; });
			//We don't need this check unless our data has been corrupted. Should be impossible right now.
			assert(field != m_fields.end());
			m_fields.erase(field);
		}
		m_fieldMap.erase(entry);
	}
}

jg::ForceFieldManager::ObjectMap::const_iterator jg::ForceFieldManager::findEntry(void* object) const
{
	if (object) {
		std::shared_lock<decltype(m_mutex)> lock(m_mutex);
		return m_fieldMap.find(object);
	}
	return m_fieldMap.end();
}

jg::IForceFieldFactory* jg::ForceFieldManager::getFactory(RE::NiStringsExtraData* data) const
{
	assert(data);

	if (data && data->value && data->size > 0) {
		std::vector<RE::BSFixedString> vec({ data->value, data->value + data->size });

		for (const auto& strData : vec)
		{
			if (auto it = m_factories.find(strData.c_str()); it != m_factories.end()) {
				return it->second.get();
				//logger::debug("Field type: {} ({})", data->m_data[0], data->m_data[0]);
			}
			else {
				//logger::info("Unknown field type");
			}
		}
	}
	else {
		//logger::info("Field data is missing a type string");
	}

	return nullptr;
}

RE::NiStringsExtraData* jg::ForceFieldManager::getFieldData(RE::NiAVObject* model) const
{
	assert(model);

	return model->GetExtraData<RE::NiStringsExtraData>(m_dataName);
}

jg::ParamMap jg::ForceFieldManager::getFieldParameters(RE::NiStringsExtraData* data) const
{
	assert(data && data->size != 0);

	ParamMap result;

	std::vector<RE::BSFixedString> vec({ data->value, data->value + data->size });

	for (const auto& strData : vec)
	{
		result.insert(parseParamStr(strData.c_str()));
	}

	return result;
}

bool jg::ForceFieldManager::hasEntry(void* object) const
{
	if (object) {
		std::shared_lock<decltype(m_mutex)> lock(m_mutex);
		return m_fieldMap.contains(object);
	}
	return false;
}

void jg::ForceFieldManager::insertField(void* owner, std::unique_ptr<IForceField>&& field)
{
	if (field && owner) {
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);

		m_fieldMap.insert({ owner, field.get() });
		m_fields.push_back(std::move(field));
	}
}

jg::ParamMap::value_type jg::ForceFieldManager::parseParamStr(const char* str) const
{
	assert(str);

	//split each string into key:value pairs at an '=', if found
	if (auto eq = std::strchr(str, '='); eq && eq != str) {
		char buf[64];
		if (uintptr_t diff = eq - str; diff < sizeof(buf)) {
			if (!strncpy_s(buf, str, diff)) {
				buf[diff] = '\0';

				ParamMap::value_type result{ buf, ParamVal() };

				switch (buf[0]) {
				case 'b':
					result.second.b = static_cast<bool>(std::atoi(eq + 1));
					//logger::info("{}: {}", result.first.c_str(), result.second.b ? "true" : "false");
					return result;
				case 'f':
					result.second.f = static_cast<float>(std::atof(eq + 1));
					//logger::info("{}: {}", result.first.c_str(), result.second.f);
					return result;
				case 'i':
					result.second.i = std::atoi(eq + 1);
					//logger::info("{}: {}", result.first.c_str(), result.second.i);
					return result;
				case 'q':
				{
					const char* next = eq + 1;
					char* end;
					for (int i = 0; i < 4; i++) {
						float f = std::strtof(next, &end);
						if (end != next) {
							result.second.v[i] = f;
							next = end;
						}
						else {
							return { std::string(), ParamVal() };
						}
					}
					return result;
				}
				case 'v':
				{
					const char* next = eq + 1;
					char* end;
					for (int i = 0; i < 3; i++) {
						float f = std::strtof(next, &end);
						if (end != next) {
							result.second.v[i] = f;
							next = end;
						}
						else {
							//logger::info("{}: invalid vector", result.first.c_str());
							return { std::string(), ParamVal() };
						}
					}
					//logger::info("{}: ({}, {}, {})", result.first.c_str(), 
					//	result.second.v[0], result.second.v[1], result.second.v[2]);
					return result;
				}
				default:
					//logger::info("{}: NULL", result.first.c_str());
					;
				}
			}
		}
	}
	return { std::string(), ParamVal() };
}

void jg::ForceFieldManager::WorkerPool::barrierComplete() noexcept
{
	try {
		//Really stupid way of accessing the semaphore, but what to do?
		//The completion function must be nothrow_invocable with no arguments.
		//This rules out non-static member functions and std::function.
		//It must also have an explicit type, which rules out lambda and bind expressions.
		//We are left with static member functions and free functions.
		ForceFieldManager::get().m_workerPool.m_stopSignal.release();
	}
	//counting_semaphore::release can throw std::system_error. 
	//ForceFieldManager::get could hypothetically throw from the ctor of ForceFieldManager,
	//but this should not be the first call to it.
	catch (const std::exception& e) {
		logger::error("ForceFieldManager::WorkerPool::barrierComplete error: {}", e.what());
	}
}

jg::ForceFieldManager::WorkerPool::WorkerPool(int n) :
	m_workers(std::min(n > 1 ? n : 0, 128)),
	m_barrier(std::min(n > 1 ? n : 0, 128), &barrierComplete)
{
	for (auto&& worker : m_workers) {
		worker = std::thread(&WorkerPool::work, this);
	}
	logger::trace("{} worker threads started.", m_workers.size());
}

jg::ForceFieldManager::WorkerPool::~WorkerPool()
{
	m_bodies = nullptr;
	m_fields = nullptr;
	m_startSignal.release(m_workers.size());
	for (auto&& worker : m_workers) {
		worker.join();
	}
}

void jg::ForceFieldManager::WorkerPool::wait()
{
	m_stopSignal.acquire();
	m_bodies = nullptr;
	m_fields = nullptr;
}

void jg::ForceFieldManager::WorkerPool::release(const Fields& fields, const Bodies& bodies)
{
	m_bodies = &bodies;
	m_fields = &fields;
	m_jobCounter.store(bodies.size(), std::memory_order::relaxed);
	m_startSignal.release(m_workers.size());
}

void jg::ForceFieldManager::WorkerPool::work()
{
	while (true) {
		m_startSignal.acquire();

		if (!m_bodies || !m_fields) {
			break;
		}

		try {
			int i = m_jobCounter.fetch_sub(1, std::memory_order_relaxed) - 1;
			while (i >= 0) {
				btRigidBody* body = btRigidBody::upcast(m_bodies->at(i));
				if (body && !body->isStaticOrKinematicObject()) {
					for (auto&& field : *m_fields) {
						field->actOn(*body);
					}
				}
				i = m_jobCounter.fetch_sub(1, std::memory_order_relaxed) - 1;
			}
		}
		catch (const std::exception& e) {
			logger::error("ForceFieldManager::WorkerPool::work error: {}", e.what());
		}

		m_barrier.arrive_and_wait();
	}
}
