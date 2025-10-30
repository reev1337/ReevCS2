#pragma once

#include "../../sdk/datatypes/vector.h"
#include "../../sdk/datatypes/stronghandle.h"

#include "IParticleSnapshot.hh"

class particle_effect
{
public:
	const char* name{};
	char pad[0x30]{};
};

class particle_information
{
public:
	float time{};
	float width{};
	float unk2{};
};

class particle_data
{
public:
	Vector_t* positions{};
	char pad[0x74]{};
	float* times{};
	void* unk_ptr{};
	char pad2[0x28];
	float* times2{};
	char pad3[0x98];
	void* unk_ptr2{};
};

struct particle_color
{
	float r;
	float g;
	float b;
};

class CGameParticleManager
{
public:
	void CreateEffectIndex(unsigned int* effect_index, particle_effect* effect_data);
	void SetEffect(unsigned int effect_index, int unk, void* clr, int unk2);
	void fnInitEffect(int effect_index, unsigned int unk, const CStrongHandle<IParticleSnapshot>* handle);
};
