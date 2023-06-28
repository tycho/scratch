#pragma once

#if __cplusplus >= 201803L
#define likely(x) (x) [[likely]]
#define unlikely(x) (x) [[unlikely]]
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#if __cplusplus >= 201603L
#define no_discard [[nodiscard]]
#else
#define no_discard
#endif

