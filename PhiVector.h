#ifndef __PHI_VECTOR__
#define __PHI_VECTOR__

#include <compare>
#include <vector>
#include <iostream>
#include <tuple>
#include <numbers>
#include <string>
#include <unordered_map>

#include "Defines.h"

class CPhiVector {
// Functions
public:
	CPhiVector();
	CPhiVector(const std::vector<s32>& rVec);
	CPhiVector(s32 sScalar);

	operator f32() const;

	s32						operator[]	(u32 uIndex)					const;
	CPhiVector				operator-	()								const;
	CPhiVector				operator*	(const CPhiVector& rPhiVector)	const;
	CPhiVector				operator*	(s32 sScalar)					const;
	CPhiVector				operator+	(const CPhiVector& rPhiVector)	const;
	CPhiVector				operator-	(const CPhiVector& rPhiVector)	const;
	std::strong_ordering	operator<=>	(const CPhiVector& rPhiVector)	const;
	CPhiVector&				operator*=	(const CPhiVector& rPhiVector);
	CPhiVector&				operator*=	(s32 sScalar);
	CPhiVector&				operator+=	(const CPhiVector& rPhiVector);
	CPhiVector&				operator-=	(const CPhiVector& rPhiVector);

	friend	CPhiVector		operator*	(s32 sScalar, const CPhiVector& rPhiVector);
	friend	std::ostream&	operator<<	(std::ostream& rOStream, const CPhiVector& rPhiVector);

// Variables
private:
	std::vector<s32> m_Vec;
};

#endif // __PHI_VECTOR__