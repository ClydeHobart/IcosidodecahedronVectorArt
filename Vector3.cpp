#include <cmath>

#include "PhiVector3.h"
#include "Vector3.h"

CVector3::CVector3(const CPhiVector3& rPhiVector3) :
	x(rPhiVector3.x),
	y(rPhiVector3.y),
	z(rPhiVector3.z) {}

CVector3::CVector3(f32 fX, f32 fY, f32 fZ) : x(fX), y(fY), z(fZ) {}

CVector3 CVector3::Cross(const CVector3& rVector3) const {
	return CVector3(
		y * rVector3.z - rVector3.y * z,
		rVector3.x * z - x * rVector3.z,
		x * rVector3.y - rVector3.x * y);
}

f32 CVector3::Dot(const CVector3& rVector3) const {
	return x * rVector3.x + y * rVector3.y + z * rVector3.z;
}

f32 CVector3::GetMagnitudeSquared() const {
	return x * x + y * y + z * z;
}

CVector3 CVector3::GetUnitVector() const {
	return CVector3(*this) /= sqrt(GetMagnitudeSquared());
}

CVector3 CVector3::operator*(f32 fScalar) const {
	return CVector3(*this) *= fScalar;
}

CVector3 CVector3::operator/(f32 fScalar) const {
	return CVector3(*this) /= fScalar;
}

CVector3 CVector3::operator+(const CVector3& rVector3) const {
	return CVector3(*this) += rVector3;
}

CVector3 CVector3::operator-(const CVector3& rVector3) const {
	return CVector3(*this) -= rVector3;
}

CVector3& CVector3::operator*=(f32 fScalar) {
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;

	return *this;
}

CVector3& CVector3::operator/=(f32 fScalar) {
	return *this *= 1.0 / fScalar;
}

CVector3& CVector3::operator+=(const CVector3& rVector3) {
	x += rVector3.x;
	y += rVector3.y;
	z += rVector3.z;

	return *this;
}

CVector3& CVector3::operator-=(const CVector3& rVector3) {
	x -= rVector3.x;
	y -= rVector3.y;
	z -= rVector3.z;

	return *this;
}

CVector3 operator*(f32 fScalar, const CVector3& rVector3) {
	return rVector3 * fScalar;
}

std::ostream& operator<<(std::ostream& rOStream, const CVector3& rVector3) {
	return rOStream << '(' << rVector3.x << ", " << rVector3.y << ", " << rVector3.z << ')';
}