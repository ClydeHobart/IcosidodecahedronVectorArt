#include "Svg.h"

#include "Vector2.h"

CSvg::CSvg(
		const	std::string&	rFileName,
		const	CVector2& 		rMinCoord,
		const	CVector2& 		rMaxCoord,
				b8				bShouldCondense	/*	= false */,
				CColor			fillColor		/*	= CColor::sm_kBlack */,
				CColor			strokeColor		/*	= CColor::sm_kRed */,
				f32				fStrokeWidth	/*	= 0.25f */) :
	m_file				(rFileName.c_str(),	std::ios_base::trunc),
	m_bShouldCondense	(bShouldCondense),
	m_FillColor			(fillColor),
	m_StrokeColor		(strokeColor),
	m_fStrokeWidth		(fStrokeWidth)
{
	if (IsOpen()) {
		m_file << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" <<
		rMinCoord.x << ' ' << rMinCoord.y << ' ' <<
		rMaxCoord.x - rMinCoord.x << ' ' << rMaxCoord.y - rMinCoord.y << "\">" <<
		GetOptionalNewLine();
	}
}

CSvg::~CSvg() {
	if (IsOpen()) {
		m_file << "</svg>" << GetOptionalNewLine();
	}
}

/*
inline const char* CSvg::GetOptionalSpace() const {
	return m_bShouldCondense ? "" : " ";
}

inline const char* CSvg::GetOptionalNewLine() const {
	return m_bShouldCondense ? "" : "\n";
}

inline bool CSvg::IsOpen() const {
	return m_file.is_open();
}

inline bool CSvg::ShouldCondense() const {
	return m_bShouldCondense;
}
//*/

CSvg& CSvg::operator<<(const std::string& rString) {
	if (IsOpen()) {
		m_file << rString;
	}

	return *this;
}