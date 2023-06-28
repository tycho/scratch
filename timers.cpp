#include "common.h"
#include "timers.h"
#include "utility.h"

#include <cassert>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <time.h>
#endif

#ifdef _MSC_VER
#    include <intrin.h>  // for _mm_pause() and __yield()
#else
#    include <x86intrin.h>
#endif

#if defined(__ARM_ARCH_7__) || defined(__aarch64__)
#    if defined(__GNUC__) || defined(__clang__)
#        include <arm_acle.h>  // for __yield()
#    endif
#endif

#if 0
#define SIMULATE_NO_HIGH_RES_WAITABLE_TIMER
#endif

namespace {

constexpr utility::VersionTriple kVersionWindows10_RS4 = utility::VersionTriple(10, 0, 17134);

} // namespace

namespace timers
{

namespace
{

void cpu_yield()
{
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
	_mm_pause();
#elif defined(__ARM_ARCH_7A__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
	__yield();
#endif
}

typedef void (*sleep_func_t)(std::chrono::nanoseconds _sleepTime);

void adaptive_sleep(std::chrono::nanoseconds _Rel_time, sleep_func_t _highResSleep)
{
	using hrclock = std::chrono::high_resolution_clock;

	// Interval to use for sleep
	constexpr std::chrono::microseconds kMinSleep(2000);
	constexpr std::chrono::microseconds kBadSleepAverage(kMinSleep * 3);

	// Keep a running average for the observed duration of a 1ms sleep. Start
	// with an estimation that a 1ms sleep actually takes about 2ms.
	static TimeDeltaRingBuffer<hrclock::duration, 32> min_sleep_time(std::chrono::duration_cast<hrclock::duration>(kMinSleep * 2));

	// Keep doing a sleep while the requested sleep time is greater than the
	// average time to sleep 1ms
	while (_Rel_time > min_sleep_time.GetAverage()) {
		auto start = hrclock::now();
		_highResSleep(kMinSleep);
		auto end = hrclock::now();

		auto observed = end - start;
		min_sleep_time.Add(observed);

		_Rel_time -= observed;
	}

	if unlikely(min_sleep_time.GetAverage() >= kBadSleepAverage) {
		// If the average goes way out of the expected range, we need to reset the
		// ring buffer, or else this adaptive sleep will degenerate into only a
		// spin-wait loop.
		min_sleep_time.Clear();
		min_sleep_time.Add(kMinSleep * 2);
	}

	// Spin-wait for the remaining period
	auto start = hrclock::now();
	while (hrclock::now() - start < _Rel_time) {
		cpu_yield();
	}
}

#ifdef _WIN32
#if defined(SIMULATE_NO_HIGH_RES_WAITABLE_TIMER)
constexpr bool waitable_timer_supported = false;
#else
const bool waitable_timer_supported = utility::IsWindowsVersionOrLater(kVersionWindows10_RS4);
#endif

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif
static HANDLE create_waitable_timer()
{
	HANDLE hTimer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	assert(hTimer != nullptr);
	return hTimer;
}

static void sleep_waitable_timer(std::chrono::nanoseconds _Rel_time)
{
	// TODO: This handle leaks on thread destruction, need a wrapper class to
	// CloseHandle it on thread exit.
	static thread_local HANDLE hTimer(create_waitable_timer());

	// SetWaitableTimerEx uses 100ns NT timer ticks as its unit of measurement.
	// A negative value means to wait for a relative period of time instead of
	// absolute (e.g. a FILETIME deadline)
	LARGE_INTEGER relTime = {.QuadPart = -_Rel_time.count() / 100};
	BOOL timerSet = SetWaitableTimerEx(hTimer, &relTime, 0, nullptr, nullptr, nullptr, 0);
	assert(timerSet != FALSE);
	WaitForSingleObjectEx(hTimer, INFINITE, FALSE);
}
#endif

#if defined(__linux__) // Linux
static void sleep_nanosleep(std::chrono::nanoseconds _Rel_time)
{
	struct timespec ts, remain;
	ts.tv_sec = _Rel_time.count() / std::chrono::nanoseconds::period::den;
	ts.tv_nsec = _Rel_time.count() % std::chrono::nanoseconds::period::den;
	int rv;
	do {
		rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &remain);
		ts = remain;
	} while (rv != 0);
}
#endif


#if defined(__MACH__) // macOS
static void sleep_nanosleep(std::chrono::nanoseconds _Rel_time)
{
	struct timespec ts, remain;
	ts.tv_sec = _Rel_time.count() / std::chrono::nanoseconds::period::den;
	ts.tv_nsec = _Rel_time.count() % std::chrono::nanoseconds::period::den;
	int rv;
	do {
		rv = nanosleep(&ts, &remain);
		ts = remain;
	} while (rv != 0);
}
#endif

static void sleep_stl(std::chrono::nanoseconds _Rel_time)
{
	std::this_thread::sleep_for(_Rel_time);
}

} // namespace


#ifdef _WIN32

void deadline_sleep::_sleep_impl(std::chrono::nanoseconds _Rel_time)
{
	if (waitable_timer_supported) {
		adaptive_sleep(_Rel_time, sleep_waitable_timer);
	} else {
		adaptive_sleep(_Rel_time, sleep_stl);
	}
}

void precise_sleep::_sleep_impl(std::chrono::nanoseconds _Rel_time)
{
	if (waitable_timer_supported) {
		sleep_waitable_timer(_Rel_time);
	} else {
		sleep_stl(_Rel_time);
	}
}

#else // _WIN32

void deadline_sleep::_sleep_impl(std::chrono::nanoseconds _Rel_time)
{
	adaptive_sleep(_Rel_time, sleep_nanosleep);
}

void precise_sleep::_sleep_impl(std::chrono::nanoseconds _Rel_time)
{
	sleep_nanosleep(_Rel_time);
}

#endif // _WIN32

} // namespace timers
