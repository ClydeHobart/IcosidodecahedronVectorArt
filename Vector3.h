#ifndef __VECTOR_3__
#define __VECTOR_3__

#include <iostream>

#include "Defines.h"

// Forward Declarations
class CPhiVector3;

class CVector3 {
// Functions
public:
	CVector3(const CPhiVector3& rPhiVector3);
	CVector3(f32 fX = 0.0f, f32 fY = 0.0f, f32 fZ = 0.0f);

	CVector3	Cross				(const CVector3& rVector3)	const;
	f32			Dot					(const CVector3& rVector3)	const;
	f32			GetMagnitudeSquared	()							const;
	CVector3	GetUnitVector		()							const;

	CVector3	operator*	(f32 fScalar)				const;
	CVector3	operator/	(f32 fScalar)				const;
	CVector3	operator+	(const CVector3& rVector3)	const;
	CVector3	operator-	(const CVector3& rVector3)	const;
	CVector3&	operator*=	(f32 fScalar);
	CVector3&	operator/=	(f32 fScalar);
	CVector3&	operator+=	(const CVector3& rVector3);
	CVector3&	operator-=	(const CVector3& rVector3);

	friend	CVector3		operator*	(f32 fScalar, const CVector3& rVector3);
	friend	std::ostream&	operator<<	(std::ostream& rOStream, const CVector3& rVector);

// Variables
public:
	f32	x;
	f32	y;
	f32	z;
};

#endif // __VECTOR_3__