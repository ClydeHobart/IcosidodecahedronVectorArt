#include <bit>
#include <cmath>

#include "Vector2.h"

#include "Extrema.h"
#include "Vector3.h"

CVector2::CVector2(f32 fX /* = 0.0f */, f32 fY /* = 0.0f */) : x(fX), y(fY) {}

CVector2::CVector2(const CVector3& rVector3) : x(rVector3.x), y(rVector3.y) {}

f32 CVector2::GetMagnitudeSquared() const {
	return x * x + y * y;
}

CVector2 CVector2::GetUnitVector() const {
	return CVector2(*this) /= sqrt(GetMagnitudeSquared());
}

CVector2 CVector2::operator-() const {
	return *this * -1.0f;
}

CVector2 CVector2::operator*(f32 fScalar) const {
	return CVector2(*this) *= fScalar;
}

CVector2 CVector2::operator/(f32 fScalar) const {
	return CVector2(*this) /= fScalar;
}

CVector2 CVector2::operator+(const CVector2& rVector2) const {
	return CVector2(*this) += rVector2;
}

CVector2 CVector2::operator-(const CVector2& rVector2) const {
	return CVector2(*this) -= rVector2;
}

bool CVector2::operator==(const CVector2& rVector2) const {
	#if V2_CACHING
		const u64 uThis = reinterpret_cast<const u64&>(*this);
		const u64 uThat = reinterpret_cast<const u64&>(rVector2);

		if (sm_ArePointsEquivalentCache.contains(uThis)) {
			std::unordered_map<u64, bool>& rSubMap = sm_ArePointsEquivalentCache[uThis];

			if (rSubMap.contains(uThat)) {
				return rSubMap[uThat];
			}
		}

		if (sm_ArePointsEquivalentCache.contains(uThat)) {
			std::unordered_map<u64, bool>& rSubMap = sm_ArePointsEquivalentCache[uThat];

			if (rSubMap.contains(uThis)) {
				return rSubMap[uThis];
			}
		}

		#if ROUND
			const bool bResult = (*this - rVector2).GetMagnitudeSquared() <= g_kfEpsilon;
		#else // ROUND
			const bool bResult = (*this - rVector2).GetMagnitudeSquared() == 0.0f;
		#endif // ROUND

		sm_ArePointsEquivalentCache[uThis][uThat] = bResult;
		sm_ArePointsEquivalentCache[uThat][uThis] = bResult;

		return bResult;
	#else // V2_CACHING
		#if ROUND
			return (*this - rVector2).GetMagnitudeSquared() <= g_kfEpsilon;
		#else // ROUND
			return (*this - rVector2).GetMagnitudeSquared() == 0.0f;
		#endif // ROUND
	#endif // V2_CACHING
}

CVector2& CVector2::operator*=(f32 fScalar) {
	x *= fScalar;
	y *= fScalar;

	return *this;
}

CVector2& CVector2::operator/=(f32 fScalar) {
	return *this *= 1.0 / fScalar;
}

CVector2& CVector2::operator+=(const CVector2& rVector2) {
	x += rVector2.x;
	y += rVector2.y;

	return *this;
}

CVector2& CVector2::operator-=(const CVector2& rVector2) {
	x -= rVector2.x;
	y -= rVector2.y;

	return *this;
}

f32 CVector2::ComputeInterpolant(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1) {
	const CVector2 vPDiff = rP2 - rP1;

	return vPDiff.x != 0.0f ?
		(rQ1.x - rP1.x) / vPDiff.x :
		vPDiff.y != 0.0f ?
			(rQ1.y - rP1.y) / vPDiff.y :
			0.0f;
}

f32 CVector2::ComputeInterpolant(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2) {
	// This uses the math from Paul Bourke (though not the code from Damian Coventry) from the City College of New York:
	// http://www-cs.ccny.cuny.edu/~wolberg/capstone/intersection/Intersection%20point%20of%20two%20lines.html
	f32 fQXDiff = rQ2.x - rQ1.x;
	f32 fQYDiff = rQ2.y - rQ1.y;
	f32 fNumerator = fQXDiff * (rP1.y - rQ1.y) - fQYDiff * (rP1.x - rQ1.x);
	f32 fDenominator = fQYDiff * (rP2.x - rP1.x) - fQXDiff * (rP2.y - rP1.y);

	return fDenominator == 0.0f ? 0.0f : fNumerator / fDenominator;
}

CVector2 CVector2::ComputeIntersection(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2, const f32* pInterpolant) {
	f32 fInterpolant = pInterpolant ? *pInterpolant : ComputeInterpolant(rP1, rP2, rQ1, rQ2);

	return rP1 * (1.0f - fInterpolant) + rP2 * fInterpolant;
}

u32 CVector2::ComputeOrientation(const CVector2& rP, const CVector2& rQ, const CVector2& rR) {
	// This is an implementation of the following algorithm from Geeks For Geeks:
	// https://www.geeksforgeeks.org/orientation-3-ordered-points/
	f32 fResult = (rQ.y - rP.y) * (rR.x - rQ.x) - (rR.y - rQ.y) * (rQ.x - rP.x);

	return
		#if ROUND
			std::abs(fResult) < g_kfEpsilon ?
		#else // ROUND
			fResult == 0.0f ?
		#endif // ROUND
			Colinear :
			fResult > 0.0f ?
				Clockwise :
				Counterclockwise;
}

