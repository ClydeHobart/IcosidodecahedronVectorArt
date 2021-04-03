#ifndef __PHI_POLYHEDRON__
#define __PHI_POLYHEDRON__

#include <iostream>
#include <tuple>
#include <vector>

#include "Defines.h"

// Forward Declarations
class CPhiVector;
class CPhiVector3;

class CPhiPolyhedron {
// Functions
public:
	CPhiPolyhedron();

	CPhiPolyhedron&	CopyForEachVertex					(const std::vector<CPhiVector3>& rVertices);
	CPhiPolyhedron&	GenerateIcosahedronFractal			(u32 uIteration);
	CPhiPolyhedron&	GenerateIcosidodecahedronFractal	(u32 uIteration);

	static	const	CPhiPolyhedron&	GetIcosahedron();
	static	const	CPhiPolyhedron&	GetIcosidodecahedron();

	CPhiPolyhedron	operator*	(const CPhiVector& rPhiVector)	const;
	CPhiPolyhedron	operator*	(s32 sScalar)					const;
	CPhiPolyhedron&	operator*=	(const CPhiVector& rPhiVecto3);
	CPhiPolyhedron&	operator*=	(s32 sScalar);

	friend std::ostream& operator<<(std::ostream& rOStream, const CPhiPolyhedron& rPhiPolyhedron);
private:
	static	void	GenerateIcosahedron();
	static	void	GenerateIcosidodecahedron();

// Variables
public:
	std::vector<CPhiVector3>			m_Vertices;
	std::vector<std::pair<u32, u32>>	m_Edges;
	std::vector<std::vector<u32>>		m_Faces;
private:
	static	CPhiPolyhedron	m_sIcosahedron;
	static	CPhiPolyhedron	m_sIcosidodecahedron;
};

#endif // __PHI_POLYHEDRON__