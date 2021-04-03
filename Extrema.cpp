#include <algorithm>

#include "Extrema.h"

CExtrema::CExtrema() :
	m_vMin(CVector2::m_kMax),
	m_vMax(-CVector2::m_kMax) {}

CExtrema::CExtrema(const CVector2& rvMin, const CVector2& rvMax) :
	m_vMin(rvMin),
	m_vMax(rvMax) {}

bool CExtrema::OverlapsWithExtrema(const CExtrema& rExtrema) const {
	return OverlapsWithExtrema(rExtrema.m_vMin, rExtrema.m_vMax);
}

bool CExtrema::ContainsPoint(const CVector2& rVector2) const {
	return OverlapsWithExtrema(rVector2, rVector2);
}

void CExtrema::ReEvaluate(const CExtrema& rExtrema) {
	ReEvaluate(rExtrema.m_vMin, rExtrema.m_vMax);
}

void CExtrema::ReEvaluate(const CVector2& rVector2) {
	ReEvaluate(rVector2, rVector2);
}

bool CExtrema::operator==(const CExtrema& rExtrema) const {
	return m_vMin == rExtrema.m_vMin && m_vMax == rExtrema.m_vMax;
}

bool CExtrema::OverlapsWithExtrema(const CVector2& rvMin, const CVector2& rvMax) const {
#if ROUND
	return
		m_vMax.x + g_kfEpsilon >= rvMin.x &&
		m_vMin.x - g_kfEpsilon <= rvMax.x &&
		m_vMax.y + g_kfEpsilon >= rvMin.y &&
		m_vMin.y - g_kfEpsilon <= rvMax.y;
#else // ROUND
	return
		m_vMax.x >= rvMin.x &&
		m_vMin.x <= rvMax.x &&
		m_vMax.y >= rvMin.y &&
		m_vMin.y <= rvMax.y;
#endif // ROUND
}

void CExtrema::ReEvaluate(const CVector2& rvMin, const CVector2& rvMax) {
	minAssign(m_vMin.x, rvMin.x);
	minAssign(m_vMin.y, rvMin.y);
	maxAssign(m_vMax.x, rvMax.x);
	maxAssign(m_vMax.y, rvMax.y);
}