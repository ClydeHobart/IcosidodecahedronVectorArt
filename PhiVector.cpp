#include <bit>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <float.h>
#include <fstream>
#include <numbers>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>

#include "PhiVector.h"

CPhiVector::CPhiVector() : m_Vec({0}) {}

CPhiVector::CPhiVector(const std::vector<s32>& rVec) : m_Vec(rVec) {}

CPhiVector::CPhiVector(s32 sScalar) : m_Vec({sScalar}) {}

CPhiVector::operator f32() const {
	f32 fSum = 0.0;
	f32 fPower = 1.0;

	for (u32 i = 0; i < m_Vec.size(); ++i) {
		fSum += m_Vec[i] * fPower;
		fPower *= std::numbers::phi_v<f32>;
	}

	return fSum;
}

s32 CPhiVector::operator[](u32 uIndex) const {
	return uIndex < m_Vec.size() ? m_Vec[uIndex] : 0;
}

CPhiVector CPhiVector::operator-() const {
	return CPhiVector(*this) *= -1;
}

CPhiVector CPhiVector::operator*(const CPhiVector& rPhiVector) const {
	return CPhiVector(*this) *= rPhiVector;
}

CPhiVector CPhiVector::operator*(s32 sScalar) const {
	return CPhiVector(m_Vec) *= sScalar;
}

CPhiVector CPhiVector::operator+(const CPhiVector& rPhiVector) const {
	return CPhiVector(*this) += rPhiVector;
}

CPhiVector CPhiVector::operator-(const CPhiVector& rPhiVector) const {
	return CPhiVector(*this) -= rPhiVector;
}

std::strong_ordering CPhiVector::operator<=>(const CPhiVector& rPhiVector) const {
	CPhiVector diffPhiVector = *this - rPhiVector;
	f32 fDiff = diffPhiVector;

	if (abs(fDiff) > 1.0) {
		return fDiff < 0 ? std::strong_ordering::greater : std::strong_ordering::less;
	}

	CPhiVector additiveIdentityPhiVector(std::vector<s32>{-1, -1, 1});
	CPhiVector scalingPhiVector(std::vector<s32>{0, 1});

	for (u32 i = 0; i < diffPhiVector.m_Vec.size() - 2; ++i) {
		if (diffPhiVector.m_Vec[i]) {
			diffPhiVector += diffPhiVector.m_Vec[i] * additiveIdentityPhiVector;
		}

		additiveIdentityPhiVector *= scalingPhiVector;
	}

	fDiff = additiveIdentityPhiVector;

	return fDiff == 0.0 ?
		std::strong_ordering::equivalent :
		fDiff < 0 ?
			std::strong_ordering::greater :
			std::strong_ordering::less;
}

CPhiVector& CPhiVector::operator*=(const CPhiVector& rPhiVector) {
	const std::vector<s32>& rVec = rPhiVector.m_Vec;

	std::vector<s32> resultVector(m_Vec.size() + rVec.size() - 1, 0);

	for (u32 i = 0; i < m_Vec.size(); ++i) {
		if (m_Vec[i]) {
			for (u32 j = 0; j < rVec.size(); ++j) {
				resultVector[i + j] += m_Vec[i] * rVec[j];
			}
		}
	}

	m_Vec.assign(resultVector.begin(), resultVector.end());

	return *this;
}

CPhiVector& CPhiVector::operator*=(s32 sScalar) {
	if (sScalar) {
		for (u32 i = 0; i < m_Vec.size(); ++i) {
			m_Vec[i] *= sScalar;
		}
	} else {
		m_Vec.assign(1, 0);
	}

	return *this;
}

CPhiVector& CPhiVector::operator+=(const CPhiVector& rPhiVector) {
	const std::vector<s32>& rVec = rPhiVector.m_Vec;

	m_Vec.resize(std::max(m_Vec.size(), rVec.size()), 0);

	for (u32 i = 0; i < rVec.size(); ++i) {
		m_Vec[i] += rVec[i];
	}

	return *this;
}

CPhiVector& CPhiVector::operator-=(const CPhiVector& rPhiVector) {
	const std::vector<s32>& rVec = rPhiVector.m_Vec;

	m_Vec.resize(std::max(m_Vec.size(), rVec.size()), 0);

	for (u32 i = 0; i < rVec.size(); ++i) {
		m_Vec[i] -= rVec[i];
	}

	return *this;
}

CPhiVector operator*(s32 sScalar, const CPhiVector& rPhiVector) {
	return rPhiVector * sScalar;
}

std::ostream& operator<<(std::ostream& rOStream, const CPhiVector& rPhiVector) {
	rOStream << '[' << rPhiVector.m_Vec[0];

	if (rPhiVector.m_Vec.size() > 1) {
		for (u32 i = 1; i < rPhiVector.m_Vec.size(); ++i) {
			rOStream << ", " << rPhiVector.m_Vec[i];
		}
	}

	return rOStream << ']';
}