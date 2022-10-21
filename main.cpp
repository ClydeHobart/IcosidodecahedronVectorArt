#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Logging.h"
#include "PhiPolyhedron.h"
#include "PhiVector.h"
#include "PhiVector3.h"
#include "Polygon.h"
#include "Polyhedron.h"
#include "Vector2.h"
#include "Svg.h"

using namespace NLog;

void SavePolyhedronToSvg(u32 uPrintFlags, u32 uPolyhedron, u32 uPerspective, u32 uIteration) {
	CPhiPolyhedron phiPolyhedron;
	const char* pPolyhedronStr;
	u32 uPolyhedronFlag = 0;

	switch (uPolyhedron) {
	case 0:
		phiPolyhedron.GenerateIcosahedronFractal(uIteration);
		pPolyhedronStr = "Icosahedron";
		uPolyhedronFlag = CPolyhedron::Icosahedron;
		break;
	case 1:
		phiPolyhedron.GenerateIcosidodecahedronFractal(uIteration);
		pPolyhedronStr = "Icosidodecahedron";
		uPolyhedronFlag = CPolyhedron::Icosidodecahedron;
		break;
	default:
		return;
	}

	CPolyhedron polyhedron(phiPolyhedron);
	const char* pPerspectiveStr;

	switch (uPerspective) {
	case 0:
		pPerspectiveStr = "AxisOrthogonal";
		break;
	case 1:
		polyhedron.Focus3FoldSymmetry();
		pPerspectiveStr = "3FoldSymmetry";
		break;
	case 2:
		polyhedron.Focus5FoldSymmetry();
		pPerspectiveStr = "5FoldSymmetry";
		break;
	default:
		return;
	}

	std::stringstream ss;

	ss << "images/svg/" <<
		pPolyhedronStr <<
		'_' <<
		pPerspectiveStr <<
		'_' << uIteration <<
		(uPrintFlags & CPolyhedron::CullHiddenFaces ? "_culled" : "") <<
		".svg";
	polyhedron.SaveToSvg(ss.str(), uPrintFlags | uPolyhedronFlag);
	std::cout << ss.str() << " saved\n";
}

void SaveAllSvgs(
	u32 uPrintFlags = 
	CPolyhedron::Verts |
	CPolyhedron::Edges |
	CPolyhedron::Faces |
	CPolyhedron::Color |
	CPolyhedron::Depth,
	u32 uPolyhedronCount = 2,
	u32 uPerspectiveCount = 3,
	u32 uIterationCount = 4,
	u32 uPolyhedronBegin = 0,
	u32 uPerspectiveBegin = 0,
	u32 uIterationBegin = 0) {

	for (u32 uPolyhedron = uPolyhedronBegin; uPolyhedron < uPolyhedronCount; ++uPolyhedron) {
		for (u32 uPerspective = uPerspectiveBegin; uPerspective < uPerspectiveCount; ++uPerspective) {
			for (u32 uIteration = uIterationBegin; uIteration < uIterationCount; ++uIteration) {
				SavePolyhedronToSvg(uPrintFlags, uPolyhedron, uPerspective, uIteration);
			}
		}
	}
}

void PrintAllVertices() {
	CPhiPolyhedron icosidodecahedron(CPhiPolyhedron::GetIcosidodecahedron());
	const std::vector<CPhiVector3>& rVerts = icosidodecahedron.m_Vertices;

	for (u32 i = 0; i < rVerts.size(); ++i) {
		std::cout << rVerts[i] << std::endl;
	}
}

void EvaluateIcosidodecahedronDistances() {
	CPhiPolyhedron icosidodecahedron(CPhiPolyhedron::GetIcosidodecahedron());
	const std::vector<CPhiVector3>& rVerts = icosidodecahedron.m_Vertices;

	for (u32 i = 0; i < rVerts.size(); ++i) {
		for (u32 j = i + 1; j < rVerts.size(); ++j) {
			CPhiVector3 diff = rVerts[i] - rVerts[j];
			CPhiVector distSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			std::cout << static_cast<f32>(distSquared) << ' ' << distSquared << ' ' << i << ' ' << j << std::endl;
		}
	}
}

