#include <cmath>
#include <chrono>
#include <float.h>
#include <fstream>
#include <iomanip>

#include "Polyhedron.h"

#include "Logging.h"
#include "PhiPolyhedron.h"
#include "PhiVector3.h"
#include "Svg.h"
#include "Vector2.h"
#include "Vector3.h"

#define DBG_PH_PVF		(DBG_PH		&& ON)
#define DBG_PH_PVF_SVG	(DBG_PH_PVF	&& OFF)
#define DBG_PH_STS		(DBG_PH		&& OFF)

using namespace NLog;

#if DBG_PH_PVF
u32 CPolyhedron::sm_uMaskLevel = 0;
#if !DBG_PH_PVF_SVG
std::chrono::time_point<std::chrono::steady_clock> (&now)() = std::chrono::steady_clock::now;
typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> time_point;
typedef std::chrono::nanoseconds nanoseconds;

// Taken directly from: https://stackoverflow.com/questions/22590821/convert-stdduration-to-human-readable-time
std::ostream& operator<<(std::ostream& os, std::chrono::nanoseconds ns) {
	using namespace std::chrono;
	using days = duration<int, std::ratio<86400>>;

	auto d = duration_cast<days>(ns);

	ns -= d;

	auto h = duration_cast<hours>(ns);

	ns -= h;

	auto m = duration_cast<minutes>(ns);

	ns -= m;

	auto s = duration_cast<seconds>(ns);

	ns -= s;

	u64 ms = duration_cast<milliseconds>(ns).count();

	if (d.count()) {
		os << d.count() << "d ";
	}

	if (d.count() || h.count()) {
		os << std::setw(2) << h.count() << ':';
	}

	if (d.count() || h.count() || m.count()) {
		os << std::setw(2) << m.count() << ':';
	}

	os << std::setw(2) << s.count() << '.' <<
		std::setw(3) << ms;

	if (!d.count() && !h.count() && !m.count()) {
		os << 's';
	}

	return os;
}

#endif // !DBG_PH_PVF_SVG
#endif // DBG_PH_PVF

CPolyhedron::CPolyhedron() {}

CPolyhedron::CPolyhedron(const CPhiPolyhedron& rPhiPolyhedron) : m_Edges(rPhiPolyhedron.m_Edges), m_Faces(rPhiPolyhedron.m_Faces) {
	const std::vector<CPhiVector3>& rVerts = rPhiPolyhedron.m_Vertices;

	for (u32 i = 0; i < rVerts.size(); ++i) {
		m_Vertices.emplace_back(rVerts[i]);
	}
}

CPolyhedron& CPolyhedron::Focus3FoldSymmetry() {
	const f32 C = 0.93417235896271570f, S = 0.35682208977308993f;

	for (u32 v = 0; v < m_Vertices.size(); ++v) {
		CVector3& rVert = m_Vertices[v];
		const f32 fNewX =  C * rVert.x + S * rVert.z;
		const f32 fNewZ = -S * rVert.x + C * rVert.z;
		rVert.x = fNewX;
		rVert.z = fNewZ;
	}

	return *this;
}

CPolyhedron& CPolyhedron::Focus5FoldSymmetry() {
	const f32 C = 0.85065080835203993f, S = 0.52573111211913361f;

	for (u32 i = 0; i < m_Vertices.size(); ++i) {
		CVector3& rVert = m_Vertices[i];
		const f32 fNewY = C * rVert.y - S * rVert.z;
		const f32 fNewZ = S * rVert.y + C * rVert.z;
		rVert.y = fNewY;
		rVert.z = fNewZ;
	}

	return *this;
}

