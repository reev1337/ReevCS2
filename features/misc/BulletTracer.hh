#pragma once

#include "../../common.h"
#include "../../sdk/datatypes/vector.h"
#include "../../sdk/datatypes/color.h"

#include "../../core/interfaces.h"
#include "../../sdk/interfaces/CGameParticleManager.hh"
#include "../../sdk/interfaces/IParticleSystemMgr.hh"

class tracer_info
{
public:
	unsigned int effect_index = -1;
	Vector_t* positions = nullptr;
	float* times = nullptr;
	CStrongHandle<IParticleSnapshot> handle_snapshot_particle{};
	particle_data particle_data_;
};

namespace F::MISC::BULLETTRACER
{
	void AddBulletTrace(Vector_t start, Vector_t end, Color_t clr_);
}
