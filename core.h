#pragma once

namespace CORE
{
	/* @section: get */
	/// @param[out] wszDestination output for working path where files will be saved (default: "%userprofile%\documents\.crown")
	/// @returns: true if successfully got the path, false otherwise
	bool GetWorkingPath(wchar_t* wszDestination);
	bool CreateDirectoryRecursive(const wchar_t* path);

	/* @section: values */
	// handle of self module
	inline void* hDll = nullptr;
	// current process handle
	inline void* hProcess = nullptr;
}
