#pragma once

#include <chrono>
#include <thread>

#include "common.h"

namespace timers
{

//
// deadline_sleep: high resolution sleep API
//
// In the STL, std::this_thread::sleep_for is not always implemented using a
// high resolution clocksource, and may overshoot or undershoot by wide
// margins, depending on the underlying implementation. For situations where
// you need to sleep for very precise durations, use this class instead.
//
struct deadline_sleep
{
private:
	static void _sleep_impl(std::chrono::nanoseconds _Rel_time);

public:
	template <class _Rep, class _Period>
	static void sleep_for(const std::chrono::duration<_Rep, _Period> &_Rel_time)
	{
		_sleep_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(_Rel_time));
	}
};

//
// precise_sleep: fine-grained sleep API
//
// A class which will use the finest-grained sleep implementation available on
// the platform. This may overshoot or undershoot the sleep time, depending on
// the requested duration. However, this is finer-grained than current STL
// implementations of std::this_thread::sleep_for(). For example, on Windows
// the bare minimum sleep duration of std::this_thread::sleep_for() is 1ms,
// which may be too much depending on what you need to sleep for.
//
struct precise_sleep
{
private:
	static void _sleep_impl(std::chrono::nanoseconds _Rel_time);

public:
	template <class _Rep, class _Period>
	static void sleep_for(const std::chrono::duration<_Rep, _Period> &_Rel_time)
	{
		_sleep_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(_Rel_time));
	}
};

//
// relaxed_sleep: low resolution sleep API
//
// Currently simply wraps std::this_thread::sleep_for, which isn't super
// accurate, but is sufficient when you don't need the precision of
// timers::precise_sleep above.
//
struct relaxed_sleep
{
	template <class _Rep, class _Period>
	static void sleep_for(const std::chrono::duration<_Rep, _Period> &_Rel_time)
	{
		std::this_thread::sleep_for(_Rel_time);
	}
};


// Useful for measuring average time consumption by an arbitrary function, e.g.
// sleep or V-sync intervals or similar.
//
// Taken and modified from Chromium sources
//
template <class T, int64_t ringSize>
class TimeDeltaRingBuffer final
{
public:
	static_assert(ringSize > 1);

	static constexpr int64_t kRingSize = ringSize;

	TimeDeltaRingBuffer() = default;
	TimeDeltaRingBuffer(T initial)
	{
		Add(initial);
	}

	void Add(T value)
	{
		if likely(size_ == kRingSize) {
			rolling_sum_ -= values_[next_index_];
		} else {
			size_++;
		}
		values_[next_index_] = value;
		rolling_sum_ += value;
		next_index_ = (next_index_ + 1) % kRingSize;
	}

	void Clear()
	{
		rolling_sum_ = {};
		next_index_ = 0;
		size_ = 0;
	}

	void Fill(T value)
	{
		next_index_ = 0;
		size_ = kRingSize;
		rolling_sum_ = value * kRingSize;
		for (int64_t i = 0; i < kRingSize; i++)
			values_[i] = value;
	}

	no_discard T GetAverage() const
	{
		if unlikely(size_ == 0)
			return {};
		return rolling_sum_ / size_;
	}

private:
	T values_[kRingSize];
	int64_t next_index_ = 0;
	int64_t size_ = 0;
	// Rolling sum of values in the circular buffer above.
	T rolling_sum_{};
};

} // namespace timers