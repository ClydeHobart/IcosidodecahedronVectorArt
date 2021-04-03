#ifndef __VECTOR_2__
#define __VECTOR_2__

#include <iostream>
#include <unordered_map>

#include "Defines.h"

#define V2_CACHING OFF

class CVector3;

class CVector2 {
// Enums
public:
	enum : u32 {
		Colinear				= 0b0001,
		Clockwise				= 0b0010,
		Counterclockwise		= 0b0100,
		DoNotIntersect			= 0b0000,
		Intersect				= 0b0001,
		PointQ1OnSegmentP		= 0b0010,
		PointP1OnSegmentQ		= 0b0100,
		PointsP1Q1Coincident	= 0b1000
	};

// Functions
public:
	CVector2(f32 fX = 0.0f, f32 fY = 0.0f);
	CVector2(const CVector3& rVector3);

	f32			GetMagnitudeSquared	()	const;
	CVector2	GetUnitVector		()	const;

	CVector2	operator-	()							const;
	CVector2	operator*	(f32 fScalar)				const;
	CVector2	operator/	(f32 fScalar)				const;
	CVector2	operator+	(const CVector2& rVector2)	const;
	CVector2	operator-	(const CVector2& rVector2)	const;
	bool		operator==	(const CVector2& rVector2)	const;
	CVector2&	operator*=	(f32 fScalar);
	CVector2&	operator/=	(f32 fScalar);
	CVector2&	operator+=	(const CVector2& rVector2);
	CVector2&	operator-=	(const CVector2& rVector2);

	static	f32			ComputeInterpolant		(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1);
	static	f32			ComputeInterpolant		(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2);
	static	CVector2	ComputeIntersection		(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2, const f32* pInterpolant = nullptr);
	static	u32			ComputeOrientation		(const CVector2& rP, const CVector2& rQ, const CVector2& rR);
	static	u32			DoLineSegmentsIntersect	(const CVector2& rP1, const CVector2& rP2, const CVector2& rQ1, const CVector2& rQ2, const bool bIsCheckingContainment = false);
	static	bool		IsCachingEnabled		() { return V2_CACHING; }
	static	void		ResetCache				();

	friend	CVector2		operator*	(f32 fScalar, const CVector2& rVector2);
	friend	std::ostream&	operator<<	(std::ostream& rOStream, const CVector2& rVector2);
// Variables
public:
	f32	x;
	f32	y;

	#if V2_CACHING
		static std::unordered_map<u64, std::unordered_map<u64, bool>> sm_ArePointsEquivalentCache;
	#endif // V2_CACHING

// Constants
public:
	static	const	CVector2	m_kZero;
	static	const	CVector2	m_kMax;
};

#endif // __VECTOR_2__