CPhiVector GenerateRandomPhiVector() {
	std::vector<s32> vec;

	for (u32 i = 0; i < 6; ++i) {
		vec.emplace_back(rand() % 11 - 5);
	}

	return vec;
}

void GeneratePotentiallyEquivalentCPhiVectors() {
	bool bShouldContinue;

	std::cin >> bShouldContinue;

	while (bShouldContinue) {

		if (rand() & 1) {
			CPhiVector phiVector = GenerateRandomPhiVector();
			CPhiVector additiveIdentityVector(std::vector<s32>{1, 1, -1});
			CPhiVector scalingVector(std::vector<s32>{0, 1});

			std::cout << phiVector << std::endl;

			for (u32 i = 0; i < 4; ++i) {
				phiVector += additiveIdentityVector * (rand() % 11 - 5);
				additiveIdentityVector *= scalingVector;
			}

			std::cout << phiVector << std::endl;

			std::string waitStr;

			std::cin >> waitStr;

			std::cout << "Equivalent\n";
		} else {
			std::cout << GenerateRandomPhiVector() << std::endl;
			std::cout << GenerateRandomPhiVector() << std::endl;

			std::string waitStr;

			std::cin >> waitStr;

			std::cout << "Random (potentially equivalent)\n";
		}

		std::cin >> bShouldContinue;
	}
}