bool CPolyhedron::SaveToSvg(std::string fileName, u32 uPrintFlags) const {
	if (!(uPrintFlags & (Verts | Edges | Faces))) {
		return false;
	}

	f32 dMinX = FLT_MAX, dMinY = FLT_MAX, dMaxX = -FLT_MAX, dMaxY = -FLT_MAX;

	for (u32 i = 0; i < m_Vertices.size(); ++i) {
		const CVector3& rVert = m_Vertices[i];

		dMinX = std::min(dMinX, rVert.x);
		dMinY = std::min(dMinY, rVert.y);
		dMaxX = std::max(dMaxX, rVert.x);
		dMaxY = std::max(dMaxY, rVert.y);
	}

	s32 sMinX = dMinX, sMinY = dMinY, sMaxX = dMaxX, sMaxY = dMaxY;

	sMinY = sMinY * 2;
	sMaxX = sMaxX * 2;
	sMinX = sMinX * 2;
	sMaxY = sMaxY * 2;

	std::ofstream file(fileName.c_str(), std::ios_base::trunc);

	if (!file.is_open()) {
		return false;
	}

	file << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" <<
		sMinX << ' ' << sMinY << ' ' << sMaxX - sMinX << ' ' << sMaxY - sMinY << "\">\n" <<
		std::fixed << std::setprecision(5) << std::setfill('0');

	if (uPrintFlags & Depth && uPrintFlags & Faces) {
		std::vector<u32> faceIndices(m_Faces.size(), 0);
		std::unordered_map<u64, u32> inverseEdges;
		std::unordered_set<u32> visibleFaces;

		SortFaceIndicesByHeight(faceIndices);
		PopulateInverseEdges(inverseEdges);

		if (uPrintFlags & CullHiddenFaces) {
			PopulateVisibleFaces(fileName, faceIndices, visibleFaces);
		}

		for (u32 fi = 0; fi < faceIndices.size(); ++fi) {
			const u32 uFaceIndex = faceIndices[fi];

			if (uPrintFlags & CullHiddenFaces && !visibleFaces.contains(uFaceIndex)) {
				#if DBG_PH_STS
					std::cout << "Culling face " << uFaceIndex << std::endl;
				#endif // DBG_PH_STS

				continue;
			}

			const std::vector<u32>& rFace = m_Faces[uFaceIndex];

			if (!rFace.size()) {
				continue;
			}

			const CVector3& rVert0 = m_Vertices[rFace[0]];

			file << "<path fill=\"#" << std::hex << std::setw(6) << ComputeRGBA(uPrintFlags, uFaceIndex) << std::dec << "\" d=\"M " << rVert0.x << ' ' << rVert0.y;

			for (u32 fv = 1; fv < rFace.size(); ++fv) {
				const CVector3& rVert = m_Vertices[rFace[fv]];

				file << " L " << rVert.x << ' ' << rVert.y;
			}

			file << " Z\"/>\n";

			if (uPrintFlags & Edges) {
				file << "<g fill=\"none\" stroke-width=\"0.03125\" stroke-linecap=\"round\">\n";

				for (u32 fv = 0; fv < rFace.size(); ++fv) {
					u32 uVertIndex1 = rFace[fv];
					u32 uVertIndex2 = rFace[(fv + 1) % rFace.size()];

					if (uVertIndex1 > uVertIndex2) {
						std::swap(uVertIndex1, uVertIndex2);
					}

					const u32 uEdgeIndex = inverseEdges[static_cast<u64>(uVertIndex1) + (static_cast<u64>(uVertIndex2) << 32)];

					file << "<path stroke=\"#" << std::hex << std::setw(6) << ComputeRGB(uPrintFlags, Edges, uEdgeIndex) << std::dec << "\"";

					const CVector3& rVert1 = m_Vertices[uVertIndex1];
					const CVector3& rVert2 = m_Vertices[uVertIndex2];

					file << " d=\"M " << rVert1.x << ' ' << rVert1.y << " L " << rVert2.x << ' ' << rVert2.y << "\"/>\n";
				}

				file << "</g>\n";
			}

			if (uPrintFlags & Verts) {
				file << "<g stroke=\"none\">\n";

				for (u32 fv = 0; fv < rFace.size(); ++fv) {
					const CVector3& rVert = m_Vertices[rFace[fv]];

					file << "<circle fill=\"#" << std::hex << std::setw(6) << ComputeRGB(uPrintFlags, Verts, rFace[fv]) << std::dec << "\" cx=\"" << rVert.x << "\" cy=\"" << rVert.y << "\" r=\"0.0625\"/>\n";
				}

				file << "</g>\n";
			}
		}
	} else {
		if (uPrintFlags & Faces) {
			file << "<g stroke=\"none\">\n";

			for (u32 i = 0; i < m_Faces.size(); ++i) {
				const std::vector<u32>& rFace = m_Faces[i];
				const CVector3 faceNormal = GetFaceNormal(i);

				file << "<path fill=\"#" << std::hex << std::setw(8) <<
					(((uPrintFlags & Color ? ComputeRGB(uPrintFlags, Faces, i) : 0x00FF00) << 8) + 
					ComputeAlpha(sqrt(
						(faceNormal.x * faceNormal.x + faceNormal.y * faceNormal.y) /
						faceNormal.GetMagnitudeSquared()))) << std::dec <<
					"\"";

				if (rFace.size()) {
					const CVector3& rVert0 = m_Vertices[rFace[0]];

					file << " d=\"M " << rVert0.x << ' ' << rVert0.y;

					for (u32 fv = 1; fv < rFace.size(); ++fv) {
						const CVector3& rVert = m_Vertices[rFace[fv]];
						file << " L " << rVert.x << ' ' << rVert.y;
					}

					file << " Z\"";
				}

				file << "/>\n";
			}

			file << "</g>\n";
		}

		if (uPrintFlags & Edges) {
			if (uPrintFlags & Color) {
				for (u32 i = 0; i < m_Edges.size(); ++i) {
					const std::pair<u32, u32>& rEdge = m_Edges[i];
					const CVector3& rStartVert = m_Vertices[rEdge.first];
					const CVector3& rEndVert = m_Vertices[rEdge.second];

					file << "<path stroke-width=\"0.03125\" fill=\"none\" stroke-linecap=\"round\" stroke=\"#" << std::hex << std::setw(8) <<
						ComputeRGB(uPrintFlags, Edges, i) << std::dec <<
						"\" d=\"M " << rStartVert.x << ' ' << rStartVert.y << " L " << rEndVert.x << ' ' << rEndVert.y << "\"/>\n";
				}
			} else {
				file << "<path stroke-width=\"0.03125\" fill=\"none\" stroke-linecap=\"round\" stroke=\"black\" d=\"\n";

				for (u32 i = 0; i < m_Edges.size(); ++i) {
					const std::pair<u32, u32>& rEdge = m_Edges[i];
					const CVector3& rStartVert = m_Vertices[rEdge.first];
					const CVector3& rEndVert = m_Vertices[rEdge.second];

					file << "M " << rStartVert.x << ' ' << rStartVert.y << " L " << rEndVert.x << ' ' << rEndVert.y << '\n';
				}

				file << "\"/>\n";
			}
		}

		if (uPrintFlags & Verts) {
			if (uPrintFlags & Color) {
				for (u32 i = 0; i < m_Vertices.size(); ++i) {
					const CVector3& rVert = m_Vertices[i];

					file << "<circle stroke=\"none\" fill=\"#" << std::hex << std::setw(8) <<
						ComputeRGB(uPrintFlags, Verts, i) << std::dec <<
						"\" cx=\"" << rVert.x << "\" cy=\"" << rVert.y << "\" r=\"0.0625\"/>\n";
				}
			} else  {
				file << "<g stroke=\"none\" fill=\"green\">\n";

				for (u32 i = 0; i < m_Vertices.size(); ++i) {
					const CVector3& rVert = m_Vertices[i];

					file << "<circle cx=\"" << rVert.x << "\" cy=\"" << rVert.y << "\" r=\"0.0625\"/>\n";
				}

				file << "</g>\n";
			}
		}
	}

	file << "</svg>\n";

	return true;
}

