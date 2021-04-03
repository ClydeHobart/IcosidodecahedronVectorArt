#ifndef __COLOR__
#define __COLOR__

#include <iostream>

#include "Defines.h"

class CColor {
	// Enums
	public:
		enum EColor : u32 {
			C_Black			= 0x000000,	// (FF)
			C_White			= 0xFFFFFF,	// (FF)

			C_Red			= 0xFF0000,	// (FF)
			C_Green			= 0x00FF00,	// (FF)
			C_Blue			= 0x0000FF,	// (FF)

			C_Yellow		= 0xFFFF00,	// (FF)
			C_Cyan			= 0x00FFFF,	// (FF)
			C_Magenta		= 0xFF00FF,	// (FF)
			
			C_Gray_12_5		= 0x202020,	// (FF)
			C_Gray_25		= 0x404040,	// (FF)
			C_Gray_37_5		= 0x606060,	// (FF)
			C_Gray			= 0x808080,	// (FF)
			C_Gray_50		= C_Gray,
			C_Gray_62_5		= 0xA0A0A0,	// (FF)
			C_Gray_75		= 0xC0C0C0,	// (FF)
			C_Gray_87_5		= 0xE0E0E0,	// (FF)

			C_Red_Dark		= 0x800000,	// (FF)
			C_Red_Light		= 0xFF8080,	// (FF)
			C_Green_Dark	= 0x008000,	// (FF)
			C_Green_Light	= 0x80FF80,	// (FF)
			C_Blue_Dark		= 0x000080,	// (FF)
			C_Blue_Light	= 0x8080FF,	// (FF)

			C_Yellow_Dark	= 0x808000,	// (FF)
			C_Yellow_Light	= 0xFFFF80,	// (FF)
			C_Cyan_Dark		= 0x008080,	// (FF)
			C_Cyan_Light	= 0x80FFFF,	// (FF)
			C_Magenta_Dark	= 0x800080,	// (FF)
			C_Magenta_Light	= 0xFF80FF,	// (FF)
		};
	
	// Structs
	public:
		struct SColorPack {
			// Functions
			public:
				SColorPack(u8 uRed, u8 uGreen, u8 uBlue, u8 uAlpha = '\x00');
				SColorPack(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha = 1.0f);

			// Variables
			public:
				u8 m_uAlpha;
				u8 m_uBlue;
				u8 m_uGreen;
				u8 m_uRed;
		};

	// Functions
	public:
		CColor(u32 uColor = EColor::C_Black, b8 bHasAlpha = false);
		CColor(u8 uRed, u8 uGreen, u8 uBlue, u8 uAlpha = '\x00');
		CColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha = 1.0f);

		b8 operator==(const CColor& rColor) const;

		friend std::ostream& operator<<(std::ostream& rOStream, const CColor& rColor);

	// Variables
	public:
		union {
			u32 m_uColor;
			u8 m_aValues[4];
			SColorPack m_Pack;
		};

	// Constants
	public:
		static const CColor sm_kTransparent;

		static const CColor sm_kBlack;
		static const CColor sm_kWhite;

		static const CColor sm_kRed;
		static const CColor sm_kGreen;
		static const CColor sm_kBlue;

		static const CColor sm_kYellow;
		static const CColor sm_kCyan;
		static const CColor sm_kMagenta;
		
		static const CColor sm_kGray_12_5;
		static const CColor sm_kGray_25;
		static const CColor sm_kGray_37_5;
		static const CColor sm_kGray;
		static const CColor sm_kGray_50;
		static const CColor sm_kGray_62_5;
		static const CColor sm_kGray_75;
		static const CColor sm_kGray_87_5;

		static const CColor sm_kRed_Dark;
		static const CColor sm_kRed_Light;
		static const CColor sm_kGreen_Dark;
		static const CColor sm_kGreen_Light;
		static const CColor sm_kBlue_Dark;
		static const CColor sm_kBlue_Light;

		static const CColor sm_kYellow_Dark;
		static const CColor sm_kYellow_Light;
		static const CColor sm_kCyan_Dark;
		static const CColor sm_kCyan_Light;
		static const CColor sm_kMagenta_Dark;
		static const CColor sm_kMagenta_Light;
};

#endif // __COLOR__