void TestPolygonUnion(s32 sFirstIndex = -1, s32 sLastIndex = -1) {
	if (sLastIndex == -1) {
		sLastIndex = sFirstIndex;
	}

	const
		std::vector<
			std::pair<
				std::pair<
					std::vector<
						CVector2
					>, std::vector<
						u32
					>
				>, std::pair<
					std::vector<
						CVector2
					>, std::vector<
						u32
					>
				>
			>
		> polyDefs({
		{ // 0
			{
				{
					{ 1.0f, 1.0f },
					{ 1.0f, 10.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 1.0f }
				}, { 0, 4 }
			}, {
				{
					{ 19.0f, 19.0f },
					{ 19.0f, 10.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 19.0f }
				}, { 0, 4 }
			}
		}, { // 1
			{
				{
					{ 1.0f, 1.0f },
					{ 1.0f, 10.0f },
					{ 15.0f, 10.0f },
					{ 15.0f, 1.0f }
				}, { 0, 4 }
			}, {
				{
					{ 19.0f, 19.0f },
					{ 19.0f, 10.0f },
					{ 5.0f, 10.0f },
					{ 5.0f, 19.0f }
				}, { 0, 4 }
			}
		}, { // 2
			{
				{
					{ 17.0f, 3.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 1.0f },
					{ 1.0f, 1.0f }
				}, { 0, 4 }
			}, {
				{
					{ 17.0f, 3.0f },
					{ 1.0f, 1.0f },
					{ 12.0f, 8.0f },
					{ 19.0f, 19.0f }
				}, { 0, 4 }
			}
		}, { // 3
			{
				{
					{ 8.0f, 10.0f },
					{ 10.0f, 1.0f },
					{ 1.0f, 1.0f },
					{ 1.0f, 19.0f },
					{ 10.0f, 19.0f }
				}, { 0, 5 }
			}, {
				{
					{ 8.0f, 10.0f },
					{ 10.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 1.0f },
					{ 10.0f, 1.0f }
				}, { 0, 5 }
			}
		}, { // 4
			{
				{
					{ 10.0f, 5.0f },
					{ 15.0f, 10.0f },
					{ 19.0f, 10.0f },
					{ 19.0f, 1.0f },
					{ 1.0f, 1.0f },
					{ 1.0f, 10.0f },
					{ 5.0f, 10.0f }
				}, { 0, 7 }
			}, {
				{
					{ 10.0f, 15.0f },
					{ 5.0f, 10.0f },
					{ 1.0f, 10.0f },
					{ 1.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 10.0f },
					{ 15.0f, 10.0f }
				}, { 0, 7 }
			}
		}, { // 5
			{
				{
					{ 1.0f, 1.0f },
					{ 1.0f, 19.0f },
					{ 10.0f, 19.0f },
					{ 10.0f, 10.0f },
					{ 19.0f, 10.0f },
					{ 19.0f, 1.0f }
				}, { 0, 6 }
			}, {
				{
					{ 5.0f, 5.0f },
					{ 5.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 5.0f }
				}, { 0, 4 }
			}
		}, { // 6
			{
				{
					{ 5.0f, 5.0f },
					{ 5.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 5.0f }
				}, { 0, 4 }
			}, {
				{
					{ 5.0f, 5.0f },
					{ 5.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 5.0f }
				}, { 0, 4 }
			}
		}, { // 7
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 15.0f, 03.0f },
					{ 15.0f, 15.0f },
					{ 03.0f, 15.0f },
					{ 08.0f, 08.0f },
					{ 08.0f, 12.0f },
					{ 12.0f, 12.0f },
					{ 12.0f, 08.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 17.0f, 05.0f },
					{ 17.0f, 17.0f },
					{ 05.0f, 17.0f }
				}, { 0, 4, 8 }
			}
		}, { // 8
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 15.0f, 03.0f },
					{ 15.0f, 15.0f },
					{ 03.0f, 15.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 03.0f, 03.0f },
					{ 03.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 03.0f }
				}, { 0, 4 }
			}
		}, { // 9
			{
				{
					{ 1.0f, 1.0f },
					{ 1.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 1.0f }
				}, { 0, 4 }
			}, {
				{
					{ 3.0f, 3.0f },
					{ 3.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 3.0f },
					{ 5.0f, 5.0f },
					{ 15.0f, 5.0f },
					{ 15.0f, 15.0f },
					{ 5.0f, 15.0f }
				}, { 0, 4, 8 }
			}
		}, { // 10
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 10.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 15.0f, 10.0f },
					{ 10.0f, 10.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 05.0f, 10.0f },
					{ 05.0f, 15.0f },
					{ 10.0f, 15.0f },
					{ 10.0f, 10.0f }
				}, { 0, 4 }
			}
		}, { // 11
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 17.0f, 03.0f },
					{ 17.0f, 17.0f },
					{ 03.0f, 17.0f },
					{ 07.0f, 07.0f },
					{ 07.0f, 13.0f },
					{ 13.0f, 13.0f },
					{ 13.0f, 07.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 03.0f, 03.0f },
					{ 03.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 03.0f },
					{ 05.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 15.0f, 15.0f },
					{ 05.0f, 15.0f }
				}, { 0, 4, 8 }
			}
		}, { // 12
			{
				{
					{ 03.0f, 03.0f },
					{ 03.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 03.0f },
					{ 05.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 15.0f, 15.0f },
					{ 05.0f, 15.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 17.0f, 03.0f },
					{ 17.0f, 17.0f },
					{ 03.0f, 17.0f }
				}, { 0, 4, 8 }
			}
		}, { // 13
			{
				{
					{ 03.0f, 03.0f },
					{ 03.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 03.0f },
					{ 07.0f, 07.0f },
					{ 13.0f, 07.0f },
					{ 13.0f, 13.0f },
					{ 07.0f, 13.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 17.0f, 03.0f },
					{ 17.0f, 17.0f },
					{ 03.0f, 17.0f },
					{ 05.0f, 05.0f },
					{ 05.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 05.0f }
				}, { 0, 4, 8, 12 }
			}
		}, { // 14
			{
				{
					{ 05.0f, 04.0f },
					{ 05.0f, 15.0f },
					{ 16.0f, 15.0f },
					{ 16.0f, 04.0f },
					{ 06.0f, 10.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 14.0f },
					{ 06.0f, 14.0f },
					{ 12.0f, 06.0f },
					{ 14.0f, 06.0f },
					{ 14.0f, 08.0f },
					{ 12.0f, 08.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 04.0f, 03.0f },
					{ 04.0f, 16.0f },
					{ 17.0f, 16.0f },
					{ 17.0f, 03.0f },
					{ 05.0f, 04.0f },
					{ 16.0f, 04.0f },
					{ 16.0f, 15.0f },
					{ 05.0f, 15.0f },
					{ 07.0f, 11.0f },
					{ 07.0f, 13.0f },
					{ 09.0f, 13.0f },
					{ 09.0f, 11.0f },
					{ 11.0f, 05.0f },
					{ 11.0f, 09.0f },
					{ 15.0f, 09.0f },
					{ 15.0f, 05.0f }
				}, { 0, 4, 8, 12, 16 }
			}
		}, { // 15
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 18.0f, 03.0f },
					{ 18.0f, 18.0f },
					{ 03.0f, 18.0f },
					{ 05.0f, 05.0f },
					{ 05.0f, 16.0f },
					{ 16.0f, 16.0f },
					{ 16.0f, 05.0f },
					{ 06.0f, 06.0f },
					{ 10.0f, 06.0f },
					{ 10.0f, 10.0f },
					{ 06.0f, 10.0f },
					{ 12.0f, 12.0f },
					{ 14.0f, 12.0f },
					{ 14.0f, 14.0f },
					{ 12.0f, 14.0f }
				}, { 0, 4, 8, 12, 16, 20 }
			}, {
				{
					{ 02.0f, 02.0f },
					{ 02.0f, 03.0f },
					{ 03.0f, 03.0f },
					{ 03.0f, 02.0f },
					{ 04.0f, 04.0f },
					{ 04.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 04.0f },
					{ 05.0f, 05.0f },
					{ 16.0f, 05.0f },
					{ 16.0f, 16.0f },
					{ 05.0f, 16.0f },
					{ 07.0f, 07.0f },
					{ 07.0f, 09.0f },
					{ 09.0f, 09.0f },
					{ 09.0f, 07.0f },
					{ 11.0f, 11.0f },
					{ 11.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 11.0f }
				}, { 0, 4, 8, 12, 16, 20 }
			}
		}, { // 16
			{
				{}, { 0 }
			}, {
				{
					{ 5.0f, 5.0f },
					{ 5.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 5.0f }
				}, { 0, 4 }
			}
		}, { // 17
			{
				{
					{ 5.0f, 5.0f },
					{ 5.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 5.0f }
				}, { 0, 4 }
			}, {
				{}, { 0 }
			}
		}, { // 18
			{
				{
					{01.0f, 10},
					{10.0f, 15},
					{10.0f, 05}
				}, { 0, 3 }
			}, {
				{
					{10.0f, 05.0f},
					{10.0f, 15.0f},
					{19.0f, 10.0f}
				}, { 0, 3 }
			}
		}, { // 19
			{
				{
					{01.0f, 01.0f},
					{01.0f, 19.0f},
					{19.0f, 19.0f},
					{19.0f, 01.0f},
					{05.0f, 05.0f},
					{15.0f, 05.0f},
					{15.0f, 15.0f},
					{05.0f, 15.0f}
				}, { 0, 4, 8 }
			}, {
				{
					{01.0f, 01.0f},
					{01.0f, 05.0f},
					{05.0f, 01.0f}
				}, { 0, 3 }
			}
		}, { // 20
			{
				{
					{ 01.0f, 01.0f },
					{ 02.5f, 10.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 17.5f, 10.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 17.5f, 10.0f },
					{ 15.0f, 15.0f },
					{ 05.0f, 15.0f },
					{ 02.5f, 10.0f }
				}, { 0, 6, 12 }
			}, {
				{
					{01.0f, 01.0f},
					{01.5f, 04.0f},
					{05.0f, 01.0f}
				}, { 0, 3 }
			}
		}, { // 21
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 10.0f, 05.0f },
					{ 10.0f, 10.0f },
					{ 05.0f, 10.0f },
					{ 06.0f, 06.0f },
					{ 06.0f, 09.0f },
					{ 09.0f, 09.0f },
					{ 09.0f, 06.0f },
					{ 10.0f, 10.0f },
					{ 15.0f, 10.0f },
					{ 15.0f, 15.0f },
					{ 10.0f, 15.0f },
					{ 11.0f, 11.0f },
					{ 11.0f, 14.0f },
					{ 14.0f, 14.0f },
					{ 14.0f, 11.0f }
				}, { 0, 4, 8, 12, 16, 20 }
			}, {
				{
					{ 05.0f, 05.0f },
					{ 05.0f, 10.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 05.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 10.0f }
				}, { 0, 4, 8 }
			}
		}, { // 22
			{
				{
					{ 05.0f, 05.0f },
					{ 05.0f, 10.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 05.0f },
					{ 10.0f, 10.0f },
					{ 10.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 10.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 10.0f, 05.0f },
					{ 10.0f, 10.0f },
					{ 05.0f, 10.0f },
					{ 06.0f, 06.0f },
					{ 06.0f, 09.0f },
					{ 09.0f, 09.0f },
					{ 09.0f, 06.0f },
					{ 10.0f, 10.0f },
					{ 15.0f, 10.0f },
					{ 15.0f, 15.0f },
					{ 10.0f, 15.0f },
					{ 11.0f, 11.0f },
					{ 11.0f, 14.0f },
					{ 14.0f, 14.0f },
					{ 14.0f, 11.0f }
				}, { 0, 4, 8, 12, 16, 20 }
			}
		}, { // 23
			{
				{
					{ 01.0f, 01.0f },
					{ 02.5f, 10.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 17.5f, 10.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 17.5f, 10.0f },
					{ 15.0f, 15.0f },
					{ 05.0f, 15.0f },
					{ 02.5f, 10.0f }
				}, { 0, 6, 12 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 02.5f, 10.0f }
				}, { 0, 3 }
			}
		}, { // 24
			{
				{
					{ 01.0f, 03.0f },
					{ 01.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 01.0f },
					{ 03.0f, 01.0f },
					{ 03.0f, 03.0f },
					{ 17.0f, 03.0f },
					{ 17.0f, 17.0f },
					{ 03.0f, 17.0f }
				}, { 0, 5, 9 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 19.0f },
					{ 17.0f, 19.0f },
					{ 19.0f, 17.0f },
					{ 19.0f, 01.0f },
					{ 05.0f, 05.0f },
					{ 15.0f, 05.0f },
					{ 15.0f, 15.0f },
					{ 05.0f, 15.0f }
				}, { 0, 5, 9 }
			}
		}, { // 25
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 15.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 01.0f },
					{ 01.0f, 16.0f },
					{ 01.0f, 19.0f },
					{ 04.0f, 19.0f },
					{ 04.0f, 16.0f }
				}, { 0, 4, 8 }
			}, {
				{
					{ 05.0f, 05.0f },
					{ 05.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 05.0f }
				}, { 0, 4 }
			}
		}, { // 26
			{
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 01.0f, 01.0f },
					{ 13.0f, 01.0f },
					{ 13.0f, 13.0f },
					{ 01.0f, 13.0f },
					{ 02.0f, 02.0f },
					{ 02.0f, 06.0f },
					{ 06.0f, 06.0f },
					{ 06.0f, 02.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 07.0f, 07.0f },
					{ 07.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 07.0f },
					{ 08.0f, 08.0f },
					{ 12.0f, 08.0f },
					{ 12.0f, 12.0f },
					{ 08.0f, 12.0f },
					{ 14.0f, 14.0f },
					{ 18.0f, 14.0f },
					{ 18.0f, 18.0f },
					{ 14.0f, 18.0f }
				}, { 0, 4, 8, 12 }
			}
		}, { // 27
			{
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 07.0f, 07.0f },
					{ 19.0f, 07.0f },
					{ 19.0f, 19.0f },
					{ 07.0f, 19.0f },
					{ 14.0f, 14.0f },
					{ 14.0f, 18.0f },
					{ 18.0f, 18.0f },
					{ 18.0f, 14.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 13.0f },
					{ 13.0f, 13.0f },
					{ 13.0f, 01.0f },
					{ 02.0f, 02.0f },
					{ 06.0f, 02.0f },
					{ 06.0f, 06.0f },
					{ 02.0f, 06.0f },
					{ 08.0f, 08.0f },
					{ 12.0f, 08.0f },
					{ 12.0f, 12.0f },
					{ 08.0f, 12.0f }
				}, { 0, 4, 8, 12 }
			}
		}, { // 28
			{
				{
					{ 07.0f, 07.0f },
					{ 07.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 07.0f },
					{ 08.0f, 08.0f },
					{ 12.0f, 08.0f },
					{ 12.0f, 12.0f },
					{ 08.0f, 12.0f },
					{ 14.0f, 14.0f },
					{ 18.0f, 14.0f },
					{ 18.0f, 18.0f },
					{ 14.0f, 18.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 01.0f, 01.0f },
					{ 13.0f, 01.0f },
					{ 13.0f, 13.0f },
					{ 01.0f, 13.0f },
					{ 02.0f, 02.0f },
					{ 02.0f, 06.0f },
					{ 06.0f, 06.0f },
					{ 06.0f, 02.0f }
				}, { 0, 4, 8, 12 }
			}
		}, { // 29
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 13.0f },
					{ 13.0f, 13.0f },
					{ 13.0f, 01.0f },
					{ 02.0f, 02.0f },
					{ 06.0f, 02.0f },
					{ 06.0f, 06.0f },
					{ 02.0f, 06.0f },
					{ 08.0f, 08.0f },
					{ 12.0f, 08.0f },
					{ 12.0f, 12.0f },
					{ 08.0f, 12.0f }
				}, { 0, 4, 8, 12 }
			}, {
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 07.0f, 07.0f },
					{ 19.0f, 07.0f },
					{ 19.0f, 19.0f },
					{ 07.0f, 19.0f },
					{ 14.0f, 14.0f },
					{ 14.0f, 18.0f },
					{ 18.0f, 18.0f },
					{ 18.0f, 14.0f }
				}, { 0, 4, 8, 12 }
			}
		}, { // 30
			{
				{
					{ 01.0f, 01.0f },
					{ 01.0f, 13.0f },
					{ 13.0f, 13.0f },
					{ 13.0f, 01.0f },
					{ 02.0f, 02.0f },
					{ 06.0f, 02.0f },
					{ 06.0f, 06.0f },
					{ 02.0f, 06.0f },
					{ 08.0f, 08.0f },
					{ 12.0f, 08.0f },
					{ 12.0f, 12.0f },
					{ 08.0f, 12.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 15.0f }
				}, { 0, 4, 8, 12, 16 }
			}, {
				{
					{ 03.0f, 03.0f },
					{ 03.0f, 05.0f },
					{ 05.0f, 05.0f },
					{ 05.0f, 03.0f },
					{ 07.0f, 07.0f },
					{ 07.0f, 19.0f },
					{ 19.0f, 19.0f },
					{ 19.0f, 07.0f },
					{ 09.0f, 09.0f },
					{ 11.0f, 09.0f },
					{ 11.0f, 11.0f },
					{ 09.0f, 11.0f },
					{ 14.0f, 14.0f },
					{ 18.0f, 14.0f },
					{ 18.0f, 18.0f },
					{ 14.0f, 18.0f }
				}, { 0, 4, 8, 12, 16 }
			}
		}, { // 31
			{
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 01.0f, 01.0f },
					{ 13.0f, 01.0f },
					{ 13.0f, 13.0f },
					{ 01.0f, 13.0f },
					{ 03.0f, 03.0f },
					{ 03.0f, 05.0f },
					{ 05.0f, 05.0f },
					{ 05.0f, 03.0f },
					{ 07.5f, 07.5f },
					{ 07.5f, 12.5f },
					{ 12.5f, 12.5f },
					{ 12.5f, 07.5f },
					{ 08.5f, 08.5f },
					{ 11.5f, 08.5f },
					{ 11.5f, 11.5f },
					{ 08.5f, 11.5f },
					{ 14.0f, 14.0f },
					{ 18.0f, 14.0f },
					{ 18.0f, 18.0f },
					{ 14.0f, 18.0f }
				}, { 0, 4, 8, 12, 16, 20, 24 }
			}, {
				{
					{ 00.5f, 00.5f },
					{ 00.5f, 19.5f },
					{ 19.5f, 19.5f },
					{ 19.5f, 00.5f },
					{ 02.0f, 02.0f },
					{ 06.0f, 02.0f },
					{ 06.0f, 06.0f },
					{ 02.0f, 06.0f },
					{ 07.0f, 07.0f },
					{ 19.0f, 07.0f },
					{ 19.0f, 19.0f },
					{ 07.0f, 19.0f },
					{ 08.0f, 08.0f },
					{ 08.0f, 12.0f },
					{ 12.0f, 12.0f },
					{ 12.0f, 08.0f },
					{ 09.0f, 09.0f },
					{ 11.0f, 09.0f },
					{ 11.0f, 11.0f },
					{ 09.0f, 11.0f },
					{ 15.0f, 15.0f },
					{ 15.0f, 17.0f },
					{ 17.0f, 17.0f },
					{ 17.0f, 15.0f }
				}, { 0, 4, 8, 12, 16, 20, 24 }
			}
		}
	});
	CVector2 translation(20.0f, 0.0f);

	sFirstIndex = sFirstIndex == -1 ? 0 : sFirstIndex;
	sLastIndex = sFirstIndex == -1 ? polyDefs.size() - 1 : std::min<u32>(sLastIndex, polyDefs.size() - 1);

	for (s32 sTestCase = sFirstIndex; sTestCase <= sLastIndex; ++sTestCase) {
		g_log << "Running test case " << sTestCase << endl << indent;

		std::stringstream ss;

		ss << "unionTests/unionTest" << sTestCase << ".svg";

		CSvg svg(ss.str(), CVector2(0.0f, 0.0f), CVector2(60.0f, 20.0f), true);
		CPolygon poly1(polyDefs[sTestCase].first.first, polyDefs[sTestCase].first.second);
		CPolygon poly2(polyDefs[sTestCase].second.first, polyDefs[sTestCase].second.second);
		CPolygon polyUnion(poly1);
		
		polyUnion |= poly2;
		svg << poly1;
		poly2.Translate(translation);
		polyUnion.Translate(translation);
		svg << poly2;
		poly2.Translate(translation);
		polyUnion.Translate(translation);
		svg << polyUnion;
		g_log << "Finished test case " << sTestCase << endl << unindent;
	}
}

void TestPolygonUnion(s32 sArgCount, char** pArgValues) {
	switch (sArgCount) {
		case 1: {
			TestPolygonUnion();

			break;
		} case 2: {
			TestPolygonUnion(atoi(pArgValues[1]));

			break;
		} case 3: {
			TestPolygonUnion(atoi(pArgValues[1]), atoi(pArgValues[2]));

			break;
		} default: {
			// invalid
			break;
		}
	}
}

s32 main(s32 sArgCount, char* aArgValues[]) {
	SaveAllSvgs(
		CPolyhedron::Verts |
		CPolyhedron::Edges |
		CPolyhedron::Faces |
		CPolyhedron::Color |
		CPolyhedron::Depth |
		// CPolyhedron::CullHiddenFaces |
		0,
		2, 3, 4,
		0, 0, 3);

	// TestPolygonUnion(sArgCount, aArgValues);

	return 0;
}