bool CPolyhedron::CompareFaceIndices(u32 uFaceIndexA, u32 uFaceIndexB) const {
	if (std::max(uFaceIndexA, uFaceIndexB) >= m_Faces.size()) {
		std::cout << "CPolyhedron::SortFaceIndicesByHeight(): invalid index: too large\n";

		return false;
	}

	const std::vector<u32>& rFaceA = m_Faces[uFaceIndexA];
	const std::vector<u32>& rFaceB = m_Faces[uFaceIndexB];
	f32 dFaceZSumA = 0.0, dFaceZSumB = 0.0;

	for (u32 va = 0; va < rFaceA.size(); ++va) {
		dFaceZSumA += m_Vertices[rFaceA[va]].z;
	}

	for (u32 vb = 0; vb < rFaceB.size(); ++vb) {
		dFaceZSumB += m_Vertices[rFaceB[vb]].z;
	}

	return (dFaceZSumA / std::max(rFaceA.size(), 1ul)) < (dFaceZSumB / std::max(rFaceB.size(), 1ul));
}

u32 CPolyhedron::ComputeRGB(u32 uPrintFlags, u32 uIndexType, u32 uIndex) const {
	const u32 uBlack = 0x000000;

	if (!(uPrintFlags & Color && (uPrintFlags & Icosahedron || uPrintFlags & Icosidodecahedron)) || !(uPrintFlags & uIndexType)) {
		return uBlack;
	}

	const u32 uBaseVerts = uPrintFlags & Icosahedron ? 12 : 30;
	u32 uBaseTypeCount;
	u32 uIterations = 1;
	u32 uIndexMatches = 1;
	u32 uOriginalIndex;

	switch (uIndexType) {
		case Verts:

			uBaseTypeCount = uBaseVerts;
			uIterations = ComputeU32Log(m_Vertices.size(), uBaseVerts);
			uOriginalIndex = uIndex % uBaseTypeCount;
			uIndex /= uBaseTypeCount;

			for (u32 i = 1; i < uIterations; ++i) {
				if (uIndex % uBaseVerts != uOriginalIndex) {
					return ComputeRGB(uIterations - uIndexMatches);
				}

				uIndex /= uBaseVerts;
				++uIndexMatches;
			}

			break;
		case Edges: {
			uBaseTypeCount = uPrintFlags & Icosahedron ? 30 : 60;
			uIterations = ComputeU32Log(m_Edges.size() / uBaseTypeCount, uBaseVerts) + 1;
			uOriginalIndex = uIndex % uBaseTypeCount;
			uIndex /= uBaseTypeCount;

			const std::pair<u32, u32>& rEdge = m_Edges[uOriginalIndex];

			for (u32 i = 1; i < uIterations; ++i) {
				const u32 uCurrIndex = uIndex % uBaseVerts;

				if (uCurrIndex != rEdge.first && uCurrIndex != rEdge.second) {
					return ComputeRGB(uIterations - uIndexMatches);
				}

				uIndex /= uBaseVerts;
				++uIndexMatches;
			}

			break;
		}
		
		case Faces: {
			uBaseTypeCount = uPrintFlags & Icosahedron ? 20 : 32;
			uIterations = ComputeU32Log(m_Edges.size() / uBaseTypeCount, uBaseVerts) + 1;
			uOriginalIndex = uIndex % uBaseTypeCount;
			uIndex /= uBaseTypeCount;

			const std::vector<u32>& rFace = m_Faces[uOriginalIndex];

			for (u32 i = 1; i < uIterations; ++i) {
				const u32 uCurrIndex = uIndex % uBaseVerts;
				bool bMatchWasFound = false;

				for (u32 fv = 0; fv < rFace.size(); ++fv) {
					bMatchWasFound |= uCurrIndex == rFace[fv];
				}

				if (!bMatchWasFound) {
					return ComputeRGB(uIterations - uIndexMatches);
				}

				uIndex /= uBaseVerts;
				++uIndexMatches;
			}

			break;
		}

		default:
			return uBlack;

			break;
	}

	return ComputeRGB(uIterations - uIndexMatches);
}

