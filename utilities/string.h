#pragma once
#include <string>
#include <stringapiset.h>
#include <fileapi.h>

#include "pe64.h"
#include "log.h"

enum DirectionType : int
{
	PlayerModels,
	WeaponModels,
	SkyboxModels,
};

namespace STR
{
	static wchar_t* Str2WC(std::string strString)
	{
		int iLength = MultiByteToWideChar(CP_UTF8, 0, strString.c_str(), static_cast<int>(strString.size()), NULL, 0);
		wchar_t* wszString = new wchar_t[iLength + 1];

		MultiByteToWideChar(CP_UTF8, 0, strString.c_str(), static_cast<int>(strString.size()), wszString, iLength);
		wszString[iLength] = L'\0';

		return wszString;
	}

	static std::string WC2Str(const wchar_t* wszString)
	{
		int iLength = WideCharToMultiByte(CP_UTF8, 0, wszString, -1, nullptr, 0, nullptr, nullptr);
		char* szString = new char[iLength];

		WideCharToMultiByte(CP_UTF8, 0, wszString, -1, szString, iLength, nullptr, nullptr);
		return std::string(szString);
	}

	static std::string GetDirectory()
	{
		const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

		for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
		{
			const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

			std::string strzModuleName = STR::WC2Str(pEntry->BaseDllName.Buffer);
			std::string strModuleFullName = STR::WC2Str(pEntry->FullDllName.Buffer);

			if (strcmp(strzModuleName.c_str(), "client.dll") == 0)
			{
				size_t binPos = strModuleFullName.find("csgo");
				size_t clientDllPos = strModuleFullName.find("client.dll", binPos);

				strModuleFullName.erase(binPos, clientDllPos - binPos + std::string("client.dll").length());

				return strModuleFullName;
			}
		}

		return "";
	}

	static std::string GetGameDirectory()
	{
		std::string strDirectory = GetDirectory();
		return strDirectory + CS_XOR("bin\\win64\\");
	}

	static std::string GetDirectory(DirectionType eType)
	{
		std::string strDirectory = GetDirectory();

		switch (eType)
		{
		case PlayerModels:
			return strDirectory + CS_XOR("csgo\\characters\\models\\");
		case WeaponModels:
			return strDirectory + CS_XOR("csgo\\weapons\\");
		case SkyboxModels:
			return strDirectory + CS_XOR("csgo\\materials\\skybox\\");
		}
	}

	static bool CreateGameDirectory(std::string strFolder, std::string strName)
	{
		if (!CreateDirectoryW(STR::Str2WC(strFolder + strName), nullptr))
		{
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
				L_PRINT(LOG_INFO) << CS_XOR("directory \"") << strName.c_str() << CS_XOR("\" has already been created.");
			else
				return false;
		}
		else
			L_PRINT(LOG_INFO) << CS_XOR("directory \"") << strName.c_str() << CS_XOR("\" has been created.");

		return true;
	}

	static bool CreateDirectories()
	{
		bool bSuccess = true;

		std::string strGameDirectory = GetGameDirectory();
		if (strGameDirectory.empty())
			return false;

		bSuccess &= CreateGameDirectory(strGameDirectory, CS_XOR("euphoria"));
		strGameDirectory += CS_XOR("euphoria\\");
		bSuccess &= CreateGameDirectory(strGameDirectory, CS_XOR("configs"));
		bSuccess &= CreateGameDirectory(strGameDirectory, CS_XOR("sounds"));
		bSuccess &= CreateGameDirectory(strGameDirectory, CS_XOR("scripts"));
		strGameDirectory += CS_XOR("scripts\\");
		bSuccess &= CreateGameDirectory(strGameDirectory, CS_XOR("libs"));

		std::string strDirectory = GetDirectory();

		strGameDirectory += CS_XOR("csgo\\");
		bSuccess &= CreateGameDirectory(strDirectory, CS_XOR("weapons"));

		bSuccess &= CreateGameDirectory(strDirectory, CS_XOR("materials"));
		bSuccess &= CreateGameDirectory(strDirectory + CS_XOR("materials\\"), CS_XOR("skybox"));

		bSuccess &= CreateGameDirectory(strDirectory, CS_XOR("characters"));
		bSuccess &= CreateGameDirectory(strDirectory + CS_XOR("characters\\"), CS_XOR("models"));

		return bSuccess;
	}
}
