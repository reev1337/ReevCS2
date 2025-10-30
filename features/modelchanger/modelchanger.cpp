#include "modelchanger.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

#include "../../sdk/interfaces/iresourcesystem.h"
#include "../../sdk/entity.h"
#include "../../utilities/string.h"
#include "../cstrike/core/sdk.h"
#include "../cstrike/core/interfaces.h"
#include "../cstrike/core/variables.h"
#include "../../sdk/entity.h"

#include "../../sdk/interfaces/iswapchaindx11.h"
#include "../../sdk/interfaces/iviewrender.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/interfaces/iinputsystem.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/inetworkclientservice.h"
#include "../../sdk/interfaces/iglobalvars.h"
#include "../../sdk/interfaces/imaterialsystem.h"


void CModelChanger::UpdateWeaponModels()
{
	std::vector<Model_t> vecWeaponModels;

	std::string strModelsDirectory = STR::GetDirectory(WeaponModels);
	for (const auto& entry : std::filesystem::recursive_directory_iterator(strModelsDirectory))
	{
		if (!entry.is_regular_file())
			continue;

		std::string strFileName = entry.path().filename().string();

		if (strFileName.size() < 6)
			continue;

		if (strFileName.substr(strFileName.size() - 7) != ".vmdl_c")
			continue;

		std::string strModelPath = entry.path().string();

		strModelPath = strModelPath.substr(0, strModelPath.size() - 2);
		std::replace(strModelPath.begin(), strModelPath.end(), '\\', '/');

		size_t pos = strModelPath.find("weapons/");
		if (pos != std::string::npos)
			strModelPath = strModelPath.substr(pos);

		Model_t pModel;
		pModel.strModelName = strFileName.substr(0, strFileName.size() - 8);
		pModel.strModelPath = strModelPath;
		vecWeaponModels.push_back(pModel);
	}

	ModelChanger->vecWeaponModels = vecWeaponModels;
}

void CModelChanger::UpdatePlayerModels()
{
	std::vector<Model_t> vecPlayerModels;

	std::string strModelsDirectory = STR::GetDirectory(PlayerModels);
	for (const auto& entry : std::filesystem::recursive_directory_iterator(strModelsDirectory))
	{
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension().string() != ".vmdl_c")
			continue;

		std::string strFileName = entry.path().filename().string();

		if (strFileName.find("arm") != std::string::npos)
			continue;

		std::string strModelPath = entry.path().string();
		strModelPath = strModelPath.substr(0, strModelPath.size() - 2);
		std::replace(strModelPath.begin(), strModelPath.end(), '\\', '/');

		size_t pos = strModelPath.find("characters/models/");
		if (pos != std::string::npos)
			strModelPath = strModelPath.substr(pos);

		Model_t pModel;
		pModel.strModelName = strFileName.substr(0, strFileName.size() - 8);
		pModel.strModelPath = strModelPath;
		vecPlayerModels.push_back(pModel);
	}

	ModelChanger->vecPlayerModels = vecPlayerModels;
}

bool CModelChanger::SetPlayerModel()
{
	if (!I::Engine->IsInGame() || !I::Engine->IsConnected())
		return false;

	if (!SDK::LocalPawn || !SDK::LocalPawn->IsAlive())
		return false;

	if (C_GET(bool, Vars.bPlayerModelChanger))
	{
		if (nSelectedPlayerModel == ~1U)
			return false;

		if (vecPlayerModels.size() <= nSelectedPlayerModel)
			return false;

		Model_t pModel = vecPlayerModels[nSelectedPlayerModel];
		const char* szModelPath = pModel.strModelPath.c_str();

		auto uPlayerModelHash = FNV1A::Hash(szModelPath);
		if (ModelChanger->uLastPlayerModelHash == uPlayerModelHash)
			return false;

		ModelChanger->uLastPlayerModelHash = uPlayerModelHash;

		I::ResourceSystem->PreCache(szModelPath);
		SDK::LocalPawn->SetModel(szModelPath);
	}

	return true;
}
