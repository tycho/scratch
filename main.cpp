#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <cassert>
#include <chrono>
#include <thread>
#include <cstdio>
#include <windows.h>

#include "timers.h"

void stl_sleep_for(std::chrono::nanoseconds _time)
{
	std::this_thread::sleep_for(_time);
}

void relaxed_sleep_for(std::chrono::nanoseconds _time)
{
	timers::relaxed_sleep::sleep_for(_time);
}

void precise_sleep_for(std::chrono::nanoseconds _time)
{
	timers::precise_sleep::sleep_for(_time);
}

void deadline_sleep_for(std::chrono::nanoseconds _time)
{
	timers::deadline_sleep::sleep_for(_time);
}

double calculatePercentError(double targetValue, double actualValue) {
    double difference = actualValue - targetValue;
    double percentError = (difference / targetValue) * 100.0;

    return std::fabs(percentError);
}

typedef void (*sleep_func_t)(std::chrono::nanoseconds _time);
void test_sleep(sleep_func_t _func, std::chrono::nanoseconds _time)
{
	SwitchToThread();

	auto start = std::chrono::high_resolution_clock::now();
	_func(_time);
	auto end = std::chrono::high_resolution_clock::now();

	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	auto error = std::chrono::duration_cast<std::chrono::microseconds>(end - start - _time);

	printf("    elapsed: %lld us, error: %lld us (%.2f%%)\n",
		   elapsed.count(),
		   error.count(),
		   calculatePercentError(
			   static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(_time).count()),
			   static_cast<double>(elapsed.count())
		   )
	);
}

void set_timers_high_res()
{
#if defined(_WIN32)
	typedef NTSTATUS(NTAPI *NtSetTimerResolution)(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
	typedef NTSTATUS(NTAPI *NtQueryTimerResolution)(PULONG MinimumResolution, PULONG MaximumResolution, PULONG CurrentResolution);
	NtSetTimerResolution pNtSetTimerResolution;
	NtQueryTimerResolution pNtQueryTimerResolution;
	HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
	if (!hNtDll)
		return;
	pNtSetTimerResolution = (NtSetTimerResolution)GetProcAddress(hNtDll, "NtSetTimerResolution");
	pNtQueryTimerResolution = (NtQueryTimerResolution)GetProcAddress(hNtDll, "NtQueryTimerResolution");
	if (pNtSetTimerResolution && pNtQueryTimerResolution) {
		NTSTATUS result;
		ULONG ulMinimum = 0, ulMaximum = 0, ulCurrent = 0;

		result = pNtQueryTimerResolution(&ulMinimum, &ulMaximum, &ulCurrent);
		if (result != ERROR_SUCCESS)
			return;
		printf("Windows timer resolution minimum %lu us, maximum %lu us, current %lu us\n",
			   ulMinimum / 10, ulMaximum / 10, ulCurrent / 10);

		result = pNtSetTimerResolution(ulMaximum, TRUE, &ulCurrent);
		if (result != ERROR_SUCCESS)
			return;

		result = pNtQueryTimerResolution(&ulMinimum, &ulMaximum, &ulCurrent);
		if (result != ERROR_SUCCESS)
			return;
		printf("Windows timer resolution set to %lu us\n", ulCurrent / 10);
	}
#endif // _WIN32
}

int main(int argc, char **argv)
{
	set_timers_high_res();

	std::chrono::microseconds wait_times[] = {
		std::chrono::microseconds(50'000),
		std::chrono::microseconds(20'000),
		std::chrono::microseconds(10'000),
		std::chrono::microseconds(5'000),
		std::chrono::microseconds(1'000),
		std::chrono::microseconds(500),
		std::chrono::microseconds(100),
		std::chrono::microseconds(1),
		std::chrono::microseconds(100),
		std::chrono::microseconds(500),
		std::chrono::microseconds(1'000),
		std::chrono::microseconds(5'000),
		std::chrono::microseconds(10'000),
		std::chrono::microseconds(20'000),
		std::chrono::microseconds(50'000),
	};

	struct sleep_impl {
		const char *name;
		sleep_func_t func;
	};

	sleep_impl sleep_impls[] = {
		{ "std::this_thread::sleep_for", stl_sleep_for },
		//{ "timers::relaxed_sleep::sleep_for", relaxed_sleep_for },
		{ "timers::precise_sleep::sleep_for", precise_sleep_for },
		{ "timers::deadline_sleep::sleep_for", deadline_sleep_for },
	};

	for (auto wait_time : wait_times) {
		printf("\nwait time: %lld us\n", wait_time.count());
		for (auto &sleep_func : sleep_impls) {
			printf("  %s\n", sleep_func.name);
			for (int trial = 0; trial < 5; trial++) {
				test_sleep(sleep_func.func, wait_time);
			}
		}
	}

#ifdef  _WIN32
	// Ugh. Just keep the terminal window open after a run.
	system("pause");
#endif

	return 0;
}


