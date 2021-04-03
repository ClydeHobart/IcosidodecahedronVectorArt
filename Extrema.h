#ifndef __EXTREMA__
#define __EXTREMA__

#include "Defines.h"
#include "Vector2.h"

class CExtrema {
// Functions
public:
	CExtrema();
	CExtrema(const CVector2& rvMin, const CVector2& rvMax);

	bool	ContainsPoint		(const CVector2& rVector2)	const;
	bool	OverlapsWithExtrema	(const CExtrema& rExtrema)	const;
	void	ReEvaluate			(const CExtrema& rExtrema);
	void	ReEvaluate			(const CVector2& rVector2);

	bool operator==(const CExtrema& rExtrema) const;
private:
	bool	OverlapsWithExtrema	(const CVector2& rvMin, const CVector2& rvMax) const;
	void	ReEvaluate			(const CVector2& rvMin, const CVector2& rvMax);

// Variables
public:
	CVector2	m_vMin;
	CVector2	m_vMax;
};

#endif // __EXTREMA__