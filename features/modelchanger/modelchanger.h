#pragma once

#include <vector>
#include <string>

struct Model_t
{
	std::string strModelName;
	std::string strModelPath;
};

class CModelChanger
{
public:
	std::vector<Model_t> vecPlayerModels;
	std::vector<Model_t> vecWeaponModels;
	unsigned long long nSelectedPlayerModel = ~1U;
	uint32_t uLastPlayerModelHash;

	void UpdateWeaponModels();
	void UpdatePlayerModels();
	bool SetPlayerModel();
};

inline CModelChanger* ModelChanger = new CModelChanger();
