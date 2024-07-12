#include "utils.h"

thread_local std::minstd_rand g_rng;

float jg::randf(float min, float max)
{
	assert(max > min);
	std::uniform_real_distribution<float> d(min, max);
	return d(g_rng);
}

btVector3 jg::toWorld(RE::NiAVObject* parent, const btVector3& translation)
{
	if (parent) {
		auto&& t = parent->world.translate;
		auto&& A = parent->world.rotate;

		//result = parent position + parent scale * parent rotation * translation

		RE::NiPoint3 res = A * RE::NiPoint3(translation[0], translation[1], translation[2]);

		res *= parent->world.scale;
		res += t;

		return btVector3(res.x, res.y, res.z);
	}
	else {
		return translation;
	}
}

btMatrix3x3 jg::toWorld(RE::NiAVObject* parent, const btMatrix3x3& rotation)
{
	if (parent) {
		auto&& A = parent->world.rotate;

		return btMatrix3x3(
			A.entry[0][0], A.entry[0][1], A.entry[0][2],
			A.entry[1][0], A.entry[1][1], A.entry[1][2],
			A.entry[2][0], A.entry[2][1], A.entry[2][2]) * rotation;
	}
	else {
		return rotation;
	}
}

float jg::toWorld(RE::NiAVObject* parent, float scale)
{
	if (parent) {
		return parent->world.scale * scale;
	}
	else {
		return scale;
	}
}
