#include "PhiVector3.h"

CPhiVector3::CPhiVector3(const CPhiVector& rX, const CPhiVector& rY, const CPhiVector& rZ) :
	x(rX), y(rY), z(rZ) {}

CPhiVector CPhiVector3::GetMagnitudeSquared() const {
	return x * x + y * y + z * z;
}

const CPhiVector& CPhiVector3::operator[](u32 uIndex) const {
	switch (uIndex) {
	case 0:
		return x;
		break;
	case 1:
		return y;
		break;
	case 2:
		return z;
		break;
	default:
		return x;
		break;
	}
}

CPhiVector& CPhiVector3::operator[](u32 uIndex) {
	switch (uIndex) {
	case 0:
		return x;
		break;
	case 1:
		return y;
		break;
	case 2:
		return z;
		break;
	default:
		return x;
		break;
	}
}

CPhiVector3 CPhiVector3::operator*(const CPhiVector& rPhiVector) const {
	return CPhiVector3(*this) *= rPhiVector;
}

CPhiVector3 CPhiVector3::operator*(s32 sScalar) const {
	return CPhiVector3(*this) *= sScalar;
}

CPhiVector3 CPhiVector3::operator+(const CPhiVector3& rPhiVector3) const {
	return CPhiVector3(*this) += rPhiVector3;
}

CPhiVector3 CPhiVector3::operator-(const CPhiVector3& rPhiVector3) const {
	return CPhiVector3(*this) -= rPhiVector3;
}

CPhiVector3& CPhiVector3::operator*=(const CPhiVector& rPhiVector) {
	x *= rPhiVector;
	y *= rPhiVector;
	z *= rPhiVector;

	return *this;
}

CPhiVector3& CPhiVector3::operator*=(s32 sScalar) {
	x *= sScalar;
	y *= sScalar;
	z *= sScalar;

	return *this;
}

CPhiVector3& CPhiVector3::operator+=(const CPhiVector3& rPhiVector3) {
	x += rPhiVector3.x;
	y += rPhiVector3.y;
	z += rPhiVector3.z;

	return *this;
}

CPhiVector3& CPhiVector3::operator-=(const CPhiVector3& rPhiVector3) {
	x -= rPhiVector3.x;
	y -= rPhiVector3.y;
	z -= rPhiVector3.z;

	return *this;
}

CPhiVector3 operator*(s32 sScalar, const CPhiVector3& rPhiVector3) {
	return rPhiVector3 * sScalar;
}

std::ostream& operator<<(std::ostream& rOStream, const CPhiVector3& rPhiVector3) {
	return rOStream << '<' <<
		rPhiVector3.x << ", " <<
		rPhiVector3.y << ", " <<
		rPhiVector3.z << '>';
}