#ifndef __PHI_VECTOR_3__
#define __PHI_VECTOR_3__

#include <iostream>

#include "Defines.h"
#include "PhiVector.h"

// Forward Declarations
class CPhiVector;

class CPhiVector3 {
// Functions
public:
	CPhiVector3(const CPhiVector& rX, const CPhiVector& rY, const CPhiVector& rZ);

	CPhiVector GetMagnitudeSquared() const;

	const	CPhiVector&		operator[]	(u32 uIndex)						const;
			CPhiVector&		operator[]	(u32 uIndex);
			CPhiVector3		operator*	(const CPhiVector& rPhiVector)		const;
			CPhiVector3		operator*	(s32 sScalar)						const;
			CPhiVector3		operator+	(const CPhiVector3& rPhiVector3)	const;
			CPhiVector3		operator-	(const CPhiVector3& rPhiVector3)	const;
			CPhiVector3&	operator*=	(const CPhiVector& rPhiVector);
			CPhiVector3&	operator*=	(s32 sScalar);
			CPhiVector3&	operator+=	(const CPhiVector3& rPhiVector3);
			CPhiVector3&	operator-=	(const CPhiVector3& rPhiVector3);

	friend	CPhiVector3		operator*	(s32 sScalar, const CPhiVector3& rPhiVector3);
	friend	std::ostream&	operator<<	(std::ostream& rOStream, const CPhiVector3& rPhiVector3);

// Variables
public:
	CPhiVector	x;
	CPhiVector	y;
	CPhiVector	z;
};

#endif // __PHI_VECTOR_3__