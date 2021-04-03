#ifndef __POLYHEDRON__
#define __POLYHEDRON__

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Defines.h"

#include "Polygon.h"
#include "Vector3.h"

// Forward Declarations
class CPhiPolyhedron;

class CPolyhedron {
// Enums
public:
	enum {
		Verts				= 1 << 0,
		Edges				= 1 << 1,
		Faces				= 1 << 2,
		Color				= 1 << 3,
		Depth				= 1 << 4,
		Icosahedron			= 1 << 5,
		Icosidodecahedron	= 1 << 6,
		CullHiddenFaces		= 1 << 7
	};

// Functions
public:
	CPolyhedron();
	CPolyhedron(const CPhiPolyhedron& rPhiPolyhedron);

	CPolyhedron&	Focus3FoldSymmetry	();
	CPolyhedron&	Focus5FoldSymmetry	();
	bool			SaveToSvg			(std::string fileName, u32 uPrintFlags = Verts | Faces)	const;
private:
	bool		CompareFaceIndices		(u32 uIndexA, u32 uIndexB)														const;
	u32			ComputeRGB				(u32 uPrintFlags, u32 uIndexType, u32 uIndex)									const;
	u32			ComputeRGBA				(u32 uPrintFlags, u32 uFaceIndex)												const;
	CVector3	GetFaceNormal			(u32 uFace, bool bShouldNormalize = false)										const;
	CPolygon	GeneratePolygonForFace	(u32 uFaceIndex)																const;
	void		PopulateInverseEdges	(std::unordered_map<u64, u32>& rInverseEdges)									const;
	void		PopulateVisibleFaces	(const std::string& rFileName, const std::vector<u32>& rFaceIndices, std::unordered_set<u32>& rVisibleFaces)	const;
	void		SortFaceIndicesByHeight	(std::vector<u32>& rFaceIndices)												const;
	
	static	u32			ComputeAlpha	(f32 dT, f32 dMinAlpha = 4.0, f32 dMaxAlpha = 64.0);
	static	CVector3	ComputeNormal	(const CVector3& rVector0, const CVector3& rVector1, const CVector3& rVector2, bool bShouldNormalize = false);
	static	u32			ComputeRGB		(u32 uColorLevel);
	static	u32			ComputeU32Log	(u32 x, u32 uBase);
	#if DBG_PH
	static	u32			GetMaskLevel	() { return sm_uMaskLevel; }
	#endif // DBG_PH

// Variables
public:
	std::vector<CVector3>				m_Vertices;
	std::vector<std::pair<u32, u32>>	m_Edges;
	std::vector<std::vector<u32>>		m_Faces;

private:
	#if DBG_PH
	static u32 sm_uMaskLevel;
	#endif // DBG_PH

};

#endif // __POLYHEDRON__