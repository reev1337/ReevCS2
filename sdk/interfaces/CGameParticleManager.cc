#include "CGameParticleManager.hh"

#include "../../common.h"

#include "../datatypes/usercmd.h"



void CGameParticleManager::CreateEffectIndex(unsigned int* effect_index, particle_effect* effect_data)
{
	using fnCreateEffectIndex = void(__fastcall*)(CGameParticleManager*, unsigned int*, particle_effect*);
	static const fnCreateEffectIndex oCreateEffectIndex = reinterpret_cast<fnCreateEffectIndex>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 57 48 83 EC 20 49 8B ?? 48 8B")));
	oCreateEffectIndex(this, effect_index, effect_data);
}

void CGameParticleManager::SetEffect(unsigned int effect_index, int effect_parametr_index, void* parametr, int unk2)
{
	using fnSetEffect = void(__fastcall*)(CGameParticleManager*, unsigned int, int, void*, int);
	static const fnSetEffect oSetEffect = reinterpret_cast<fnSetEffect>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ?? 48 89 74 24 10 57 48 83 EC ?? ?? ?? ?? ?? ?? ?? ?? ?? 41 8B F8 8B DA 4C")));
	oSetEffect(this, effect_index, effect_parametr_index, parametr, unk2);
}

void CGameParticleManager::fnInitEffect(int effect_index, unsigned int unk, const CStrongHandle<IParticleSnapshot>* particle_snapshot)
{
	using fnInitEffect = bool(__fastcall*)(CGameParticleManager*, int, unsigned int, const CStrongHandle<IParticleSnapshot>*);
	static const fnInitEffect ofnInitEffect = reinterpret_cast<fnInitEffect>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 74 24 10 57 48 83 EC 30 4C 8B D9 49 8B F9 33 C9 41 8B F0 83 FA FF 0F")));
	ofnInitEffect(this, effect_index, unk, particle_snapshot);
}
