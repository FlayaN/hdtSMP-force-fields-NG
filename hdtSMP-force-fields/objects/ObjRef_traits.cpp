#include "ObjRef_traits.h"

bool jg::obj_traits<RE::TESObjectREFR>::isAttached(const RE::TESObjectREFR& obj)
{
	return obj.GetParentCell() && obj.GetParentCell()->cellState.get() == RE::TESObjectCELL::CellState::kAttached;
}
