#include <bit>
#include <iomanip>

#include "PhiPolyhedron.h"

#include "PhiVector.h"
#include "PhiVector3.h"

CPhiPolyhedron::CPhiPolyhedron() {}

CPhiPolyhedron& CPhiPolyhedron::CopyForEachVertex(const std::vector<CPhiVector3>& rVertices) {
	const u32 uInitialVertCount = m_Vertices.size();
	const u32 uInitialEdgeCount = m_Edges.size();
	const u32 uInitialFaceCount = m_Faces.size();
	u32 uVertCount = uInitialVertCount;

	if (rVertices.size() > 1) {
		for (u32 uRootVertIndex = 1; uRootVertIndex < rVertices.size(); ++uRootVertIndex) {

			for (u32 uCopyVertIndex = 0; uCopyVertIndex < uInitialVertCount; ++uCopyVertIndex) {
				m_Vertices.emplace_back(rVertices[uRootVertIndex] + m_Vertices[uCopyVertIndex]);
			}

			for (u32 uCopyEdgeIndex = 0; uCopyEdgeIndex < uInitialEdgeCount; ++uCopyEdgeIndex) {
				const std::pair<u32, u32>& rCopyEdge = m_Edges[uCopyEdgeIndex];
				m_Edges.emplace_back(uVertCount + rCopyEdge.first, uVertCount + rCopyEdge.second);
			}

			for (u32 uCopyFaceIndex = 0; uCopyFaceIndex < uInitialFaceCount; ++uCopyFaceIndex) {
				std::vector<u32> copyFace = m_Faces[uCopyFaceIndex];

				for (u32 uFaceVert = 0; uFaceVert < copyFace.size(); ++uFaceVert) {
					copyFace[uFaceVert] += uVertCount;
				}

				m_Faces.push_back(copyFace);
			}

			uVertCount += uInitialVertCount;
		}
	}

	for (u32 uOrigVertIndex = 0; uOrigVertIndex < uInitialVertCount; ++uOrigVertIndex) {
		m_Vertices[uOrigVertIndex] += rVertices[0];
	}

	return *this;
}

CPhiPolyhedron& CPhiPolyhedron::GenerateIcosahedronFractal(u32 uIteration) {
	*this = GetIcosahedron();

	if (!uIteration) {
		return *this;
	}

	CPhiPolyhedron scalingIcosahedron(GetIcosahedron());
	CPhiVector scalingVector(std::vector<s32>{0, 1});

	scalingIcosahedron *= scalingVector;
	CopyForEachVertex(scalingIcosahedron.m_Vertices);
	scalingVector *= scalingVector;

	while (--uIteration) {
		scalingIcosahedron *= scalingVector;
		CopyForEachVertex(scalingIcosahedron.m_Vertices);
	}

	return *this;
}

CPhiPolyhedron& CPhiPolyhedron::GenerateIcosidodecahedronFractal(u32 uIteration) {
	*this = GetIcosidodecahedron();

	if (!uIteration) {
		return *this;
	}

	CPhiPolyhedron scalingPolyhedron(GetIcosidodecahedron());

	scalingPolyhedron *= std::vector<s32>{0, 2};
	CopyForEachVertex(scalingPolyhedron.m_Vertices);

	CPhiVector scalingVector(std::vector<s32>{0, 1, 1});

	while (--uIteration) {
		scalingPolyhedron *= scalingVector;
		CopyForEachVertex(scalingPolyhedron.m_Vertices);
	}

	return *this;
}

const CPhiPolyhedron& CPhiPolyhedron::GetIcosahedron() {
	if (!m_sIcosahedron.m_Vertices.size()) {
		GenerateIcosahedron();
	}

	return m_sIcosahedron;
}

const CPhiPolyhedron& CPhiPolyhedron::GetIcosidodecahedron() {
	if (!m_sIcosidodecahedron.m_Vertices.size()) {
		GenerateIcosidodecahedron();
	}

	return m_sIcosidodecahedron;
}

CPhiPolyhedron CPhiPolyhedron::operator*(const CPhiVector& rPhiVector) const {
	return CPhiPolyhedron(*this) *= rPhiVector;
}

CPhiPolyhedron CPhiPolyhedron::operator*(s32 sScalar) const {
	return CPhiPolyhedron(*this) *= sScalar;
}

