#pragma once

#include <tuple>

namespace utility {

// Helper for version number comparisons
struct VersionTriple
{
	constexpr VersionTriple() {}

	constexpr VersionTriple(int major, int minor, int patch)
		: majorVersion(major), minorVersion(minor), patchVersion(patch)
	{}

	int majorVersion = 0;
	int minorVersion = 0;
	int patchVersion = 0;
};

inline bool operator==(const VersionTriple &a, const VersionTriple &b)
{
	return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) ==
	       std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

inline bool operator!=(const VersionTriple &a, const VersionTriple &b)
{
	return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) !=
	       std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

inline bool operator<(const VersionTriple &a, const VersionTriple &b)
{
	return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) <
	       std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

inline bool operator>=(const VersionTriple &a, const VersionTriple &b)
{
	return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) >=
	       std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

bool IsWindowsVersionOrLater(VersionTriple greaterOrEqual);

} // namespace utility
