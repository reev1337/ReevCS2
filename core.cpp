
#include <shlobj_core.h>
#include "core.h"
#include "features.h"
#include "utilities/crt.h"
#include "utilities/memory.h"
#include "features/modelchanger/modelchanger.h"
#include "utilities/log.h"
#include "utilities/inputsystem.h"
#include "utilities/draw.h"
#include "core/interfaces.h"
#include "core/sdk.h"
#include "core/variables.h"
#include "core/hooks.h"
#include "core/schema.h"
#include "core/convars.h"
#include "core/menu.h"
#include "sdk/interfaces/iengineclient.h"
#include "utilities/notify.h"

	bool blockInsert = true;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && blockInsert && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_INSERT)
	{
		return 1;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void ToggleBlockInsert(bool enable)
{
	blockInsert = enable;
}

std::atomic<bool> g_bShuttingDown = false;

bool CORE::CreateDirectoryRecursive(const wchar_t* path)
{
	wchar_t temp[MAX_PATH];
	wchar_t* pos = nullptr;
	bool result = false;

	CRT::StringCopy(temp, path);

	for (pos = temp + 1; *pos; pos++)
	{
		if (*pos == L'\\')
		{
			*pos = L'\0';
			if (!::CreateDirectoryW(temp, nullptr) && ::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
			*pos = L'\\';
		}
	}

	result = (::CreateDirectoryW(temp, nullptr) != 0) || (::GetLastError() == ERROR_ALREADY_EXISTS);
	return result;
}

bool CORE::GetWorkingPath(wchar_t* wszDestination)
{
	bool bSuccess = false;
	PWSTR wszPathToDocuments = nullptr;

	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &wszPathToDocuments)))
	{
		CRT::StringCat(CRT::StringCopy(wszDestination, wszPathToDocuments), CS_XOR(L"\\SW\\"));
		bSuccess = true;

		if (!::CreateDirectoryW(wszDestination, nullptr) && ::GetLastError() != ERROR_ALREADY_EXISTS)
			bSuccess = false;
	}
	::CoTaskMemFree(wszPathToDocuments);

	if (bSuccess)
	{
		wchar_t wszSteamPath[MAX_PATH];
		if (SUCCEEDED(::SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, wszSteamPath)))
		{
			CRT::StringCat(wszSteamPath, CS_XOR(L"\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\game"));

			const wchar_t* folders[] = {
				L"\\materials",
				L"\\weapons",
				L"\\characters\\models",
				L"\\csgo\\materials",
				L"\\csgo\\weapons",
				L"\\csgo\\characters\\models"
			};

			for (const wchar_t* folder : folders)
			{
				wchar_t fullPath[MAX_PATH];
				CRT::StringCopy(fullPath, wszSteamPath);
				CRT::StringCat(fullPath, folder);

				if (!CreateDirectoryRecursive(fullPath) && ::GetLastError() != ERROR_ALREADY_EXISTS)
				{
					L_PRINT(LOG_ERROR) << L"Failed to create directory: " << fullPath;
				}
			}
		}
	}

	return bSuccess;
}

static bool Setup(HMODULE hModule)
{
#ifdef CS_LOG_CONSOLE
	if (!L::AttachConsole(CS_XOR(L"HackvsHack.net BETA | Debug x64")))
		return false;
#endif
#ifdef CS_LOG_FILE
	if (!L::OpenFile(CS_XOR(L"SW.log")))
		return false;
#endif

	if (!MEM::Setup() || !MATH::Setup() || !I::Setup() || !SDK::Setup() || !IPT::Setup())
		return false;

	D::Setup(IPT::hWindow, I::Device, I::DeviceContext);
	while (!D::bInitialized)
		::Sleep(200);

	if (!F::Setup())
		return false;

	std::vector<std::string> vecNeededModules = { CS_XOR("client.dll"), CS_XOR("engine2.dll"), CS_XOR("schemasystem.dll") };
	for (auto& szModule : vecNeededModules)
		if (!SCHEMA::Setup(CS_XOR(L"schema.txt"), szModule.c_str()))
			return false;

	if (!CONVAR::Setup() || !H::Setup() || !C::Setup(CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME)))
		return false;

	ModelChanger->UpdatePlayerModels();
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_GREEN | LOG_COLOR_FORE_INTENSITY) << CS_XOR("Player models updated");

	MENU::bMainWindowOpened = true;
	return true;
}

static void SafeDestroy()
{
	if (g_bShuttingDown)
		return;
	g_bShuttingDown = true;

	F::Destroy();
	H::Destroy();
	IPT::Destroy();
	D::Destroy();

#ifdef CS_LOG_CONSOLE
	L::DetachConsole();
#endif
#ifdef CS_LOG_FILE
	L::CloseFile();
#endif
}

DWORD WINAPI PanicThread(LPVOID lpParameter)
{
	while (!IPT::IsKeyReleased(C_GET(unsigned int, Vars.nPanicKey)) && !C_GET(bool, Vars.bUnloadCheat))
		::Sleep(50);

	SafeDestroy();
	::Sleep(500);
	::FreeLibraryAndExitThread(static_cast<HMODULE>(lpParameter), EXIT_SUCCESS);
	return 0;
}

extern "C" BOOL WINAPI _CRT_INIT(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);

BOOL APIENTRY CoreEntryPoint(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		if (!_CRT_INIT(hModule, dwReason, lpReserved))
			return FALSE;

		CORE::hProcess = MEM::GetModuleBaseHandle(nullptr);
		if (!CORE::hProcess || !MEM::GetModuleBaseHandle(NAVSYSTEM_DLL))
			return FALSE;

		CORE::hDll = hModule;
		if (!Setup(hModule))
		{
			SafeDestroy();
			return FALSE;
		}

		HANDLE hThread = ::CreateThread(nullptr, 0U, &PanicThread, hModule, 0UL, nullptr);
		if (hThread)
			::CloseHandle(hThread);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		return _CRT_INIT(hModule, dwReason, lpReserved);
	}
	return TRUE;
}
