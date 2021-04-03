#include "Color.h"

#include <iomanip>

CColor::SColorPack::SColorPack(u8 uRed, u8 uGreen, u8 uBlue, u8 uAlpha /* = '\x00' */) :
	m_uAlpha	(uAlpha),
	m_uBlue		(uBlue), 
	m_uGreen	(uGreen),
	m_uRed		(uRed) 		{}

CColor::SColorPack::SColorPack(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha /* = 1.0f */) :
	m_uAlpha	(255.0f * std::clamp(fAlpha,	0.0f, 1.0f)),
	m_uBlue		(255.0f * std::clamp(fBlue,		0.0f, 1.0f)),
	m_uGreen	(255.0f * std::clamp(fGreen,	0.0f, 1.0f)),
	m_uRed		(255.0f * std::clamp(fRed,		0.0f, 1.0f))	{}

CColor::CColor(u32 uColor /* = EColor::C_Black */, b8 bHasAlpha /* = false */) :
	m_uColor(bHasAlpha ? uColor : uColor << 8 | 0xFF) {}

CColor::CColor(u8 uRed, u8 uGreen, u8 uBlue, u8 uAlpha /* = '\x00' */) :
	m_Pack(uRed, uGreen, uBlue, uAlpha) {}

CColor::CColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha /* = 1.0f */) :
	m_Pack(fRed, fGreen, fBlue, fAlpha) {}

b8 CColor::operator==(const CColor& rColor) const {
	return m_uColor == rColor.m_uColor;
}

std::ostream& operator<<(std::ostream& rOStream, const CColor& rColor) {
	std::ios_base::fmtflags flags = rOStream.setf(std::ios_base::hex, std::ios_base::basefield);
	c8 cFill = rOStream.fill('0');
	const b8 bIncludeAlpha = rColor.m_Pack.m_uAlpha != 0xFF;

	return (
		rOStream <<
		'#' <<
		std::setw(
			bIncludeAlpha ?
				8 :
				6) <<
		(
			bIncludeAlpha ?
				rColor.m_uColor :
				rColor.m_uColor >> 8) <<
		std::setfill(cFill) <<
		std::setiosflags(flags));
}

const CColor CColor::sm_kTransparent	(EColor::C_Black, true);

const CColor CColor::sm_kBlack			(EColor::C_Black);
const CColor CColor::sm_kWhite			(EColor::C_White);

const CColor CColor::sm_kRed			(EColor::C_Red);
const CColor CColor::sm_kGreen			(EColor::C_Green);
const CColor CColor::sm_kBlue			(EColor::C_Blue);

const CColor CColor::sm_kYellow			(EColor::C_Yellow);
const CColor CColor::sm_kCyan			(EColor::C_Cyan);
const CColor CColor::sm_kMagenta		(EColor::C_Magenta);
			
const CColor CColor::sm_kGray_12_5		(EColor::C_Gray_12_5);
const CColor CColor::sm_kGray_25		(EColor::C_Gray_25);
const CColor CColor::sm_kGray_37_5		(EColor::C_Gray_37_5);
const CColor CColor::sm_kGray			(EColor::C_Gray);
const CColor CColor::sm_kGray_50		(EColor::C_Gray_50);
const CColor CColor::sm_kGray_62_5		(EColor::C_Gray_62_5);
const CColor CColor::sm_kGray_75		(EColor::C_Gray_75);
const CColor CColor::sm_kGray_87_5		(EColor::C_Gray_87_5);

const CColor CColor::sm_kRed_Dark		(EColor::C_Red_Dark);
const CColor CColor::sm_kRed_Light		(EColor::C_Red_Light);
const CColor CColor::sm_kGreen_Dark		(EColor::C_Green_Dark);
const CColor CColor::sm_kGreen_Light	(EColor::C_Green_Light);
const CColor CColor::sm_kBlue_Dark		(EColor::C_Blue_Dark);
const CColor CColor::sm_kBlue_Light		(EColor::C_Blue_Light);

const CColor CColor::sm_kYellow_Dark	(EColor::C_Yellow_Dark);
const CColor CColor::sm_kYellow_Light	(EColor::C_Yellow_Light);
const CColor CColor::sm_kCyan_Dark		(EColor::C_Cyan_Dark);
const CColor CColor::sm_kCyan_Light		(EColor::C_Cyan_Light);
const CColor CColor::sm_kMagenta_Dark	(EColor::C_Magenta_Dark);
const CColor CColor::sm_kMagenta_Light	(EColor::C_Magenta_Light);