CPhiPolyhedron& CPhiPolyhedron::operator*=(const CPhiVector& rPhiVector) {
	for (u32 v = 0; v < m_Vertices.size(); ++v) {
		m_Vertices[v] *= rPhiVector;
	}

	return *this;
}

CPhiPolyhedron& CPhiPolyhedron::operator*=(s32 sScalar) {
	for (u32 v = 0; v < m_Vertices.size(); ++v) {
		m_Vertices[v] *= sScalar;
	}

	return *this;
}

std::ostream& operator<<(std::ostream& rOStream, const CPhiPolyhedron& rPhiPolyhedron) {
	rOStream << "{\nVertices (" << rPhiPolyhedron.m_Vertices.size() << "):\n";

	for (u32 v = 0; v < rPhiPolyhedron.m_Vertices.size(); ++v) {
		rOStream << "\t" << std::setw(4) << v << ": " << rPhiPolyhedron.m_Vertices[v] << '\n';
	}

	rOStream << "Edges (" << rPhiPolyhedron.m_Edges.size() << "):\n";

	for (u32 e = 0; e < rPhiPolyhedron.m_Edges.size(); ++e) {
		const std::pair<u32, u32>& rEdge = rPhiPolyhedron.m_Edges[e];
		rOStream << "\t" << std::setw(4) << e << ": (" << rEdge.first << ", " << rEdge.second << ")\n";
	}

	rOStream << "Faces (" << rPhiPolyhedron.m_Faces.size() << "):\n";

	for (u32 f = 0; f < rPhiPolyhedron.m_Faces.size(); ++f) {
		const std::vector<u32>& rFace = rPhiPolyhedron.m_Faces[f];
		rOStream << "\t" << std::setw(4) << f << ": (";

		if (rFace.size()) {
			rOStream << rFace[0];

			for (u32 fv = 1; fv < rFace.size(); ++fv) {
				rOStream << ", " << rFace[fv];
			}
		}

		rOStream << ")\n";
	}

	return rOStream << "}\n";
}

void CPhiPolyhedron::GenerateIcosahedron() {
	const CPhiVector3 baseVector(
		std::vector<s32>{1},
		std::vector<s32>{0, 1},
		std::vector<s32>{0});

	std::vector<CPhiVector3>& rVerts = m_sIcosahedron.m_Vertices;

	for (u32 i = 0; i < 3; ++i) {
		for (u32 j = 0; j < 4; ++j) {
			const CPhiVector3 reflectedVector(
				baseVector.x * (j & 2 ? -1 : 1),
				baseVector.y * (j & 1 ? -1 : 1),
				baseVector.z);

			rVerts.emplace_back(
				reflectedVector[(i    ) % 3],
				reflectedVector[(i + 1) % 3],
				reflectedVector[(i + 2) % 3]);
		}
	}

	std::vector<std::pair<u32, u32>>& rEdges = m_sIcosahedron.m_Edges;
	std::vector<std::vector<u32>>& rFaces = m_sIcosahedron.m_Faces;

	for (u32 i = 0; i < 11; ++i) {
		for (u32 j = i + 1; j < 12; ++j) {
			if (static_cast<f32>((rVerts[i] - rVerts[j]).GetMagnitudeSquared()) < 5.0f) {
				rEdges.emplace_back(i, j);

				for (u32 k = j + 1; k < 12; ++k) {
					if (static_cast<f32>((rVerts[i] - rVerts[k]).GetMagnitudeSquared()) < 5.0f &&
						static_cast<f32>((rVerts[j] - rVerts[k]).GetMagnitudeSquared()) < 5.0f) {
						rFaces.push_back(std::vector<u32>{i, j, k});
					}
				}
			}
		}
	}
}