u32 CPolyhedron::ComputeRGBA(u32 uPrintFlags, u32 uFaceIndex) const {
	const CVector3 faceNormal = GetFaceNormal(uFaceIndex);
	u32 uRGBA = (ComputeRGB(uPrintFlags, Faces, uFaceIndex) << 8) + 
		ComputeAlpha(sqrt((faceNormal.x * faceNormal.x + faceNormal.y * faceNormal.y) / faceNormal.GetMagnitudeSquared()));

	const float fOpacityBonus = (255 - (uRGBA & 0xFF)) / 255.0f;
	u8* pAlphaByte = static_cast<u8*>(static_cast<void*>(&uRGBA));

	for (u32 cb = 0; cb < 3; ++cb) {
		u8* pColorByte = &pAlphaByte[cb + 1];

		*pColorByte += static_cast<u8>((255 - *pColorByte) * fOpacityBonus);
	}

	return uRGBA >> 8;
}

CVector3 CPolyhedron::GetFaceNormal(u32 uFace, bool bShouldNormalize) const {
	const std::vector<u32>& rFace = m_Faces[uFace];

	return ComputeNormal(m_Vertices[rFace[0]], m_Vertices[rFace[1]], m_Vertices[rFace[2]], bShouldNormalize);
}

CPolygon CPolyhedron::GeneratePolygonForFace(u32 uFaceIndex) const {
	std::vector<CVector2> polyVertices;

	for (u32 uVertIndex : m_Faces[uFaceIndex]) {
		polyVertices.emplace_back(m_Vertices[uVertIndex]);
	}

	return polyVertices;
}

