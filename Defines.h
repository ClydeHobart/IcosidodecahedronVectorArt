#ifndef __DEFINES__
#define __DEFINES__

#include <algorithm>

#include "IntegralTypes.h"
#include "FloatingTypes.h"

#define ON	1
#define OFF	0

#define DBG_FV	OFF
#define DBG_FV3	OFF
#define DBG_FPH	OFF
#define DBG_V3	OFF
#define DBG_V2	OFF
#define DBG_S	OFF
#define DBG_PG	OFF
#define DBG_PH	ON
#define DBG_M	OFF

#define ROUND	ON

constexpr const	f32	g_kfEpsilon = 0.001f;

template<typename _Type>
constexpr inline void minAssign(_Type& rValA, const _Type& rValB) {
	rValA = std::min(rValA, rValB);
}

template<typename _Type>
constexpr inline void maxAssign(_Type& rValA, const _Type& rValB) {
	rValA = std::max(rValA, rValB);
}

template<typename _Type>
constexpr inline void clampAssign(_Type& rVal, const _Type& rLow, const _Type& rHigh) {
	rVal = std::clamp(rVal, rLow, rHigh);
}

template<u32 N>
inline u64 hashFNV1(const u8 (&rByteArray)[N]) {
	u64 uHash = 0xCBF29CE484222325ull;

	for (u32 uByteIndex = 0; uByteIndex < N; ++uByteIndex) {
		uHash = (uHash * 0x100000001B3ull) ^ rByteArray[uByteIndex];
	}

	return uHash;
}

template<typename _BigType, typename _SmallType>
constexpr inline _BigType pack(_SmallType valA, _SmallType valB) {
	static_assert(sizeof(_BigType) == sizeof(_SmallType) << 1);

	return (static_cast<_BigType>(valA) << (sizeof(_SmallType) << 3)) | valB;
}

#endif // __DEFINES__