void CPhiPolyhedron::GenerateIcosidodecahedron() {
	std::vector<CPhiVector3>& rIcosaVerts = m_sIcosahedron.m_Vertices;

	if (!rIcosaVerts.size()) {
		GenerateIcosahedron();
	}

	std::vector<std::pair<u32, u32>>& rIcosaEdges = m_sIcosahedron.m_Edges;
	std::vector<CPhiVector3>& rVerts = m_sIcosidodecahedron.m_Vertices;

	for (u32 i = 0; i < 30; ++i) {
		const std::pair<u32, u32>& rIcosaEdge = rIcosaEdges[i];

		rVerts.emplace_back(rIcosaVerts[rIcosaEdge.first] + rIcosaVerts[rIcosaEdge.second]);
	}

	std::vector<std::pair<u32, u32>>& rEdges = m_sIcosidodecahedron.m_Edges;
	std::vector<u32> edgeMap(30, 0);

	for (u32 i = 0; i < 29; ++i) {
		for (u32 j = i + 1; j < 30; ++j) {
			if (static_cast<f32>((rVerts[i] - rVerts[j]).GetMagnitudeSquared()) < 5.0f) {
				rEdges.emplace_back(i, j);
				edgeMap[i] |= 1 << j;
				edgeMap[j] |= 1 << i;
			}
		}
	}

	const CPhiVector adjacentPhiVectorSquared(std::vector<s32>{4});
	const CPhiVector acrossPhiVectorSquared(std::vector<s32>{0, 0, 4});
	std::vector<std::vector<u32>>& rFaces = m_sIcosidodecahedron.m_Faces;
	std::vector<u32> octants(8, 0);

	for (u32 i = 0; i < 30; ++i) {
		CPhiVector3& rVert = rVerts[i];

		if (!rVert.x[0] && !rVert.y[0] && !rVert.z[0]) {
			u32 aNeighbors[4];
			u32 uEdges = edgeMap[i];

			// Unpack the adjacent vertex indices
			for (u32 n = 0; n < 4; ++n) {
				aNeighbors[n] = std::countr_zero(uEdges);
				uEdges &= ~(1 << aNeighbors[n]);
			}

			// Make aNeighbors[0] and aNeighbors[1] be on the same pentagon (likewise with 2 and 3)
			for (u32 n = 1; n < 4; ++n) {
				if ((rVerts[aNeighbors[0]] - rVerts[aNeighbors[n]]).GetMagnitudeSquared() <=> acrossPhiVectorSquared == 0) {
					std::swap(aNeighbors[n], aNeighbors[1]);

					break;
				}
			}

			// Make aNeighbors[0] and aNeighbors[2] be on the same triangle (likewise with 1 and 3)
			if ((rVerts[aNeighbors[0]] - rVerts[aNeighbors[2]]).GetMagnitudeSquared() <=> adjacentPhiVectorSquared != 0) {
				std::swap(aNeighbors[2], aNeighbors[3]);
			}

			rFaces.push_back(std::vector<u32>{i, aNeighbors[0], aNeighbors[2]});
			rFaces.push_back(std::vector<u32>{i, aNeighbors[1], aNeighbors[3]});

			u32 aAcrosses[4];

			for (u32 n = 0; n < 4; ++n) {
				u32 uNeighborEdges = edgeMap[aNeighbors[n]];

				for (u32 a = 0; a < 4; ++a) {
					u32 uAcross = std::countr_zero(uNeighborEdges);

					if ((rVerts[i] - rVerts[uAcross]).GetMagnitudeSquared() <=> acrossPhiVectorSquared == 0) {
						aAcrosses[n] = uAcross;

						break;
					}

					uNeighborEdges &= ~(1 << uAcross);
				}
			}

			rFaces.push_back(std::vector<u32>{i, aNeighbors[0], aAcrosses[0], aAcrosses[1], aNeighbors[1]});
			rFaces.push_back(std::vector<u32>{i, aNeighbors[2], aAcrosses[2], aAcrosses[3], aNeighbors[3]});
		} else {
			octants[
				((static_cast<f32>(rVert.x) < 0.0)     ) |
				((static_cast<f32>(rVert.y) < 0.0) << 1) |
				((static_cast<f32>(rVert.z) < 0.0) << 2)] |= 1 << i; 
		}
	}

	for (u32 o = 0; o < 8; ++o) {
		std::vector<u32> face;
		u32 uOctant = octants[o];

		// Unpack the octant vertex indices
		for (u32 n = 0; n < 3; ++n) {
			u32 vert = std::countr_zero(uOctant);

			uOctant &= ~(1 << vert);
			face.emplace_back(vert);
		}

		rFaces.push_back(face);
	}
}

CPhiPolyhedron CPhiPolyhedron::m_sIcosahedron;
CPhiPolyhedron CPhiPolyhedron::m_sIcosidodecahedron;