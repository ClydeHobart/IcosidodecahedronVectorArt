#ifndef __SVG__
#define __SVG__

#include <iostream>
#include <fstream>
#include <string>

#include "Color.h"
#include "Defines.h"

class CVector2;

class CSvg {
public:
	CSvg(
		const	std::string&	rFileName,
		const	CVector2& 		rMinCoord,
		const	CVector2& 		rMaxCoord,
				b8				bShouldCondense	= false,
				CColor			fillColor		= CColor::sm_kBlack,
				CColor			strokeColor		= CColor::sm_kRed,
				f32				fStrokeWidth	= 0.25f
	);

	~CSvg();

	inline	const	c8*		GetOptionalNewLine	()								const	{ return m_bShouldCondense ? "" : "\n"; }
	inline	const	c8*		GetOptionalSpace	()								const	{ return m_bShouldCondense ? "" : " "; }
	inline	const	CColor&	GetFillColor		()								const	{ return m_FillColor; }
	inline			void	SetFillColor		(const CColor& rFillColor)				{ m_FillColor = rFillColor; }
	inline	const	CColor&	GetStrokeColor		()								const	{ return m_StrokeColor; }
	inline			void	SetStrokeColor		(const CColor& rStrokeColor)			{ m_StrokeColor = rStrokeColor; }
	inline			f32		GetStrokeWidth		()								const	{ return m_fStrokeWidth; }
	inline			void	SetStrokeWidth		(f32 fStrokeWidth)						{ m_fStrokeWidth = fStrokeWidth; }
	inline			b8		IsOpen				()								const	{ return m_file.is_open(); }
	inline			b8		ShouldCondense		()								const	{ return m_bShouldCondense; }

	CSvg& operator<<(const std::string& rString);

private:
	std::ofstream	m_file;
	b8				m_bShouldCondense;
	CColor			m_FillColor;
	CColor			m_StrokeColor;
	f32				m_fStrokeWidth;
};

#endif // __SVG__