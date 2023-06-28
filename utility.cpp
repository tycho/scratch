#include "utility.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace utility
{

bool IsWindowsVersionOrLater(VersionTriple greaterOrEqual)
{
#if defined(_WIN32)
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;

	dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi = {};
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = greaterOrEqual.majorVersion;
	osvi.dwMinorVersion = greaterOrEqual.minorVersion;
	osvi.dwBuildNumber = greaterOrEqual.patchVersion;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER,
							  dwlConditionMask) != FALSE;
#else
	return false;
#endif  // _WIN32
}

}