#ifndef __INTEGRAL_TYPES__
#define __INTEGRAL_TYPES__

#include <cstdint>

typedef bool		b8;
typedef char		c8;
typedef	int8_t		s8;
typedef	uint8_t		u8;
typedef	int16_t		s16;
typedef	uint16_t	u16;
typedef	int32_t		s32;
typedef	uint32_t	u32;
typedef	int64_t		s64;
typedef	uint64_t	u64;

#define	s8_MIN		static_cast<s8 >(0x0000000000000080)
#define	s8_MAX		static_cast<s8 >(0x000000000000007F)
#define	u8_MIN		static_cast<u8 >(0x0000000000000000)
#define	u8_MAX		static_cast<u8 >(0x00000000000000FF)

#define	s16_MIN		static_cast<s16>(0x0000000000008000)
#define	s16_MAX		static_cast<s16>(0x0000000000007FFF)
#define	u16_MIN		static_cast<u16>(0x0000000000000000)
#define	u16_MAX		static_cast<u16>(0x000000000000FFFF)

#define	s32_MIN		static_cast<s32>(0x0000000080000000)
#define	s32_MAX		static_cast<s32>(0x000000007FFFFFFF)
#define	u32_MIN		static_cast<u32>(0x0000000000000000)
#define	u32_MAX		static_cast<u32>(0x00000000FFFFFFFF)

#define	s64_MIN		static_cast<s64>(0x8000000000000000)
#define	s64_MAX		static_cast<s64>(0x7FFFFFFFFFFFFFFF)
#define	u64_MIN		static_cast<u64>(0x0000000000000000)
#define	u64_MAX		static_cast<u64>(0xFFFFFFFFFFFFFFFF)

#endif // __INTEGRAL_TYPES__