void CPolyhedron::PopulateInverseEdges(std::unordered_map<u64, u32>& rInverseEdges) const {
	for (u32 e = 0; e < m_Edges.size(); ++e) {
		const std::pair<u32, u32>& rEdge = m_Edges[e];

		rInverseEdges.emplace(static_cast<u64>(rEdge.first) + (static_cast<u64>(rEdge.second) << 32), e);
	}
}

void CPolyhedron::PopulateVisibleFaces(const std::string& rFileName, const std::vector<u32>& rFaceIndices, std::unordered_set<u32>& rVisibleFaces) const {
	if (!rFaceIndices.size()) {
		return;
	}

	CVector2::ResetCache();

	CPolygon polyMask;

	#if DBG_PH_PVF
		std::string polyhedronName(rFileName.substr(0, rFileName.size() - 4));

		sm_uMaskLevel = 0;

		#if !DBG_PH_PVF_SVG
			enum {
				Hidden,
				Visible
			};

			const u32 uFaceCount = m_Faces.size();
			u32 uRemainingFaces = uFaceCount;
			const u32 uMaxDigits = static_cast<u32>(log10(uFaceCount)) + 1;
			std::vector<u64> aRecentDurations[2] = { std::vector<u64>(16), std::vector<u64>(16) };
			u32 auRecentDurationIndices[2] = { 0, 0 };
			u64 auRecentDurationSums[2] = { 0, 0 };
			const time_point initTime = now();
			time_point prevTime = initTime;

			std::cout << std::setprecision(3) << std::fixed << std::setfill('0');
		#endif // !DBG_PH_PVF_SVG
	#endif // DBG_PH_PVF

	for (std::vector<u32>::const_reverse_iterator faceIndicesIter = rFaceIndices.rbegin(); faceIndicesIter != rFaceIndices.rend(); ++faceIndicesIter) {
		#if DBG_PH_PVF_SVG
			std::stringstream svgName;

			svgName << "polyMasks/" << polyhedronName << '_' << sm_uMaskLevel;
			g_log << svgName.str() << ":\n" << indent;
		#endif // DBG_PH_PVF_SVG
		
		CPolygon polyForFace(GeneratePolygonForFace(*faceIndicesIter));

		polyMask |= polyForFace;

		if (polyMask.WereAnyNewLoopsAdded()) {
			rVisibleFaces.insert(*faceIndicesIter);
		}

		#if DBG_PH_PVF
			#if DBG_PH_PVF_SVG
				svgName << (polyMask.WereAnyNewLoopsAdded() ? "" : "_culled") << ".svg";

				const CExtrema& rPolyMaskExtrema = polyMask.GetExtrema(0);
				const std::string svgNameStr = svgName.str();
				CSvg maskSvg(svgName.str(), rPolyMaskExtrema.m_vMin * 1.1f, rPolyMaskExtrema.m_vMax * 1.1f);

				maskSvg << polyMask;

				if (!polyMask.WereAnyNewLoopsAdded()) {
					maskSvg.SetStrokeColor(CColor::sm_kBlue);
					maskSvg.SetStrokeWidth(0.125f);
					maskSvg << polyForFace;
				}

				g_log << svgNameStr << " done\n" << unindent;
				++sm_uMaskLevel;
			#else // DBG_PH_PVF_SVG
				++sm_uMaskLevel;
				--uRemainingFaces;
				const u32 uVisible = rVisibleFaces.size();
				const f32 fCompletionPercent = sm_uMaskLevel * 100.0f / uFaceCount;
				const f32 fVisiblePortion = static_cast<f32>(uVisible) / sm_uMaskLevel;
				const time_point currTime = now();
				const nanoseconds totalDuration = std::chrono::duration_cast<nanoseconds>(currTime - initTime);
				const f32 fAverageDuration = static_cast<f32>(totalDuration.count()) / sm_uMaskLevel;
				const bool bIsVisible = polyMask.WereAnyNewLoopsAdded();

				auRecentDurationSums[bIsVisible] += (aRecentDurations[bIsVisible][(auRecentDurationIndices[bIsVisible]++) & 0xF] = (currTime - prevTime).count());

				const u64 auAverageDurations[2] = { auRecentDurationSums[Hidden] / std::clamp(auRecentDurationIndices[Hidden], 1u, 16u), auRecentDurationSums[Visible] / std::clamp(auRecentDurationIndices[Visible], 1u, 16u) };

				std::cout << polyhedronName << ": " <<
					std::setw(uMaxDigits) << sm_uMaskLevel << '/' << std::setw(uMaxDigits) << uFaceCount << " (" << fCompletionPercent << "%) done; " <<
					std::setw(uMaxDigits) << uVisible << '/' << std::setw(uMaxDigits) << sm_uMaskLevel << " (" << (100.0f * fVisiblePortion) << "%) <0>; " <<
					totalDuration << " so far; " <<
					nanoseconds(static_cast<u64>(fAverageDuration)) << " avg (~" << nanoseconds(static_cast<u64>(fAverageDuration * uRemainingFaces)) << " left); " <<
					nanoseconds(auAverageDurations[Hidden]) << " avg ---, " << nanoseconds(auAverageDurations[Visible]) << " avg <0> (~" <<
						nanoseconds(static_cast<u64>(((fVisiblePortion * auAverageDurations[Visible]) + ((1.0f - fVisiblePortion) * auAverageDurations[Hidden])) * uRemainingFaces)) << " left)                \r" << std::flush;

				if (auRecentDurationIndices[bIsVisible] & ~0xF) {
					auRecentDurationSums[bIsVisible] -= aRecentDurations[bIsVisible][(auRecentDurationIndices[bIsVisible]) & 0xF];
				}

				prevTime = currTime;
			#endif // DBG_PH_PVF_SVG
		#endif // DBG_PH_PVF
	}

	#if !DBG_PH_PVF_SVG
		std::cout << std::endl;
	#endif // !DBG_PH_PVF_SVG
}

