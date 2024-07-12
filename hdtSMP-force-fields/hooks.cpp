#include "ForceFieldManager.h"
#include "Objects.h"

namespace Hooks
{
	struct TESObjectREFR__Load3D
	{
		static RE::NiAVObject* thunk(RE::TESObjectREFR* _this, bool bkgnd)
		{
			auto result = func(_this, bkgnd);
			if (result) {
				jg::ForceFieldManager::get().onAttach(result, _this);
			}
			return result;
		}
		static inline decltype(&thunk) func;
	};

	struct TESObjectREFR__Release3D
	{
		static void thunk(RE::TESObjectREFR* _this)
		{
			assert(_this);

			if (auto model = _this->Get3D()) {
				jg::ForceFieldManager::get().onDetach(model, _this);
			}

			func(_this);
		}
		static inline decltype(&thunk) func;
	};

	struct ModelReferenceEffect__DetachImpl
	{
		static void thunk(RE::ModelReferenceEffect* _this)
		{
			assert(_this);

			if (auto model = _this->Get3D()) {
				jg::ForceFieldManager::get().onDetach(model, _this);
			}
			func(_this);
		}
		static inline decltype(&thunk) func;
	};

	struct ModelReferenceEffect__Unk57bcc0
	{
		//This is part of the update procedure and will run every frame.
		//Ideally, we should move the hook to +0x86, which is where the attaching begins.
		//That may be less portable, though.
		static std::uint64_t thunk(RE::ModelReferenceEffect* _this)
		{
			assert(_this);

			RE::NiAVObject* old_model = _this->Get3D();

			std::uint64_t result = func(_this);

			if (!old_model) {
				if (auto model = _this->Get3D()) {
					jg::ForceFieldManager::get().onAttach(model, _this);
				}
			}

			return result;
		}
		static inline decltype(&thunk) func;
	};

	bool Install()
	{
		if (LONG res = DetourTransactionBegin() != NO_ERROR) {
			logger::error("DetourTransactionBegin failed with error code {}", res);
			return false;
		}

		if(!stl::detour_func<TESObjectREFR__Load3D>(REL::RelocationID(19300, 19727))) return false;
		if(!stl::detour_func<TESObjectREFR__Release3D>(REL::RelocationID(19301, 19728))) return false;
		if(!stl::detour_func<ModelReferenceEffect__DetachImpl>(REL::RelocationID(33869, 34665))) return false;
		if(!stl::detour_func<ModelReferenceEffect__Unk57bcc0>(REL::RelocationID(33873, 34669))) return false;

		if (LONG res = DetourTransactionCommit() != NO_ERROR) {
			logger::error("DetourTransactionCommit failed with error code {}", res);
			return false;
		}

		return true;
	}
}