u32 CVector2::DoLineSegmentsIntersect(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2, const bool bIsCheckingContainment) {
	#define DBG_V2_DLSI (DBG_V2 && ON)
	// This is an implementation of the following algorithm from Geeks For Geeks:
	// https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
	CExtrema extremaP, extremaQ;

	extremaP.ReEvaluate(rP1);
	extremaP.ReEvaluate(rP2);
	extremaQ.ReEvaluate(rQ1);
	extremaQ.ReEvaluate(rQ2);

	if (!extremaP.OverlapsWithExtrema(extremaQ)) {
		return DoNotIntersect;
	}

	if (!bIsCheckingContainment && rP1 == rQ1) {
		return PointsP1Q1Coincident;
	}

	// Since the return value of ComputeOrientation() is 0b001 (Colinear), 0b010, or 0b100,
	// uAndProduct will be 1 at the end iff all 4 points are colinear, otherwise it'll be 0
	u32 uOrientationP1P2Q1, uOrientationP1P2Q2, uOrientationQ1Q2P1, uOrientationQ1Q2P2;
	const u32 uAndProduct = Colinear &
		(uOrientationP1P2Q1 = ComputeOrientation(rP1, rP2, rQ1)) &
		(uOrientationP1P2Q2 = ComputeOrientation(rP1, rP2, rQ2)) &
		(uOrientationQ1Q2P1 = ComputeOrientation(rQ1, rQ2, rP1)) &
		(uOrientationQ1Q2P2 = ComputeOrientation(rQ1, rQ2, rP2));
	
	#if DBG_V2_DLSI
		const char* aOrientations[] = { "Colinear", "Clockwise", "Counterclockwise" };

		std::cout << "P1: " << rP1 << ", P2: " << rP2 << ", Q1: " << rQ1 << ", Q2: " << rQ2 <<
			"\n  P1-P2-Q1: " << aOrientations[std::countr_zero(uOrientationP1P2Q1)] <<
			"\n  P1-P2-Q2: " << aOrientations[std::countr_zero(uOrientationP1P2Q2)] <<
			"\n  Q1-Q2-P1: " << aOrientations[std::countr_zero(uOrientationQ1Q2P1)] <<
			"\n  Q1-Q2-P2: " << aOrientations[std::countr_zero(uOrientationQ1Q2P2)] << std::endl;
	#endif // DBG_V2_DLSI

	if (uAndProduct) {
		// If neither P1 nor Q1 lies on the other segment, then count this as DoNotIntersect (0b000).
		// P2 and Q2 will be counted in different calls of this function.
		// If checking for containment, then don't count colinear edges as intersection.

		#if DBG_V2_DLSI
		const u32 uResult = 
		#else // DBG_V2_DLSI
		return
		#endif // DBG_V2_DLSI
		
		bIsCheckingContainment ?
			DoNotIntersect :
			(extremaP.ContainsPoint(rQ1) ? (rP2 == rQ1 ? DoNotIntersect : PointQ1OnSegmentP) : 0) |
			(extremaQ.ContainsPoint(rP1) ? (rP1 == rQ2 ? DoNotIntersect : PointP1OnSegmentQ) : 0);

		#if DBG_V2_DLSI
			if (uResult & (PointQ1OnSegmentP | PointP1OnSegmentQ)) {
				std::cout << rP1 << ", " << rP2 << ", " << rQ1 << ", " << rQ2 << ", rP1 == rQ2: " << (rP1 == rQ2) <<
					", uResult & pointQ1OnSegmentP: " << static_cast<bool>(uResult & PointQ1OnSegmentP) <<
					", uResult & pointP1OnSegmentQ: " << static_cast<bool>(uResult & PointP1OnSegmentQ) << std::endl;
			}

			return uResult;
		#endif // DBG_V2_DLSI
	}

	const bool bSegmentPOrientationsDiffer = uOrientationP1P2Q1 ^ uOrientationP1P2Q2;
	const bool bSegmentQOrientationsDiffer = uOrientationQ1Q2P1 ^ uOrientationQ1Q2P2;

	if (bIsCheckingContainment) {
		return bSegmentPOrientationsDiffer && bSegmentQOrientationsDiffer &&
			(rQ1.y < rQ2.y ?
				uOrientationP1P2Q1 :
				uOrientationP1P2Q2
			) ^ Colinear; // If checking for containment, only count if the bottom point on segment Q is below segment P
	}

	if (bSegmentQOrientationsDiffer && uOrientationP1P2Q1 & Colinear) {
		return rP2 == rQ1 ? DoNotIntersect : PointQ1OnSegmentP;
	}

	if (bSegmentPOrientationsDiffer && uOrientationQ1Q2P1 & Colinear) {
		return rP1 == rQ2 ? DoNotIntersect : PointP1OnSegmentQ;
	}

	return bSegmentPOrientationsDiffer && bSegmentQOrientationsDiffer &&
		uOrientationP1P2Q2 ^ Colinear && uOrientationQ1Q2P2 ^ Colinear && // If the end point of one segment lies on the other segment, that intersection will be caught in a different call
		rP2 != rQ2; // Intersect = 0b001, DoNotIntersect = 0b000;
}

void CVector2::ResetCache() {
	#if V2_CACHING
		sm_ArePointsEquivalentCache.clear();
	#endif // V2_CACHING
}

CVector2 operator*(f32 fScalar, const CVector2& rVector2) {
	return rVector2 * fScalar;
}

std::ostream& operator<<(std::ostream& rOStream, const CVector2& rVector2) {
	return rOStream << '(' << rVector2.x << ", " << rVector2.y << ')';
}

#if V2_CACHING
	std::unordered_map<u64, std::unordered_map<u64, bool>> CVector2::sm_ArePointsEquivalentCache;
#endif // V2_CACHING

const CVector2 CVector2::m_kZero;
const CVector2 CVector2::m_kMax(f32_MAX, f32_MAX);