void CPolyhedron::SortFaceIndicesByHeight(std::vector<u32>& rFaceIndices) const {
	if (rFaceIndices.size() != m_Faces.size()) {
		rFaceIndices.resize(m_Faces.size());
	}

	for (u32 i = 0; i < rFaceIndices.size(); ++i) {
		rFaceIndices[i] = i;
	}

	sort(rFaceIndices.begin(), rFaceIndices.end(),
		[this](u32 uFaceIndexA, u32 uFaceIndexB) -> bool {
			return CompareFaceIndices(uFaceIndexA, uFaceIndexB);
		});
}

u32 CPolyhedron::ComputeAlpha(f32 dT, f32 dMinAlpha, f32 dMaxAlpha) {
	return static_cast<u32>(dMinAlpha * (1.0 - dT) + dMaxAlpha * dT);
}

CVector3 CPolyhedron::ComputeNormal(const CVector3& rVector0, const CVector3& rVector1, const CVector3& rVector2, bool bShouldNormalize) {
	const CVector3 crossProduct = (rVector1 - rVector0).Cross(rVector2 - rVector0);

	return bShouldNormalize ? crossProduct.GetUnitVector() : crossProduct;
}

u32 CPolyhedron::ComputeRGB(u32 uColorLevel) {
	switch (uColorLevel % 6) {
	case 0:
		return 0xFF0000;
	case 1:
		return 0xFFFF00;
	case 2:
		return 0x00FF00;
	case 3:
		return 0x00FFFF;
	case 4:
		return 0x0000FF;
	case 5:
		return 0xFF00FF;
	}

	return 0x000000;
}

u32 CPolyhedron::ComputeU32Log(u32 x, u32 uBase) {
	u32 uDivisionCount = 0;

	if (!x) {	
		return 0;
	}

	while (x) {
		x /= uBase;
		++uDivisionCount;
	}

	return --uDivisionCount;
}
