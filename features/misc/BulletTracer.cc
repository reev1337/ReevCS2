#include "BulletTracer.hh"

void F::MISC::BULLETTRACER::AddBulletTrace(Vector_t start, Vector_t end, Color_t clr_)
{
	if (!I::pGameParticleManager) {
		using fnGetGameParticlemanager = CGameParticleManager* (__fastcall*)();
		static const fnGetGameParticlemanager oGetGameParticlemanager = reinterpret_cast<fnGetGameParticlemanager>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8b 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 57")));
		I::pGameParticleManager = oGetGameParticlemanager();
	}

	tracer_info bullet;

	particle_effect particle_effect_{};
	particle_effect_.name = "particles/entity/spectator_utility_trail.vpcf";
	particle_effect_.pad[0] = 8;
	I::pGameParticleManager->CreateEffectIndex(&bullet.effect_index, &particle_effect_);
	particle_color clr = { float(clr_.r), float(clr_.g), float(clr_.b) };
	I::pGameParticleManager->SetEffect(bullet.effect_index, 16, &clr, 0);
	bullet.particle_data_ = {};

	auto dir = (end - start);
	auto stage_2 = start + (dir * 0.3f);
	auto stage_3 = start + (dir * 0.5f);

	Vector_t positions_[] = { start, stage_2, stage_3, end };

	for (int i{}; i < sizeof(positions_) / sizeof(Vector_t); i++)
	{
		particle_information particle_info{};
		particle_info.time = 4.f;
		particle_info.width = 2.f;
		particle_info.unk2 = 1.f;
		I::pGameParticleManager->SetEffect(bullet.effect_index, 3, &particle_info, 0);

		bullet.positions = new Vector_t[i + 1];
		bullet.times = new float[i + 1];

		for (int j{}; j < i + 1; j++)
		{
			bullet.positions[j] = positions_[j];
			bullet.times[j] = 0.015625f * float(j);
		}

		bullet.particle_data_.positions = bullet.positions;
		bullet.particle_data_.times2 = bullet.times;

		I::pParticleSystemMgr->CreateSnapshot(&bullet.handle_snapshot_particle);

		I::pGameParticleManager->fnInitEffect(bullet.effect_index, 0, &bullet.handle_snapshot_particle);
		I::pParticleSystemMgr->Draw(&bullet.handle_snapshot_particle, i + 1, &bullet.particle_data_);

		delete[] bullet.positions;
		delete[] bullet.times;
	}
}
