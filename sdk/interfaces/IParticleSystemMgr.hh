#pragma once

#include "../../utilities/memory.h"

#include "../datatypes/stronghandle.h"
#include "IParticleSnapshot.hh"

class IParticleSystemMgr
{
public:
	auto CreateSnapshot(CStrongHandle<IParticleSnapshot>* out_particle_snapshot) -> void
	{
		__int64 nUnknown = 0;
		MEM::CallVFunc<void, 42U > (this, out_particle_snapshot, &nUnknown);
	}

	auto Draw(CStrongHandle<IParticleSnapshot>* particle_snapshot, int count, void* data) -> void
	{
		MEM::CallVFunc<void, 43U>(this, particle_snapshot, count, data);
	}
};
