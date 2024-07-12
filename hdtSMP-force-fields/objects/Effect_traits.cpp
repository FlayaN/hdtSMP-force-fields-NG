#include "Effect_traits.h"
#include "ObjRef_traits.h"

bool jg::obj_traits<RE::ModelReferenceEffect>::isAttached(const RE::ModelReferenceEffect& obj)
{
	if (auto target = obj.target.get()) {
		return obj_traits<RE::TESObjectREFR>::isAttached(*target);
	}
	else {
		return false;
	}
}
