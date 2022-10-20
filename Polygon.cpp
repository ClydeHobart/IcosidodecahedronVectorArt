#include <bit>
#include <bitset>
#include <cmath>
#include <deque>
#include <iomanip>
#include <numbers>
#include <map>
#include <set>
#include <sstream>

#include "Polygon.h"

#include "Logging.h"
#include "Svg.h"

#define DBG_PG_RECTIFY			(DBG_PG				&& ON)
#define DBG_PG_UNION			(DBG_PG				&& ON)
#define DBG_PG_RCCV				(DBG_PG				&& ON)
#define DBG_PG_RHL				(DBG_PG				&& ON)
#define DBG_PG_XING_DESCS		(DBG_PG				&& ON)
#define DBG_PG_EDGE_MAPS		(DBG_PG				&& ON)
#define DBG_PG_EDGE_MAPS_PRELIM	(DBG_PG_EDGE_MAPS	&& ON)
#define DBG_PG_EDGE_MAPS_BIDIR	(DBG_PG_EDGE_MAPS	&& ON)
#define DBG_PG_EDGE_MAPS_VERTEX	(DBG_PG_EDGE_MAPS	&& ON)
#define DBG_PG_FIND_LOOPS		(DBG_PG				&& ON)
#define DBG_PG_GHOST_VERTS		(DBG_PG				&& ON)
#define DBG_PG_LOOP_MATCHER		(DBG_PG				&& ON)
#define DBG_PG_FIND_XINGS		(DBG_PG				&& ON)

using namespace NLog;

std::ostream& operator<<(std::ostream& rOStream, const CPolygon::SLoopOriginData& rLoopOriginData) {
	rLoopOriginData.ToOStream(rOStream);

	return rOStream;
}

inline void CPolygon::SLoopOriginData::ToOStream(std::ostream& rOStream) const {
	const u32 uIndentSize = g_log.GetIndentSize();
	const u8 uFill = rOStream.fill(' ');

	rOStream << "Loop origin data:\n" << std::setw(uIndentSize) << "" << "Loop origins:\n";

	for (u32 uLoopIndex = 0; uLoopIndex < m_LoopOrigins.size(); ++uLoopIndex) {
		rOStream << std::setw(2 * uIndentSize) << "" << uLoopIndex << ": " << CPolygon::sm_aOriginStrings[m_LoopOrigins[uLoopIndex]] << std::endl;
	}

	rOStream << std::setw(uIndentSize) << "" << "Origin loops:\n";

	for (u32 uLoopOrigin = EOrigin::Poly1; uLoopOrigin < EOrigin::Count; ++uLoopOrigin) {
		rOStream << std::setw(2 * uIndentSize) << "" << CPolygon::sm_aOriginStrings[uLoopOrigin] << " (" << m_aOriginLoops[uLoopOrigin].size() << " loops):\n";

		for (u32 uLoopIndex : m_aOriginLoops[uLoopOrigin]) {
			rOStream << std::setw(3 * uIndentSize) << "" << uLoopIndex << std::endl;
		}
	}

	rOStream << std::setw(uIndentSize) << "" << "Old Poly Loops:\n";

	for (const std::pair<const u32, std::set<u32>>& rOPLPair : m_OldPolyLoops) {
		rOStream << std::setw(2 * uIndentSize) << "" << CPolygon::sm_aOriginStrings[m_LoopOrigins[rOPLPair.first]] << " loop " << rOPLPair.first << ":\n";

		for (u32 uOldPolyLoopIndex : rOPLPair.second) {
			rOStream << std::setw(3 * uIndentSize) << "" << CPolygon::sm_aOriginStrings[m_LoopOrigins[uOldPolyLoopIndex]] << " loop " << uOldPolyLoopIndex << std::endl;
		}
	}

	rOStream.fill(uFill);
}

CPolygon::SOldPolyLoopData::SOldPolyLoopData() {}

CPolygon::SLoopData::SLoopData() :
	m_pOldPolyLoopData(nullptr) {}

CPolygon::SLoopData::SLoopData(SLoopData&& rrLoopData) :
	m_Vertices(reinterpret_cast<std::vector<CVector2>&&>(rrLoopData.m_Vertices)),
	m_Extrema(rrLoopData.m_Extrema),
	m_pOldPolyLoopData(rrLoopData.m_pOldPolyLoopData) {

	rrLoopData.m_pOldPolyLoopData = nullptr;
}

CPolygon::SLoopData::~SLoopData() {
	delete m_pOldPolyLoopData;
}

void CPolygon::SLoopData::RemoveConsecutiveColinearVertices() {
	std::vector<u32> vertsToRemove;
	std::vector<u32> oldVertIndices;

	if (m_pOldPolyLoopData) {
		oldVertIndices.resize(m_pOldPolyLoopData->m_VertToLoops.size());

		u32 uOVIIndex = 0;

		for (const std::pair<const u32, std::set<u32>>& rVertToLoopsPair : m_pOldPolyLoopData->m_VertToLoops) {
			oldVertIndices[uOVIIndex++] = rVertToLoopsPair.first;
		}
	}

	do {
		if (vertsToRemove.size()) {
			std::vector<CVector2> oldVertices;

			std::swap(m_Vertices, oldVertices);

			for (u32 uVertIndex = 0, uVTRIndex = 0; uVertIndex < oldVertices.size(); ++uVertIndex) {
				if (uVTRIndex < vertsToRemove.size() && uVertIndex == vertsToRemove[uVTRIndex]) {
					++uVTRIndex;
				} else {
					if (m_pOldPolyLoopData && uVertIndex != m_Vertices.size()) {
						oldVertIndices[m_Vertices.size()] = oldVertIndices[uVertIndex];
					}

					m_Vertices.push_back(oldVertices[uVertIndex]);
				}
			}

			if (m_pOldPolyLoopData) {
				oldVertIndices.resize(m_Vertices.size());
			}

			vertsToRemove.clear();
		}

		if (m_Vertices.size() < 3) {
			break;
		}

		u32 uVert1 = 0;
		u32 uVert2 = 1;
		u32 uVert3 = 2;

		do {
			if (CVector2::ComputeOrientation(m_Vertices[uVert1], m_Vertices[uVert2], m_Vertices[uVert3]) & CVector2::Colinear) {
				#if DBG_PG_RCCV
					g_log << "Verts " << uVert1 << ", " << uVert2 << ", and " << uVert3 <<
						" (" << m_Vertices[uVert1] << ", " << m_Vertices[uVert2] << ", and " << m_Vertices[uVert3] << ") are colinear\n";
				#endif // DBG_PG_RCCV

				vertsToRemove.push_back(uVert2);
			}

			uVert1 = uVert2;
			uVert2 = uVert3;
			uVert3 = uVert3 + 1 == m_Vertices.size() ? 0 : uVert3 + 1;
		} while (uVert1);
	} while (vertsToRemove.size());

	if (m_pOldPolyLoopData && oldVertIndices.size() != m_pOldPolyLoopData->m_VertToLoops.size()) {
		std::map<u32, std::set<u32>> oldVertToLoops;
		std::map<u32, std::set<u32>>& rLoopToVert = m_pOldPolyLoopData->m_LoopToVerts;
		std::map<u32, std::set<u32>>& rVertToLoops = m_pOldPolyLoopData->m_VertToLoops;

		std::swap(rVertToLoops, oldVertToLoops);

		for (u32 uNewVertIndex = 0; uNewVertIndex < oldVertIndices.size(); ++uNewVertIndex) {
			const u32 uOldVertIndex = oldVertIndices[uNewVertIndex];

			if (uOldVertIndex != uNewVertIndex) {
				const std::set<u32>& rLoopIDs = oldVertToLoops[uOldVertIndex];

				rVertToLoops[uNewVertIndex] = rLoopIDs;

				for (u32 uLoopID : rLoopIDs) {
					std::set<u32>& rLoopVerts = rLoopToVert[uLoopID];

					rLoopVerts.insert(uNewVertIndex);
					rLoopVerts.erase(uOldVertIndex);
				}

				oldVertToLoops.erase(uOldVertIndex);
			} else {
				rVertToLoops[uNewVertIndex] = oldVertToLoops[uOldVertIndex];
			}
		}

		for (const std::pair<const u32, std::set<u32>>& rOldVertToLoopsPair : oldVertToLoops) {
			for (u32 uLoopID : rOldVertToLoopsPair.second) {
				std::set<u32>& rLoopVerts = rLoopToVert[uLoopID];

				rLoopVerts.erase(rOldVertToLoopsPair.first);

				if (rLoopVerts.empty()) {
					rLoopToVert.erase(uLoopID);
				}
			}
		}
	}
}

CPolygon::CPolygon() {
	Reset();
}

CPolygon::CPolygon(const std::vector<CVector2>& rVertices) :
	m_Vertices(rVertices),
	m_bWereAnyNewLoopsAdded(false)
{
	#if DBG_PG
		g_log << "CPolygon::CPolygon(const std::vector<CVector2>&)" << endl << indent;
	#endif // DBG_PG
	CExtrema& rPolyExtrema = m_Extrema.emplace_back();

	for (const CVector2& rVert : m_Vertices) {
		rPolyExtrema.ReEvaluate(rVert);
	}

	// Index 0 is for the whole polygon, index i is for loop i - 1
	m_Extrema.push_back(rPolyExtrema);

	InitializeLoopTrackers();
	VerifyVerticesAreClockwise();
	RemoveConsecutiveEquivalentVertices();
	RemoveConsecutiveColinearVertices();
	RemoveHiddenLoops();
	Rectify();

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

CPolygon::CPolygon(const std::vector<CVector2>& rVertices, const std::vector<u32>& rLoopBoundaries) :
	m_Vertices(rVertices),
	m_LoopBoundaries(rLoopBoundaries),
	m_bWereAnyNewLoopsAdded(false)
{
	#if DBG_PG
		g_log << "CPolygon::CPolygon(const std::vector<CVector2>&, const std::vector<u32>&)" << endl << indent;
	#endif // DBG_PG

	m_Extrema.reserve(m_LoopBoundaries.size());

	CExtrema& rPolyExtrema = m_Extrema.emplace_back();

	for (u32 uLoopIndex = 0; uLoopIndex < m_LoopBoundaries.size() - 1; ++uLoopIndex) {
		CExtrema& rLoopExtrema = m_Extrema.emplace_back();

		for (u32 uVertIndex = m_LoopBoundaries[uLoopIndex]; uVertIndex < m_LoopBoundaries[uLoopIndex + 1]; ++uVertIndex) {
			rLoopExtrema.ReEvaluate(m_Vertices[uVertIndex]);
		}

		rPolyExtrema.ReEvaluate(rLoopExtrema);
		m_IsLoopClockwise.push_back(IsLoopClockwise(uLoopIndex));
	}

	RemoveConsecutiveEquivalentVertices();
	RemoveConsecutiveColinearVertices();
	RemoveHiddenLoops();
	Rectify();

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

u32 CPolygon::GetLoopSize(u32 uLoopIndex) const {
	clampAssign(uLoopIndex, 0u, GetLoopCount());

	return m_LoopBoundaries[uLoopIndex + 1] - m_LoopBoundaries[uLoopIndex];
}

void CPolygon::Reset() {
	m_Vertices.clear();
	m_Extrema.clear();
	m_Extrema.emplace_back();
	m_LoopBoundaries.clear();
	m_LoopBoundaries.push_back(0);
	m_IsLoopClockwise.clear();
	m_bWereAnyNewLoopsAdded = false;
}

void CPolygon::Translate(const CVector2& rTranslation) {
	for (u32 v = 0; v < m_Vertices.size(); ++v) {
		m_Vertices[v] += rTranslation;
	}

	for (CExtrema& rExtrema : m_Extrema) {
		rExtrema.m_vMin += rTranslation;
		rExtrema.m_vMax += rTranslation;
	}
}

CPolygon& CPolygon::operator|=(CPolygon& rPoly2) {
	#if DBG_PG
		g_log << "CPolygon::operator|=(CPolygon&)\n";
		g_log.Indent();
	#endif // DBG_PG

	#if DBG_PG_UNION
		g_log << "pre-union:\n";
		g_log.Indent();
		g_log << "rPoly1 " << *this << "rPoly2 " << rPoly2;
		g_log.Unindent();
	#endif // DBG_PG_UNION

	if (!rPoly2.GetVertexCount()) {
		#if DBG_PG_UNION
			g_log << "rPoly2 is empty, no loops were added\n";
		#endif // DBG_PG_UNION

		m_bWereAnyNewLoopsAdded = false;

		#if DBG_PG
			g_log.Unindent();
		#endif // DBG_PG

		return *this;
	}

	std::vector<u32> intersectionIndices;

	FindIntersections(*this, rPoly2, intersectionIndices);

	if (intersectionIndices.size()) {
		CPolygon oldThis;

		std::swap(*this, oldThis);
		ComputeUnion(oldThis, rPoly2, intersectionIndices, *this);
		m_bWereAnyNewLoopsAdded = *this != oldThis;
	} else {
		#if DBG_PG_UNION
			g_log << "No intersections found, inserting Poly 2 data into this\nrPoly2.GetLoopCount(): " << rPoly2.GetLoopCount() << "\nrPoly2.GetVertexCount(): " << rPoly2.GetVertexCount() << endl;
		#endif // DBG_PG_UNION

		const u32 uOriginalVertsSize = m_Vertices.size();
		SLoopOriginData loopOriginData;

		loopOriginData.m_LoopOrigins.resize(GetLoopCount() + rPoly2.GetLoopCount());

		for (u32 uLoopIndex1 = 0; uLoopIndex1 < GetLoopCount(); ++uLoopIndex1) {
			loopOriginData.m_LoopOrigins[uLoopIndex1] = Poly1;
			loopOriginData.m_aOriginLoops[Poly1].insert(uLoopIndex1);
		}

		m_Vertices.insert(m_Vertices.end(), rPoly2.m_Vertices.begin(), rPoly2.m_Vertices.end());
		m_Extrema.reserve(m_Extrema.size() + rPoly2.GetLoopCount());
		m_Extrema.front().ReEvaluate(rPoly2.m_Extrema.front());

		for (u32 uLoopIndex2 = 0, uLoopIndexUnion = GetLoopCount(); uLoopIndex2 < rPoly2.GetLoopCount(); ++uLoopIndex2, ++uLoopIndexUnion) {
			m_Extrema.emplace_back(rPoly2.m_Extrema[uLoopIndex2 + 1]);
			m_LoopBoundaries.push_back(rPoly2.m_LoopBoundaries[uLoopIndex2 + 1] + uOriginalVertsSize);
			m_IsLoopClockwise.push_back(rPoly2.m_IsLoopClockwise[uLoopIndex2]);
			loopOriginData.m_LoopOrigins[uLoopIndexUnion] = Poly2;
			loopOriginData.m_aOriginLoops[Poly2].insert(uLoopIndexUnion);
		}

		Rectify(&loopOriginData);
		m_bWereAnyNewLoopsAdded = !RemoveHiddenLoops(&loopOriginData);
	}

	#if DBG_PG_UNION
		g_log << "post-union:\n";
		g_log.Indent();
		g_log << "rPoly1 " << *this;
		g_log.Unindent();
	#endif // DBG_PG_UNION

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG

	return *this;
}

std::ostream& operator<<(std::ostream& rOStream, const CPolygon& rPoly) {
	const std::ios_base::fmtflags flags = rOStream.flags();
	const u32 uIndentSize = g_log.GetIndentSize();
	const u32 uVertCount = rPoly.GetVertexCount();

	rOStream << std::dec << uVertCount << " (0x" << std::hex << uVertCount << ") verts, " << std::dec << rPoly.GetLoopCount() << " (0x" << std::hex << rPoly.GetLoopCount() << ") loops:\n" << std::fixed << std::setprecision(5);

	for (u32 uVertIndex = 0, uLoopIndex = 0; uVertIndex < uVertCount; ++uVertIndex) {
		if (uVertIndex == rPoly.m_LoopBoundaries[uLoopIndex]) {
			rOStream << std::setfill(' ') << std::setw(uIndentSize) << "" << '[' << uLoopIndex << "] (" << (rPoly.m_IsLoopClockwise[uLoopIndex] ? "CW" : "CCW") << ")\n";
			++uLoopIndex;
		}

		rOStream << std::setfill(' ') << std::setw(2 * uIndentSize) << "" << std::setfill('0') << std::setw(4) << uVertIndex << ": " << rPoly.m_Vertices[uVertIndex] << (uVertIndex < uVertCount - 1 ? "," : "") << std::endl;
	}

	rOStream.flags(flags);

	return rOStream;
}

CSvg& operator<<(CSvg& rSvg, const CPolygon& rPoly) {
	if (!rSvg.IsOpen()) {
		return rSvg;
	}

	std::stringstream ss;

	ss << "<path fill=\"" << rSvg.GetFillColor() << "\" stroke=\"" << rSvg.GetStrokeColor() << "\" stroke-width=\"" << rSvg.GetStrokeWidth() << "\" d=\"" << std::fixed;

	if (!rSvg.ShouldCondense()) {
		ss << std::setprecision(5);
	}

	const char* pOptionalSpace = rSvg.GetOptionalSpace();
	const char* pOptionalNewLine = rSvg.GetOptionalNewLine();

	for (u32 l = 0; l < rPoly.GetLoopCount(); ++l) {
		const CVector2& rFirstLoopVert = rPoly.m_Vertices[rPoly.m_LoopBoundaries[l]];
		ss << pOptionalNewLine << 'M' << pOptionalSpace << rFirstLoopVert.x << ' ' << rFirstLoopVert.y;

		for (u32 v = rPoly.m_LoopBoundaries[l] + 1; v < rPoly.m_LoopBoundaries[l + 1]; ++v) {
			ss << pOptionalNewLine << 'L' << pOptionalSpace << rPoly.m_Vertices[v].x << ' ' << rPoly.m_Vertices[v].y;
		}

		ss << pOptionalNewLine << 'L' << pOptionalSpace << rFirstLoopVert.x << ' ' << rFirstLoopVert.y;
		ss << pOptionalNewLine << 'M' << pOptionalSpace << rPoly.m_Vertices.front().x << ' ' << rPoly.m_Vertices.front().y;
	}

	ss << pOptionalNewLine << "\"/>";
	rSvg << ss.str();

	return rSvg;
}

bool operator==(const CPolygon& rPoly1, const CPolygon& rPoly2) {
	// If they have different vertex or loop counts, they're not the same
	if (rPoly1.GetVertexCount() != rPoly2.GetVertexCount() ||
		rPoly1.GetLoopCount() != rPoly2.GetLoopCount() ||
		rPoly1.m_Extrema.front() != rPoly2.m_Extrema.front()) {

		return false;
	}

	const u32 uVertCount = rPoly1.GetVertexCount();
	const u32 uLoopCount = rPoly1.GetLoopCount();

	// If they have different loop properties, they're not the same (assuming they've been rectified)
	for (u32 uLoopIndex = 0; uLoopIndex < uLoopCount; ++uLoopIndex) {
		if (rPoly1.m_LoopBoundaries[uLoopIndex] != rPoly2.m_LoopBoundaries[uLoopIndex] ||
			rPoly1.m_IsLoopClockwise[uLoopIndex] != rPoly2.m_IsLoopClockwise[uLoopIndex] ||
			rPoly1.m_Extrema[uLoopIndex + 1] != rPoly2.m_Extrema[uLoopIndex + 1]) {

			return false;
		}
	}

	// If they're vertices aren't equal, they're not the same. This is the most expensive check, so it's done last
	for (u32 uVertIndex = 0; uVertIndex < uVertCount; ++uVertIndex) {
		if (rPoly1.m_Vertices[uVertIndex] != rPoly2.m_Vertices[uVertIndex]) {
			return false;
		}
	}

	// At this point, the only value that hasn't been checked is m_bWereAnyLoopsAdded, which isn't important.
	return true;
}

CPolygon::Result CPolygon::DoLoopsOverlap(u32 uLoop1, u32 uLoop2) const {
	return static_cast<Result>(
		HasPointInsideLoop((m_Vertices[m_LoopBoundaries[uLoop2]] + m_Vertices[m_LoopBoundaries[uLoop2] + 1]) / 2.0f, uLoop1) << 1 |
		HasPointInsideLoop((m_Vertices[m_LoopBoundaries[uLoop1]] + m_Vertices[m_LoopBoundaries[uLoop1] + 1]) / 2.0f, uLoop2));
}

u32 CPolygon::GetLoopIndex(u32 uVertIndex) const {
	if (uVertIndex >= GetVertexCount()) {
		return GetLoopCount() - 1;
	}

	u32 uLowLoopIndex = 0;
	u32 uHighLoopIndex = GetLoopCount();
	u32 uMiddleLoopIndex = (uLowLoopIndex + uHighLoopIndex) >> 1;
	bool bIsTooLow = uVertIndex < m_LoopBoundaries[uMiddleLoopIndex];
	bool bIsTooHigh = uVertIndex >= m_LoopBoundaries[uMiddleLoopIndex + 1];

	while (bIsTooLow || bIsTooHigh) {
		if (bIsTooLow) {
			uHighLoopIndex = uMiddleLoopIndex;
		} else {
			uLowLoopIndex = uMiddleLoopIndex + 1;
		}

		uMiddleLoopIndex = (uLowLoopIndex + uHighLoopIndex) >> 1;
		bIsTooLow = uVertIndex < m_LoopBoundaries[uMiddleLoopIndex];
		bIsTooHigh = uVertIndex >= m_LoopBoundaries[uMiddleLoopIndex + 1];
	}

	return uMiddleLoopIndex;
}

u32 CPolygon::GetNextIndex(u32 uIndex) const {
	++uIndex;

	for (u32 l = 0; l < m_LoopBoundaries.size(); ++l) {
		if (m_LoopBoundaries[l] == uIndex) {
			return m_LoopBoundaries[l - 1];
		} else if (m_LoopBoundaries[l] > uIndex) {
			break;
		}
	}

	return uIndex;
}

u32 CPolygon::GetPrevIndex(u32 uIndex) const {
	for (u32 l = 0; l < m_LoopBoundaries.size(); ++l) {
		if (m_LoopBoundaries[l] == uIndex) {
			return m_LoopBoundaries[l + 1] - 1;
		} else if (m_LoopBoundaries[l] > uIndex) {
			break;
		}
	}

	return uIndex - 1;
}

bool CPolygon::HasPointInsideLoop(const CVector2& rPoint, u32 uLoopIndex) const {
	if (!m_Extrema[uLoopIndex + 1].ContainsPoint(rPoint)) {
		return false;
	}

	const CVector2 outsidePoint(m_Extrema[uLoopIndex + 1].m_vMin.x - 1.0f, rPoint.y);
	bool bIntersectedOddly = false;

	for (u32 uIndex = m_LoopBoundaries[uLoopIndex], uStopIndex = m_LoopBoundaries[uLoopIndex + 1]; uIndex < uStopIndex; ++uIndex) {
		bIntersectedOddly ^= CVector2::DoLineSegmentsIntersect(
			outsidePoint,
			rPoint,
			m_Vertices[uIndex],
			m_Vertices[uIndex + 1 == uStopIndex ? m_LoopBoundaries[uLoopIndex] : uIndex + 1],
			true);
	}

	return bIntersectedOddly;
}

void CPolygon::InitializeLoopTrackers() {
	m_LoopBoundaries.push_back(0);

	if (m_Vertices.size()) {
		m_LoopBoundaries.push_back(m_Vertices.size());
		m_IsLoopClockwise.push_back(true);
	}
}

void CPolygon::Rectify(SLoopOriginData* pLoopOriginData /* = nullptr */) {
	#if DBG_PG
		g_log << "CPolygon::Rectify(SLoopOriginData*)" << endl;
	#endif // DBG_PG

	// Order vertices such that the first vertex of a loop is the furthest left, then the furthest down
	auto CompareVertexIndicesLambda = [this](u32 uVertIndex1, u32 uVertIndex2) -> bool {
		return CompareVertices(m_Vertices[uVertIndex1], m_Vertices[uVertIndex2]) == std::strong_ordering::less;
	};

	for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
		const u32 uBeginLoopIndex = m_LoopBoundaries[uLoopIndex];
		const u32 uEndLoopIndex = m_LoopBoundaries[uLoopIndex + 1];
		u32 uInitialVertexIndex = uBeginLoopIndex;

		for (u32 uVertexIndex = uBeginLoopIndex; uVertexIndex < uEndLoopIndex; ++uVertexIndex) {
			uInitialVertexIndex = CompareVertexIndicesLambda(uVertexIndex, uInitialVertexIndex) ? uVertexIndex : uInitialVertexIndex;
		}

		if (uInitialVertexIndex != uBeginLoopIndex) {
			const std::vector<CVector2>::iterator uBeginLoopIter = m_Vertices.begin() + uBeginLoopIndex;
			const std::vector<CVector2>::iterator uEndLoopIter = m_Vertices.begin() + uEndLoopIndex;
			std::vector<CVector2> oldLoop(uBeginLoopIter, uEndLoopIter);

			oldLoop.insert(oldLoop.end(), uBeginLoopIter, uEndLoopIter);
			uInitialVertexIndex -= 2 * uBeginLoopIndex;

			for (u32 uVertexIndex = uBeginLoopIndex; uVertexIndex < uEndLoopIndex; ++uVertexIndex) {
				m_Vertices[uVertexIndex] = oldLoop[uVertexIndex + uInitialVertexIndex];
			}
		}
	}

	// Order loops using the vertex ordering of the first vertices of each loop
	auto CompareLoopIndicesLambda = [this, &CompareVertexIndicesLambda](u32 uLoopIndex1, u32 uLoopIndex2) -> bool {
		return CompareVertexIndicesLambda(m_LoopBoundaries[uLoopIndex1], m_LoopBoundaries[uLoopIndex2]);
	};

	std::vector<u32> loopOrder(GetLoopCount());

	for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
		loopOrder[uLoopIndex] = uLoopIndex;
	}

	sort(loopOrder.begin(), loopOrder.end(), CompareLoopIndicesLambda);

	if (pLoopOriginData) {
		SLoopOriginData oldLoopOriginData;

		// Old data from pLoopOriginData->m_aOriginLoops isn't used, but it's cleaner to just do a swap than to clear out those maps
		std::swap(*pLoopOriginData, oldLoopOriginData);
		pLoopOriginData->m_LoopOrigins.resize(oldLoopOriginData.m_LoopOrigins.size());

		std::vector<u32> loopIndices(GetLoopCount());

		for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
			loopIndices[loopOrder[uLoopIndex]] = uLoopIndex;
		}

		for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
			const u32 uOldLoopIndex = loopOrder[uLoopIndex];
			const EOrigin eOrigin = oldLoopOriginData.m_LoopOrigins[uOldLoopIndex];

			pLoopOriginData->m_LoopOrigins[uLoopIndex] = eOrigin;
			pLoopOriginData->m_aOriginLoops[eOrigin].insert(uLoopIndex);

			if (
				eOrigin == EOrigin::Xings ||
				eOrigin == EOrigin::OldP1 ||
				eOrigin == EOrigin::OldP2
			) {
				std::set<u32>& rOldPolyLoops = pLoopOriginData->m_OldPolyLoops[uLoopIndex];

				for (u32 uOldPolyOldLoopIndex : oldLoopOriginData.m_OldPolyLoops[uOldLoopIndex]) {
					rOldPolyLoops.insert(loopIndices[uOldPolyOldLoopIndex]);
				}
			}
		}
	}

	CPolygon oldThis;

	std::swap(*this, oldThis);
	m_Extrema.front() = oldThis.m_Extrema.front();
	m_bWereAnyNewLoopsAdded = oldThis.m_bWereAnyNewLoopsAdded;

	for (u32 uLoopOrderIndex = 0; uLoopOrderIndex < loopOrder.size(); ++uLoopOrderIndex) {
		const u32 uOldLoopIndex = loopOrder[uLoopOrderIndex];

		m_Vertices.insert(m_Vertices.end(),
			oldThis.m_Vertices.begin() + oldThis.m_LoopBoundaries[uOldLoopIndex],
			oldThis.m_Vertices.begin() + oldThis.m_LoopBoundaries[uOldLoopIndex + 1]);
		m_Extrema.emplace_back(oldThis.m_Extrema[uOldLoopIndex + 1]);
		m_LoopBoundaries.push_back(m_Vertices.size());
		m_IsLoopClockwise.push_back(oldThis.m_IsLoopClockwise[uOldLoopIndex]);
	}
}

void CPolygon::RemoveConsecutiveEquivalentVertices() {
	#if DBG_PG
		g_log << "CPolygon::RemoveConsecutiveEquivalentVertices()" <<  endl << indent;
	#endif // DBG_PG

	std::vector<CVector2> oldVerts;
	std::vector<u32> oldLoopBoundaries;

	std::swap(m_Vertices, oldVerts);
	std::swap(m_LoopBoundaries, oldLoopBoundaries);

	for (u32 uCurrVertIndex = 0, uLoopIndex = 0, uLoopBeginIndex, uLoopEndIndex = 0; uCurrVertIndex < oldVerts.size(); ++uCurrVertIndex) {
		if (uCurrVertIndex == uLoopEndIndex) {
			m_LoopBoundaries.push_back(m_Vertices.size());
			uLoopBeginIndex = uLoopEndIndex;
			uLoopEndIndex = oldLoopBoundaries[++uLoopIndex];
		}

		u32 uNextVertIndex = (uCurrVertIndex - uLoopBeginIndex + 1) % (uLoopEndIndex - uLoopBeginIndex) + uLoopBeginIndex;

		if (oldVerts[uCurrVertIndex] != oldVerts[uNextVertIndex]) {
			m_Vertices.push_back(oldVerts[uCurrVertIndex]);
		}
	}

	m_LoopBoundaries.push_back(m_Vertices.size());

	if (m_Vertices.size() < 3) {
		Reset();
	}

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

void CPolygon::RemoveConsecutiveColinearVertices() {
	#if DBG_PG
		g_log << "CPolygon::RemoveConsecutiveColinearVertices()" << endl << indent;
	#endif // DBG_PG

	#if DBG_PG_RCCV
		g_log << "pre-removal: " << *this;
	#endif // DBG_PG_RCCV

	if (m_Vertices.size() < 3) {
		#if DBG_PG
			g_log.Unindent();
		#endif // DBG_PG

		return;
	}

	std::vector<u32> vertsToRemove;

	do {
		if (vertsToRemove.size()) {
			std::vector<CVector2> oldVertices;
			std::vector<u32> oldLoopBoundaries;

			std::swap(m_Vertices, oldVertices);
			std::swap(m_LoopBoundaries, oldLoopBoundaries);

			for (u32 uVert = 0, uLoopIndex = 0, uVTRIndex = 0; uVert < oldVertices.size(); ++uVert) {
				if (uVert == oldLoopBoundaries[uLoopIndex]) {
					m_LoopBoundaries.push_back(m_Vertices.size());
					++uLoopIndex;
				}

				if (uVTRIndex < vertsToRemove.size() && uVert == vertsToRemove[uVTRIndex]) {
					++uVTRIndex;
				} else {
					m_Vertices.push_back(oldVertices[uVert]);
				}
			}

			vertsToRemove.clear();
			m_LoopBoundaries.push_back(m_Vertices.size());
		}

		if (m_Vertices.size() < 3) {
			break;
		}

		for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
			const u32 uInitialVert1 = m_LoopBoundaries[uLoopIndex];

			if (m_LoopBoundaries[uLoopIndex + 1] - uInitialVert1 < 3) {
				continue;
			}

			u32 uVert1 = uInitialVert1;
			u32 uVert2 = uVert1 + 1;
			u32 uVert3 = uVert2 + 1;

			do {
				if (CVector2::ComputeOrientation(m_Vertices[uVert1], m_Vertices[uVert2], m_Vertices[uVert3]) & CVector2::Colinear) {
					#if DBG_PG_RCCV
						g_log << "Verts " << uVert1 << ", " << uVert2 << ", and " << uVert3 <<
							" (" << m_Vertices[uVert1] << ", " << m_Vertices[uVert2] << ", and " << m_Vertices[uVert3] << ") are colinear\n";
					#endif // DBG_PG_RCCV

					vertsToRemove.push_back(uVert2);
				}

				uVert1 = uVert2;
				uVert2 = uVert3;
				uVert3 = uVert3 + 1 == m_LoopBoundaries[uLoopIndex + 1] ? uInitialVert1 : uVert3 + 1;
			} while (uVert1 != uInitialVert1);
		}
	} while (vertsToRemove.size());

	RemoveInvalidLoops();

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

bool CPolygon::RemoveHiddenLoops(const CPolygon::SLoopOriginData* pLoopOriginData /* = nullptr */) {
	#if DBG_PG
		g_log << "CPolygon::RemoveHiddenLoops(SLoopOriginData*)\n" << indent;
	#endif // DBG_PG

	std::map<u32, std::set<u32>> outEdgeMap;
	std::map<u32, std::set<u32>> inEdgeMap;

	auto InsertEdgesLambda = [&outEdgeMap, &inEdgeMap](u32 uOutLoopIndex, u32 uInLoopIndex) -> void {
		outEdgeMap[uOutLoopIndex].insert(uInLoopIndex);
		inEdgeMap[uInLoopIndex].insert(uOutLoopIndex);
	};

	auto IsOriginOldPoly = [](EOrigin eOrigin) -> bool {
		return eOrigin >= EOrigin::OldP1 && eOrigin <= EOrigin::OldP2;
	};

	if (pLoopOriginData) {
		const std::vector<EOrigin>& rLoopOrigins = pLoopOriginData->m_LoopOrigins;
		const std::map<u32, std::set<u32>>& rOldPolyLoops = pLoopOriginData->m_OldPolyLoops;

		// Construct the maps for loops that are contained within other loops (in) or contain other loops (out)
		for (u32 uLoop1 = 0; static_cast<s32>(uLoop1) < static_cast<s32>(GetLoopCount()) - 1; ++uLoop1) {
			for (u32 uLoop2 = uLoop1 + 1; uLoop2 < GetLoopCount(); ++uLoop2) {
				const EOrigin eLoopOrigin1 = rLoopOrigins[uLoop1];
				const EOrigin eLoopOrigin2 = rLoopOrigins[uLoop2];
				bool bShouldDoStandardCheck = true;

				if (IsOriginOldPoly(eLoopOrigin1) || IsOriginOldPoly(eLoopOrigin2)) {
					if (eLoopOrigin1 == EOrigin::Xings && rOldPolyLoops.at(uLoop1).contains(uLoop2)) {
						InsertEdgesLambda(uLoop1, uLoop2);

						bShouldDoStandardCheck = false;
					} else if (eLoopOrigin2 == EOrigin::Xings && rOldPolyLoops.at(uLoop2).contains(uLoop1)) {
						InsertEdgesLambda(uLoop2, uLoop1);

						bShouldDoStandardCheck = false;
					} else if (
						IsOriginOldPoly(eLoopOrigin1) &&
						IsOriginOldPoly(eLoopOrigin2) &&
						eLoopOrigin1 != eLoopOrigin2
					) {
						for (u32 uXingLoop : pLoopOriginData->m_OldPolyLoops.at(uLoop1)) {
							if (pLoopOriginData->m_OldPolyLoops.at(uXingLoop).contains(uLoop2)) {
								bShouldDoStandardCheck = false;

								break;
							}
						}
					}
				}

				if (bShouldDoStandardCheck) {
					switch (DoLoopsOverlap(uLoop1, uLoop2)) {
						case Contained1In2: {
							InsertEdgesLambda(uLoop2, uLoop1);

							break;
						} case Contained2In1: {
							InsertEdgesLambda(uLoop1, uLoop2);

							break;
						} default: {
							break;
						}
					}
				}
			}
		}
	} else {
		// Construct the maps for loops that are contained within other loops (in) or contain other loops (out)
		for (u32 uLoop1 = 0; static_cast<s32>(uLoop1) < static_cast<s32>(GetLoopCount()) - 1; ++uLoop1) {
			for (u32 uLoop2 = uLoop1 + 1; uLoop2 < GetLoopCount(); ++uLoop2) {
				if (
					m_Extrema[uLoop1 + 1].m_vMax.x
					
					#if ROUND
						+ g_kfEpsilon
					#endif // ROUND

					< m_Extrema[uLoop2 + 1].m_vMin.x
				) {
					break;
				}

				switch (DoLoopsOverlap(uLoop1, uLoop2)) {
					case Contained1In2: {
						InsertEdgesLambda(uLoop2, uLoop1);

						break;
					} case Contained2In1: {
						InsertEdgesLambda(uLoop1, uLoop2);

						break;
					} default: {
						break;
					}
				}
			}
		}
	}

	std::set<u32> outerLoops;

	// Trim the graph so that it becomes a collection of trees, where the outer-most loops are the roots
	for (u32 uLoop = 0; uLoop < GetLoopCount(); ++uLoop) {
		if (outEdgeMap.contains(uLoop) && inEdgeMap.contains(uLoop)) {
			for (u32 uInLoop : inEdgeMap[uLoop]) {
				for (u32 uOutLoop : outEdgeMap[uLoop]) {
					// Note that this will leave no empty sets in outEdgeMap, since outEdgeMap[uInLoop] contains uLoop
					outEdgeMap[uInLoop].erase(uOutLoop);
				}
			}
		} else if (!inEdgeMap.contains(uLoop)) {
			outerLoops.insert(uLoop);
			VerifyVerticesAreClockwise(uLoop);
		}
	}

	std::map<u32, std::set<u32>> completeInEdgeMap;

	std::swap(inEdgeMap, completeInEdgeMap);

	for (const std::pair<const u32, std::set<u32>>& outEdgePair : outEdgeMap) {
		for (u32 uOutLoop : outEdgePair.second) {
			inEdgeMap[uOutLoop].insert(outEdgePair.first);
		}
	}

	#if DBG_PG_RHL
		g_log << "pre-removal: " << *this <<
			"Loop count: " << GetLoopCount() << endl;

		if (pLoopOriginData) {
			g_log << *pLoopOriginData;
		}
		g_log << "outEdgeMap and inEdgeMap:\n" << indent;

		for (u32 uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
			g_log << uLoopIndex << ":\n" << indent << "out:\n" << indent;

			if (outEdgeMap.contains(uLoopIndex)) {
				for (u32 uOutLoopIndex : outEdgeMap[uLoopIndex]) {
					g_log << uOutLoopIndex << endl;
				}
			}

			g_log << unindent << "in:\n" << indent;

			if (inEdgeMap.contains(uLoopIndex)) {
				for (u32 uInLoopIndex : inEdgeMap[uLoopIndex]) {
					g_log << uInLoopIndex << endl;
				}
			}

			g_log << decindent(2);
		}

		g_log << unindent << "outerLoops:\n" << indent;

		for (u32 uOuterLoopIndex : outerLoops) {
			g_log << uOuterLoopIndex << endl;
		}

		g_log << unindent;
	#endif // DBG_PG_RHL

	std::set<u32> loopsToRemove;

	enum ELoopType : u8 {
		LT_Poly1	= 0b0001,
		LT_Poly2	= 0b0010,
		LT_Xings	= 0b0011,
		LT_Ghost	= 0b0100,
		LT_OldP1	= 0b1001,
		LT_OldP2	= 0b1010,
		LT_OldPs	= 0b1000,
		LT_Inval	= 0b0000
	};

	enum EState : u8 {
		S_None	= 0b00,
		S_Poly1	= 0b01,
		S_Poly2	= 0b10,
		S_Both	= 0b11,
	};

	#if DBG_PG_RHL
		static const char s_kaLoopTypeStrings[11][6] = {
			"Inval",
			"Poly1",
			"Poly2",
			"Xings",
			"Ghost",
			"     ",
			"     ",
			"     ",
			"OldPs",
			"OldP1",
			"OldP2"
		};

		static const char s_kaStateStrings[4][6] = {
			"None ",
			"Poly1",
			"Poly2",
			"Both "
		};
	#endif // DBG_PG_RHL

	static const ELoopType s_kaLoopTypes[EOrigin::Count] = {
		ELoopType::LT_Poly1,
		ELoopType::LT_Poly2,
		ELoopType::LT_Xings,
		ELoopType::LT_Ghost,
		ELoopType::LT_OldP1,
		ELoopType::LT_OldP2
	};

	auto LoopTypeLambda = [pLoopOriginData](u32 uLoopIndex) -> ELoopType {
		return pLoopOriginData ?
			s_kaLoopTypes[pLoopOriginData->m_LoopOrigins[uLoopIndex]] :
			ELoopType::LT_Poly1;
	};

	auto GetSetLambda = [&outerLoops, &outEdgeMap](u32 uLoopIndex) -> const std::set<u32>& {
		return uLoopIndex == u32_MAX ?
			outerLoops :
			outEdgeMap[uLoopIndex];
	};

	struct SSearchData {
		// Functions
		public:
			SSearchData(
				u32 uLoopIndex = u32_MAX
			) :
				m_uLoopIndex(uLoopIndex),
				m_eState(EState::S_None),
				m_eLoopType(ELoopType::LT_Inval),
				m_bIsLoopClockwise(false),
				m_bHasSetCurrentOutNeighbor(false)
				
				#if DBG_PG_RHL
					, m_bIsInverted(false)
				#endif // DBG_PG_RHL
			{}

		// Variables
		public:
			u32						m_uLoopIndex;
			EState					m_eState;
			ELoopType				m_eLoopType;
			bool					m_bIsLoopClockwise;
			bool					m_bHasSetCurrentOutNeighbor;
			std::set<u32>::iterator	m_CurrentOutNeighbor;

			#if DBG_PG_RHL
				bool				m_bIsInverted;
			#endif // DBG_PG_RHL
	};

	std::vector<SSearchData> searchHistory;
	std::vector<bool> hasLoopBeenExamined(GetLoopCount(), false);
	u32 uDepth = 0;

	searchHistory.emplace_back();

	// From now until the end of the DFS, uDepth == searchHistory.size() - 1

	#if DBG_PG_RHL
		g_log << "Loop DFS:\n" << indent;
	#endif // DBG_PG_RHL

	// DFS through outEdgeMap, finding which loops need to be removed
	while (searchHistory.size()) {
		SSearchData& rCurrSearchData = searchHistory.back();
		const u32 uLoopIndex = rCurrSearchData.m_uLoopIndex;
		const std::set<u32>& outNeighborSet = GetSetLambda(uLoopIndex);

		#if DBG_PG_RHL
			g_log << static_cast<s32>(uLoopIndex) << '\n' << indent << "Status\n" << indent;
		#endif // DBG_PG_RHL

		if (!rCurrSearchData.m_bHasSetCurrentOutNeighbor) {
			rCurrSearchData.m_CurrentOutNeighbor = outNeighborSet.begin();
			rCurrSearchData.m_bHasSetCurrentOutNeighbor = true;

			if (uDepth) {
				if (hasLoopBeenExamined[uLoopIndex]) {
					searchHistory.pop_back();

					#if DBG_PG_RHL
						g_log << "Loop has already been examined, skipping\n" << decindent(3);
					#endif // DBG_PG_RHL

					--uDepth;

					continue;
				}

				hasLoopBeenExamined[uLoopIndex] = true;

				const SSearchData& rPrevSearchData = searchHistory[uDepth - 1];
				u32 ePrevState				= rPrevSearchData.m_eState;
				const ELoopType eLoopType	= rCurrSearchData.m_eLoopType			= LoopTypeLambda(uLoopIndex);
				const bool bIsLoopClockwise	= rCurrSearchData.m_bIsLoopClockwise	= m_IsLoopClockwise[uLoopIndex];

				#if DBG_PG_RHL
					rCurrSearchData.m_bIsInverted = rPrevSearchData.m_bIsInverted;
				#endif // DBG_PG_RHL

				switch (eLoopType) {
					case ELoopType::LT_Poly1 : { // Intentional fall-through
					} case ELoopType::LT_Poly2 : {
						if (inEdgeMap[uLoopIndex].size() > 1) {
							for (u32 uInLoopIndex : inEdgeMap[uLoopIndex]) {
								ePrevState =
									(m_IsLoopClockwise[uInLoopIndex] ? 
										ePrevState | LoopTypeLambda(uInLoopIndex) :
										ePrevState & ~LoopTypeLambda(uInLoopIndex)) &
									EState::S_Both;
							}
						} else if (
							uDepth > 2 &&
							(
								rPrevSearchData.m_eLoopType &
								ELoopType::LT_OldPs
							) &&
							!completeInEdgeMap[uLoopIndex].contains(searchHistory[uDepth - 2].m_uLoopIndex)
						) {
							// Skip this child, make sure parent is revisited
							hasLoopBeenExamined[uLoopIndex] = false;
							hasLoopBeenExamined[rPrevSearchData.m_uLoopIndex] = false;

							#if DBG_PG_RHL
								g_log << "Loop " << uLoopIndex << " isn't contained within grandparent loop " << searchHistory[uDepth - 2].m_uLoopIndex << ", skipping\n" << decindent(3);
							#endif // DBG_PG_RHL

							searchHistory.pop_back();

							--uDepth;

							continue;
						}

						rCurrSearchData.m_eState = static_cast<EState>(
							bIsLoopClockwise ?
								ePrevState | eLoopType :
								ePrevState & ~eLoopType);
						
						if (static_cast<bool>(ePrevState) == static_cast<bool>(rCurrSearchData.m_eState)) {
							#if DBG_PG_RHL
								g_log << "(eLoopType == LT_Poly1 || eLoopType == LT_Poly2) && ((bool) ePrevState) == ((bool) eCurrState), removing " << uLoopIndex << endl;
							#endif // DBG_PG_RHL

							loopsToRemove.insert(uLoopIndex);
						}

						break;
					} case ELoopType::LT_Xings : {
						if (uDepth > 1 && rPrevSearchData.m_eLoopType == ELoopType::LT_Xings && !(bIsLoopClockwise ^ rPrevSearchData.m_bIsLoopClockwise)) {
							#if DBG_PG_RHL
								g_log << "eLoopType == LT_Xings && uDepth > 1 && ePrevLoopType == LT_Xings && !(bIsLoopClockwise ^ bPrevIsLoopClockwise), removing " << (bIsLoopClockwise ? uLoopIndex : rPrevSearchData.m_uLoopIndex) << endl;
							#endif // DBG_PG_RHL

							loopsToRemove.insert(bIsLoopClockwise ? uLoopIndex : rPrevSearchData.m_uLoopIndex);
						} else {
							if (bIsLoopClockwise) {
								if (ePrevState) {
									#if DBG_PG_RHL
										g_log << "eLoopType == LT_Xings && !(uDepth > 1 && ePrevLoopType == LT_Xings && ...) && bIsLoopClockwise && ePrevState, removing " << uLoopIndex << endl;
									#endif // DBG_PG_RHL

									loopsToRemove.insert(uLoopIndex);
								}
							} else {
								if (ePrevState == EState::S_None) {
									#if DBG_PG_RHL
										g_log << "eLoopType == LT_Xings && !(uDepth > 1 && ePrevLoopType == LT_Xings && ...) && !bIsLoopClockwise && ePrevState == S_BothClosed, removing " << uLoopIndex << endl;
									#endif // DBG_PG_RHL

									loopsToRemove.insert(uLoopIndex);
								}
							}
						}

						rCurrSearchData.m_eState = bIsLoopClockwise ? EState::S_Both : EState::S_None;

						break;
					} case ELoopType::LT_Ghost : {
						static const EState aInvertedStates[4] = {
							EState::S_None,
							EState::S_Poly2,
							EState::S_Poly1,
							EState::S_Both
						};

						rCurrSearchData.m_eState = aInvertedStates[ePrevState];

						#if DBG_PG_RHL
							rCurrSearchData.m_bIsInverted = !rPrevSearchData.m_bIsInverted;
						#endif // DBG_PG_RHL

						break;
					} case ELoopType::LT_OldP1 : { // Intentional fall-through
					} case ELoopType::LT_OldP2 : {
						const u8 uMask = static_cast<u8>(eLoopType) & EState::S_Both;

						u32 uYoungestNonXingAncestor = uDepth - 1;

						while (uYoungestNonXingAncestor && searchHistory[uYoungestNonXingAncestor].m_eLoopType == ELoopType::LT_Xings) {
							--uYoungestNonXingAncestor;
						}

						rCurrSearchData.m_eState = (
							static_cast<EState>(
								(
									searchHistory[uYoungestNonXingAncestor].m_eState &
									~uMask
								) | (
									bIsLoopClockwise ?
										uMask :
										EState::S_None
								)
							)
						);
						
						break;
					} default : {
						break;
					}
				}
			}

			#if DBG_PG_RHL
				g_log << "Loop indices:      [" << setfill(' ') << left;

				for (u32 uDepthIter = 0; uDepthIter <= uDepth; ++uDepthIter) {
					g_log <<
						setw(5) <<
						static_cast<s32>(searchHistory[uDepthIter].m_uLoopIndex) <<
						(uDepthIter != uDepth ? ", " : "");
				}

				g_log << "]\nStates:            [";

				for (u32 uDepthIter = 0; uDepthIter <= uDepth; ++uDepthIter) {
					g_log <<
						s_kaStateStrings[searchHistory[uDepthIter].m_eState] <<
						(uDepthIter != uDepth ? ", " : "");
				}

				g_log << "]\nLoop types:        [";

				for (u32 uDepthIter = 0; uDepthIter <= uDepth; ++uDepthIter) {
					g_log <<
						s_kaLoopTypeStrings[searchHistory[uDepthIter].m_eLoopType] <<
						(uDepthIter != uDepth ? ", " : "");
				}

				g_log << "]\nIs loop clockwise: [" << boolalpha;

				for (u32 uDepthIter = 0; uDepthIter <= uDepth; ++uDepthIter) {
					g_log <<
						setw(5) <<
						searchHistory[uDepthIter].m_bIsLoopClockwise <<
						(uDepthIter != uDepth ? ", " : "");
				}

				g_log << "]\nAre inverted:      [";

				for (u32 uDepthIter = 0; uDepthIter <= uDepth; ++uDepthIter) {
					g_log <<
						setw(5) <<
						searchHistory[uDepthIter].m_bIsInverted <<
						(uDepthIter != uDepth ? ", " : "");
				}

				g_log << NLog::resetiosflags(std::ios_base::adjustfield) << noboolalpha << setfill('0') << "]\n";
			#endif // DBG_PG_RHL
		}

		#if DBG_PG_RHL
			g_log << "outNeighborSet: {";

			auto outNeighborIter = outNeighborSet.begin();

			for (u32 uOutNeighbor = 0; uOutNeighbor <= outNeighborSet.size(); ++uOutNeighbor) {
				const bool bIsCurrentOutNeighbor = outNeighborIter == rCurrSearchData.m_CurrentOutNeighbor;
				const bool bIsEnd = uOutNeighbor == outNeighborSet.size();

				g_log <<
					(bIsCurrentOutNeighbor	? ">"		: ""								) <<
					(bIsEnd					? "(end)"	: std::to_string(*outNeighborIter)	) <<
					(bIsCurrentOutNeighbor	? "<"		: ""								) <<
					(!bIsEnd				? ", "		: ""								);
				
				if (!bIsEnd) {
					++outNeighborIter;
				}
			}

			g_log << "}\n" << decindent(2);
		#endif // DBG_PG_RHL

		if (rCurrSearchData.m_CurrentOutNeighbor != outNeighborSet.end()) {
			searchHistory.emplace_back(*(rCurrSearchData.m_CurrentOutNeighbor++));
			++uDepth;

			#if DBG_PG_RHL
				g_log.Indent();
			#endif // DBG_PG_RHL
		} else {
			searchHistory.pop_back();

			#if DBG_PG_RHL
				g_log.Unindent();
			#endif // DBG_PG_RHL

			--uDepth;
		}
	}

	if (loopsToRemove.empty() && !(
			pLoopOriginData && (
				pLoopOriginData->m_aOriginLoops[Ghost].size() ||
				pLoopOriginData->m_aOriginLoops[OldP1].size() ||
				pLoopOriginData->m_aOriginLoops[OldP2].size()
			)
		)
	) {
		#if DBG_PG
			g_log << "Found no loops to remove, returning false" << endl << unindent;
		#endif // DBG_PG

		return false;
	}

	std::set<u32> loopsFromPoly2;
	const bool bAreTherePoly2Loops = pLoopOriginData && pLoopOriginData->m_aOriginLoops[Poly2].size();

	// If we need to pay attention to whether all loops are removed from the other poly,
	// copy the set of all those loops
	if (bAreTherePoly2Loops) {
		const std::set<u32>& rPoly2Loops = pLoopOriginData->m_aOriginLoops[Poly2];

		loopsFromPoly2.insert(rPoly2Loops.begin(), rPoly2Loops.end());
	}

	CPolygon oldThis;

	std::swap(*this, oldThis);

	// Reconstruct the member variables, including only data from valid loops
	for (u32 uLoopIndex = 0; uLoopIndex < oldThis.GetLoopCount(); ++uLoopIndex) {
		if (loopsToRemove.contains(uLoopIndex)) {
			if (bAreTherePoly2Loops) {
				loopsFromPoly2.erase(uLoopIndex);
			}
		} else if (
			!(
				s_kaLoopTypes[
					pLoopOriginData ?
						pLoopOriginData->m_LoopOrigins[uLoopIndex] :
						EOrigin::Poly1
				] & (
					ELoopType::LT_Ghost |
					ELoopType::LT_OldPs
				)
			)
		) {
			m_Vertices.insert(
				m_Vertices.end(),
				oldThis.m_Vertices.begin() + oldThis.m_LoopBoundaries[uLoopIndex],
				oldThis.m_Vertices.begin() + oldThis.m_LoopBoundaries[uLoopIndex + 1]);
			m_Extrema.emplace_back(oldThis.m_Extrema[uLoopIndex + 1]);
			m_Extrema.front().ReEvaluate(m_Extrema.back());
			m_LoopBoundaries.push_back(m_Vertices.size());
			m_IsLoopClockwise.push_back(oldThis.m_IsLoopClockwise[uLoopIndex]);
		}
	}

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG

	return bAreTherePoly2Loops && loopsFromPoly2.empty();
}

void CPolygon::RemoveInvalidLoops() {
	#if DBG_PG
		g_log << "CPolygon::RemoveInvalidLoops()\n" << indent;
	#endif // DBG_PG

	CPolygon oldThis;

	std::swap(*this, oldThis);

	for (u32 uLoop = 0; uLoop < oldThis.m_IsLoopClockwise.size(); ++uLoop) {
		const u32 uStartLoopVert = oldThis.m_LoopBoundaries[uLoop];
		const u32 uStopLoopVert = oldThis.m_LoopBoundaries[uLoop + 1];
		const u32 uLoopSize = uStopLoopVert - uStartLoopVert;
		
		if (uLoopSize > 3 ||
			(uLoopSize == 3 &&
				(CVector2::ComputeOrientation(
					oldThis.m_Vertices[uStartLoopVert],
					oldThis.m_Vertices[uStartLoopVert + 1],
					oldThis.m_Vertices[uStartLoopVert + 2]) ^

					CVector2::Colinear
				)
			)
		) {
			m_Vertices.insert(m_Vertices.end(), oldThis.m_Vertices.begin() + uStartLoopVert, oldThis.m_Vertices.begin() + uStopLoopVert);
			m_Extrema.emplace_back(oldThis.m_Extrema[uLoop + 1]);
			m_Extrema.front().ReEvaluate(m_Extrema.back());
			m_LoopBoundaries.push_back(m_Vertices.size());
			m_IsLoopClockwise.push_back(oldThis.m_IsLoopClockwise[uLoop]);
		}
	}

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

void CPolygon::VerifyVerticesAreClockwise(u32 uLoopIndex /* = u32_MAX */) {
	if (uLoopIndex == u32_MAX) {
		for (uLoopIndex = 0; uLoopIndex < GetLoopCount(); ++uLoopIndex) {
			VerifyVerticesAreClockwise(uLoopIndex);
		}
	} else if (uLoopIndex < GetLoopCount() && !IsLoopClockwise(uLoopIndex)) {
		const u32 uBeginLoopIndex = m_LoopBoundaries[uLoopIndex];
		const u32 uEndLoopIndex = m_LoopBoundaries[uLoopIndex + 1];
		const u32 uEndVertsToSwapIndex = ((uEndLoopIndex - uBeginLoopIndex) >> 1) + uBeginLoopIndex;

		for (u32 uVertIndexA = uBeginLoopIndex, uVertIndexB = uEndLoopIndex - 1; uVertIndexA < uEndVertsToSwapIndex; ++uVertIndexA, --uVertIndexB) {
			std::swap(m_Vertices[uVertIndexA], m_Vertices[uVertIndexB]);
		}
	}
}

bool CPolygon::IsLoopClockwise(u32 uLoopIndex) const {
	const u32 uStartIndex = m_LoopBoundaries[uLoopIndex], uStopIndex = m_LoopBoundaries[uLoopIndex + 1];
	const CVector2* pCurrVert = &m_Vertices[uStopIndex - 1];
	const CVector2* pPrevVert;
	f32 fEdgeAreaSum = 0.0f;

	for (u32 uCurrVertIndex = uStartIndex; uCurrVertIndex < uStopIndex; ++uCurrVertIndex) {
		pPrevVert = pCurrVert;
		pCurrVert = &m_Vertices[uCurrVertIndex];
		fEdgeAreaSum += (pCurrVert->x - pPrevVert->x) * (pCurrVert->y + pPrevVert->y);
	}

	return fEdgeAreaSum >= 0.0f;
}

bool CPolygon::AreLoopsEqual(u32 uLoopIndex1, u32 uLoopIndex2) const {
	return AreLoopsEqual(*this, *this, uLoopIndex1, uLoopIndex2);
}

std::strong_ordering CPolygon::CompareVertices(const CVector2& rVert1, const CVector2& rVert2) {
		typedef std::strong_ordering ordering;

		const float fXDiff = rVert2.x - rVert1.x;
		const float fYDiff = rVert2.y - rVert1.y;

		#if ROUND
			return fXDiff > g_kfEpsilon ?
				ordering::less :
				fXDiff < -g_kfEpsilon ?
					ordering::greater :
					fYDiff > g_kfEpsilon ?
						ordering::less :
						fYDiff < -g_kfEpsilon ?
							ordering::greater :
							ordering::equal;
		#else // ROUND
			return fXDiff > 0.0f ?
				ordering::less :
				fXDiff < 0.0f ?
					ordering::greater :
					fYDiff > 0.0f ?
						ordering::less :
						fYDiff < 0.0f ?
							ordering::greater :
							ordering::equal;
		#endif // ROUND
}

void CPolygon::ComputeUnion(const CPolygon& rPoly1, const CPolygon& rPoly2, const std::vector<u32>& rIntersectionDescriptors, CPolygon& rPolyUnion) {
	#if DBG_PG
		g_log << "CPolygon::ComputeUnion(const CPolygon&, const CPolygon&, const std::vector<u32>&, CPolygon&)" << endl << fixed << setprecision(6) << indent;
	#endif // DBG_PG
	
	std::map<u32, std::map<f32, u32>> preliminaryEdgeMap;	// Map of verts to a map of all the verts along the edge to their original neighbor, with distance as keys
	std::map<u32, std::set<u32>> inEdgeMap;					// Map of verts to the set of all verts which lead into it
	std::map<u32, std::set<u32>> outEdgeMap;				// Map of verts to the set of all verts which lead out of it
	std::map<u32, u32> vertexMappings;						// Map of verts from Poly1 or Poly2 to the corresponding point on Poly1 (the vert with the smallest index, in the case where both come from Poly1)
	std::map<u32, std::set<u32>> inverseVertexMappings;		// Map of values from vertexMappings to the set of keys that map to that value
	std::set<u32> intersectionIDs;
	std::vector<CVector2> intersections;
	const u32 kaPrefices[3] = { Poly1 << 16, Poly2 << 16, Xings << 16 };
	const std::vector<CVector2>* aAllVertices[3] = { &rPoly1.m_Vertices, &rPoly2.m_Vertices, &intersections };
	const CPolygon* aPolies[2] = { &rPoly1, &rPoly2 };

	auto GetVertexForIDLambda = [&aAllVertices](u32 uVertIndex) -> const CVector2& {
		return aAllVertices[uVertIndex >> 16][0][uVertIndex & LS16];
	};

	auto GetTrueVertexLambda = [&vertexMappings](u32 uVert) -> u32 {
		while (vertexMappings.contains(uVert)) {
			uVert = vertexMappings[uVert];
		}

		return uVert;
	};

	for (u32 uPoly = Poly1; uPoly <= Poly2; ++uPoly) {
		const CPolygon& rPoly = *aPolies[uPoly];
		const u32 uPrefix = kaPrefices[uPoly];
		const u32 uVertCount = rPoly.GetVertexCount();

		for (u32 uCurrVertIndex = 0, uPrevVertIndex = 0, uLoopIndex = 0; uCurrVertIndex < uVertCount; ++uCurrVertIndex) {
			if (uCurrVertIndex == rPoly.m_LoopBoundaries[uLoopIndex]) {
				uPrevVertIndex = rPoly.m_LoopBoundaries[++uLoopIndex] - 1;
			}

			preliminaryEdgeMap[uPrefix | uPrevVertIndex][1.0f] = uPrefix | uCurrVertIndex;
			uPrevVertIndex = uCurrVertIndex;
		}

		for (u32 uVertIndexA = 0; uVertIndexA < uVertCount - 1; ++uVertIndexA) {
			for (u32 uVertIndexB = uVertIndexA + 1; uVertIndexB < uVertCount; ++uVertIndexB) {
				if (rPoly.m_Vertices[uVertIndexA] == rPoly.m_Vertices[uVertIndexB]) {
					const u32 uSinkVertex = GetTrueVertexLambda(uPrefix | uVertIndexA);

					vertexMappings[uPrefix | uVertIndexB] = uSinkVertex;
					inverseVertexMappings[uSinkVertex].insert(uPrefix | uVertIndexB);
				}
			}
		}
	}

	#if DBG_PG_XING_DESCS
		g_log << "Intersections:\n" << indent;
	#endif // DBG_PG_XING_DESCS

	for (u32 i = 0; i < rIntersectionDescriptors.size(); ++i) {
		const u32 uXingDesc = rIntersectionDescriptors[i];
		const u32 uVert1 = (uXingDesc & Vert1) >> 2;
		const u32 uVert2 = (uXingDesc & Vert2) >> 17;
		const u32 uXingType = 1 << (uXingDesc & XingType);
		const CVector2* aVertPtrs[4];

		#if DBG_PG_XING_DESCS
			const char* aIntersectionTypes[4] = { "Intersect", "PointQ1OnSegmentP", "PointP1OnSegmentQ", "PointsP1Q1Coincident" };

			g_log << setfill('0') << hex << setw(4) << uVert1 << ", " << setw(4) << uVert2 << ", " << aIntersectionTypes[uXingDesc & XingType] << setfill(' ') << endl << indent;
		#endif // DBG_PG_XING_DESCS

		auto InitializeVertPtrsLambda = [&aVertPtrs, &rPoly1, &rPoly2, uVert1, uVert2]() -> void {
			aVertPtrs[0] = &rPoly1.m_Vertices[uVert1];
			aVertPtrs[1] = &rPoly1.m_Vertices[rPoly1.GetNextIndex(uVert1)];
			aVertPtrs[2] = &rPoly2.m_Vertices[uVert2];
			aVertPtrs[3] = &rPoly2.m_Vertices[rPoly2.GetNextIndex(uVert2)];
		};

		u32 uXing = 0;
		f32 fInterpolant;

		switch (uXingType) {
			case CVector2::Intersect: {
				uXing = kaPrefices[Xings] | intersections.size();
				InitializeVertPtrsLambda();
				fInterpolant = CVector2::ComputeInterpolant(*aVertPtrs[0], *aVertPtrs[1], *aVertPtrs[2], *aVertPtrs[3]);
				preliminaryEdgeMap[kaPrefices[Poly1] | uVert1][fInterpolant] = uXing;
				fInterpolant = CVector2::ComputeInterpolant(*aVertPtrs[2], *aVertPtrs[3], *aVertPtrs[0], *aVertPtrs[1]);
				preliminaryEdgeMap[kaPrefices[Poly2] | uVert2][fInterpolant] = uXing;
				intersections.push_back(CVector2::ComputeIntersection(*aVertPtrs[2], *aVertPtrs[3], *aVertPtrs[0], *aVertPtrs[1], &fInterpolant));

				break;
			} case CVector2::PointQ1OnSegmentP: {
				uXing = GetTrueVertexLambda(kaPrefices[Poly2] | uVert2);
				InitializeVertPtrsLambda();
				fInterpolant = CVector2::ComputeInterpolant(*aVertPtrs[0], *aVertPtrs[1], *aVertPtrs[2]);
				preliminaryEdgeMap[kaPrefices[Poly1] | uVert1][fInterpolant] = uXing;

				#if DBG_PG_XING_DESCS
					g_log << "Point Q1, " << *aVertPtrs[2] << " [" << hex << uVert2 << "], on Segment P, " << *aVertPtrs[0] << " [" << uVert1 << "] - " << *aVertPtrs[1] << ", with interpolant " << fInterpolant << endl;
				#endif // DBG_PG_XING_DESCS

				break;
			} case CVector2::PointP1OnSegmentQ: {
				uXing = GetTrueVertexLambda(kaPrefices[Poly1] | uVert1);
				InitializeVertPtrsLambda();
				fInterpolant = CVector2::ComputeInterpolant(*aVertPtrs[2], *aVertPtrs[3], *aVertPtrs[0]);
				preliminaryEdgeMap[kaPrefices[Poly2] | uVert2][fInterpolant] = uXing;

				#if DBG_PG_XING_DESCS
					g_log << "Point P1, " << *aVertPtrs[0] << " [" << hex << uVert1 << "], on Segment Q, " << *aVertPtrs[2] << " [" << uVert2 << "] - " << *aVertPtrs[3] << ", with interpolant " << fInterpolant << endl;
				#endif // DBG_PG_XING_DESCS

				break;
			} case CVector2::PointsP1Q1Coincident: {
				uXing = GetTrueVertexLambda(kaPrefices[Poly1] | uVert1);

				const u32 uVert2ID = kaPrefices[Poly2] | uVert2;

				vertexMappings[uVert2ID] = uXing;
				inverseVertexMappings[uXing].insert(uVert2ID);

				break;
			}
		}

		#if DBG_PG_XING_DESCS
			g_log.Unindent();
		#endif // DBG_PG_XING_DESCS

		intersectionIDs.insert(uXing);
	}

	#if DBG_PG_XING_DESCS
		g_log.Unindent();
	#endif // DBG_PG_XING_DESCS

	#if DBG_PG_EDGE_MAPS_PRELIM
		g_log << "preliminaryEdgeMap:\n" << fixed << setfill('0') << hex << indent;
	#endif // DBG_PG_EDGE_MAPS_PRELIM

	for (const std::pair<const u32, std::map<f32, u32>>& rEdges : preliminaryEdgeMap) {
		u32 uPrevIndex = GetTrueVertexLambda(rEdges.first);

		#if DBG_PG_EDGE_MAPS_PRELIM
			g_log << setw(8) << uPrevIndex << ":\n" << indent;
		#endif // DBG_PG_EDGE_MAPS_PRELIM

		for (const std::pair<const f32, u32>& rVert : rEdges.second) {
			const u32 uCurrIndex = GetTrueVertexLambda(rVert.second);

			#if DBG_PG_EDGE_MAPS_PRELIM
				g_log << rVert.first << ": " << setw(8) << uCurrIndex << endl;
			#endif // DBG_PG_EDGE_MAPS_PRELIM

			outEdgeMap[uPrevIndex].insert(uCurrIndex);
			inEdgeMap[uCurrIndex].insert(uPrevIndex);
			uPrevIndex = uCurrIndex;
		}

		#if DBG_PG_EDGE_MAPS_PRELIM
			g_log.Unindent();
		#endif // DBG_PG_EDGE_MAPS_PRELIM
	}

	#if DBG_PG_EDGE_MAPS_PRELIM
		g_log << unindent << dec << setfill(' ');
	#endif // DBG_PG_EDGE_MAPS_PRELIM

	#if DBG_PG_EDGE_MAPS_BIDIR
		g_log << "outEdgeMap and inEdgeMap:\n" << setfill('0') << hex << indent;

		for (const std::pair<u32, std::set<u32>>& rEdges : outEdgeMap) {
			g_log << setw(8) << rEdges.first << ":\n" << indent << "out:\n" << indent;

			for (u32 uOutVert : rEdges.second) {
				g_log << setw(8) << uOutVert << endl;
			}

			g_log << unindent << "in:\n" << indent;

			for (u32 uInVert : inEdgeMap[rEdges.first]) {
				g_log << setw(8) << uInVert << endl;
			}

			g_log << decindent(2);
		}

		g_log << unindent << dec << setfill(' ');
	#endif // DBG_PG_EDGE_MAPS_BIDIR

	#if DBG_PG_EDGE_MAPS_VERTEX
		g_log << "vertex map:\n" << setfill('0') << hex << indent;

		for (const std::pair<u32, u32>& rVertexMapping : vertexMappings) {
			g_log << setw(8) << rVertexMapping.first << ": " << setw(8) << GetTrueVertexLambda(rVertexMapping.first) << endl;
		}

		g_log.Unindent();
	#endif // DBG_PG_EDGE_MAPS_VERTEX

	std::vector<u32> pathVertsVector;	// Vector of vert IDs in the current path
	std::map<u32, u32> pathVertsMap;		// Map of vert IDs in the current path to their index within pathVertsVector
	std::vector<SLoopData> loopDatas;
	std::vector<u32> aLoopOrigins[4];
	std::set<u64> removedInvalidEdges;
	std::set<u32> ghostLoops;

	rPolyUnion.Reset();

	#if DBG_PG_FIND_LOOPS
		g_log << "Find loops:\n" << indent;
	#endif // DBG_PG_FIND_LOOPS

	while(!outEdgeMap.empty()) {
		#if DBG_PG_FIND_LOOPS
			g_log << "New loop search\n" << indent;
		#endif // DBG_PG_FIND_LOOPS

		u32 uCurrPoint = outEdgeMap.begin()->first;
		const CVector2* pPrevPoint = &GetVertexForIDLambda(*inEdgeMap[uCurrPoint].begin());

		pathVertsVector.clear();
		pathVertsMap.clear();

		do {
			#if DBG_PG_FIND_LOOPS
				if (pathVertsVector.size() < 20) {
					g_log << hex << setfill('0') << setw(8) << uCurrPoint << setfill(' ') << dec << endl;
				} else if (pathVertsVector.size() == 20) {
					g_log << "...\n";
				}
			#endif // DBG_PG_FIND_LOOPS

			pathVertsMap.emplace(uCurrPoint, pathVertsVector.size());
			pathVertsVector.push_back(uCurrPoint);
			
			const CVector2& rCurrPoint = GetVertexForIDLambda(uCurrPoint);

			auto atan2Lambda = [](const CVector2& rVector2) -> f32 {
				return atan2(rVector2.y, rVector2.x);
			};

			if (outEdgeMap.contains(uCurrPoint)) {
				f32 fAngleToPrevPoint = atan2Lambda(*pPrevPoint - rCurrPoint);
				u32 uBestNextPoint = u32_MAX;
				f32 fBestAngle = 0.0f;

				for (const u32 uNextPoint : outEdgeMap[uCurrPoint]) {
					f32 fAngle = fAngleToPrevPoint - atan2Lambda(GetVertexForIDLambda(uNextPoint) - rCurrPoint);

					fAngle += fAngle < 0.0f ? kfTau : 0.0f;
					fAngle -= fAngle >= kfTau ? kfTau : 0.0f;

					if (uBestNextPoint == u32_MAX || fBestAngle >= fAngle) {
						uBestNextPoint = uNextPoint;
						fBestAngle = fAngle;
					}
				}

				pPrevPoint = &rCurrPoint;
				uCurrPoint = uBestNextPoint;
			} else {
				uCurrPoint = u32_MAX;
			}
		} while (uCurrPoint != u32_MAX && !pathVertsMap.contains(uCurrPoint));

		auto RemoveEdgeLambda = [&outEdgeMap, &inEdgeMap, &removedInvalidEdges](u32 uOutVertID, u32 uInVertID, bool bIsEdgeValid = true) -> void {
			outEdgeMap[uOutVertID].erase(uInVertID);

			if (outEdgeMap[uOutVertID].empty()) {
				outEdgeMap.erase(uOutVertID);
			}

			inEdgeMap[uInVertID].erase(uOutVertID);

			if (inEdgeMap[uInVertID].empty()) {
				inEdgeMap.erase(uInVertID);
			}

			if (!bIsEdgeValid) {
				removedInvalidEdges.insert(pack<u64>(uOutVertID, uInVertID));
			}

			#if DBG_PG_FIND_LOOPS
				g_log << "removing edge: " << hex << setfill('0') << setw(8) << uOutVertID << " -> " << setw(8) << uInVertID << setfill(' ') << dec << (!bIsEdgeValid ? " (invalid)" : "") << endl;
			#endif // DBG_PG_FIND_LOOPS
		};

		if (uCurrPoint == u32_MAX || pathVertsVector.size() - pathVertsMap[uCurrPoint] <= 2) {
			// The loop is invalid, so cache some u32s and remove bad edges
			const u32 uPathSize = pathVertsVector.size();

			if (uCurrPoint == u32_MAX) {
				if (pathVertsVector.size() >= 2) {
					// Remove the last edge
					RemoveEdgeLambda(pathVertsVector[uPathSize - 2], pathVertsVector[uPathSize - 1], false);

					#if DBG_PG_FIND_LOOPS
						g_log << indent << "(no next vert)\n" << decindent(2);
					#endif // DBG_PG_FIND_LOOPS
				}

				#if DBG_PG_FIND_LOOPS
					else {
						g_log << indent << "CPolygon::ComputeUnion(): Error: Started a path with length 1 and no out vertices, despite the vertex being in outEdgeMap\n" << unindent;
					}
				#endif // DBG_PG_FIND_LOOPS
			} else {
				const u32 uLoopSize = pathVertsVector.size() - pathVertsMap[uCurrPoint];

				if (uLoopSize == 1) {
					// This shouldn't happen, but if it does, remove the reflexive edge
					const u32 uVert = pathVertsVector.back();

					#if DBG_PG_FIND_LOOPS
						g_log << indent << "CPolygon::ComputeUnion(): Error: Vertex " << setfill('0') << hex << setw(8) << uVert << dec << setfill(' ') << " had a reflexive edge\n" << unindent;
					#endif // DBG_PG_FIND_LOOPS

					RemoveEdgeLambda(uVert, uVert, false);
				} else if (uLoopSize == 2) {
					// Remove the bi-directional edge
					const u32 uVert1 = pathVertsVector.back();

					pathVertsVector.pop_back();

					const u32 uVert2 = pathVertsVector.back();

					RemoveEdgeLambda(uVert1, uVert2, false);
					RemoveEdgeLambda(uVert2, uVert1, false);

					#if DBG_PG_FIND_LOOPS
						g_log << indent << "(bi-directional edge)\n" << decindent(2);
					#endif // DBG_PG_FIND_LOOPS
				}
			}
		} else {
			// Remove all edges from the loop, storing the loop data
			SLoopData& rLoopData = loopDatas.emplace_back();
			bool bLoopContainsIntersection = false;

			for (u32 uPathVertIndex = pathVertsMap[uCurrPoint]; uPathVertIndex < pathVertsVector.size(); ++uPathVertIndex) {
				const u32 uOutVertID = pathVertsVector[uPathVertIndex];
				const u32 uInVertID = pathVertsVector[uPathVertIndex + 1 == pathVertsVector.size() ? pathVertsMap[uCurrPoint] : uPathVertIndex + 1];

				bLoopContainsIntersection |= intersectionIDs.contains(uOutVertID);
				RemoveEdgeLambda(uOutVertID, uInVertID);
				rLoopData.m_Extrema.ReEvaluate(rLoopData.m_Vertices.emplace_back(GetVertexForIDLambda(uOutVertID)));
			}

			if (bLoopContainsIntersection) {
				rLoopData.m_pOldPolyLoopData = new SOldPolyLoopData();

				for (u32 uPathVertIndex = pathVertsMap[uCurrPoint], uLoopVertIndex = 0;
					uPathVertIndex < pathVertsVector.size();
					++uPathVertIndex, ++uLoopVertIndex) {

					u32 uVertID = pathVertsVector[uPathVertIndex];
					u32 uPoly = uVertID >> 16;

					if (uPoly != EOrigin::Xings) {
						u32 uLoopID = kaPrefices[uPoly] | aPolies[uPoly]->GetLoopIndex(uVertID & u16_MAX);
						
						rLoopData.m_pOldPolyLoopData->m_LoopToVerts[uLoopID].insert(uLoopVertIndex);
						rLoopData.m_pOldPolyLoopData->m_VertToLoops[uLoopVertIndex].insert(uLoopID);

						if (inverseVertexMappings.contains(uVertID)) {
							for (u32 uMappingVertID : inverseVertexMappings[uVertID]) {
								uPoly = uMappingVertID >> 16;
								uLoopID = kaPrefices[uPoly] | aPolies[uPoly]->GetLoopIndex(uMappingVertID & u16_MAX);
								rLoopData.m_pOldPolyLoopData->m_LoopToVerts[uLoopID].insert(uLoopVertIndex);
								rLoopData.m_pOldPolyLoopData->m_VertToLoops[uLoopVertIndex].insert(uLoopID);
							}
						}
					}
				}
			}

			aLoopOrigins[bLoopContainsIntersection ? Xings : uCurrPoint >> 16].push_back(loopDatas.size() - 1);

			#if DBG_PG_FIND_LOOPS
				g_log << "Adding loopDatas[" << (loopDatas.size() - 1) << "], describing a loop from " << sm_aOriginStrings[bLoopContainsIntersection ? Xings : uCurrPoint >> 16] << endl << unindent;
			#endif // DBG_PG_FIND_LOOPS
		}
	}

	#if DBG_PG_FIND_LOOPS
		g_log.Unindent();
	#endif // DBG_PG_FIND_LOOPS

	if (removedInvalidEdges.size()) {
		#if DBG_PG_GHOST_VERTS
			g_log << "Checking for ghost loops:\n" << indent << "Removed invalid edges:\n" << setfill('0') << hex << indent;

			for (u64 uRemovedInvalidEdge : removedInvalidEdges) {
				g_log << std::setw(8) << (uRemovedInvalidEdge >> 32) << " -> " << std::setw(8) << (uRemovedInvalidEdge & u32_MAX) << endl;
			}

			g_log << unindent << dec << setfill(' ') << "Checking polygon loops:\n" << indent;
		#endif // DBG_PG_GHOST_VERTS

		for (u32 uPoly = Poly1; uPoly <= Poly2; ++uPoly) {
			const CPolygon& rPoly = uPoly ? rPoly2 : rPoly1;
			const u32 uPolyPrefix = kaPrefices[uPoly];

			#if DBG_PG_GHOST_VERTS
				g_log << sm_aOriginStrings[uPoly] << ":\n" << indent;
			#endif // DBG_PG_GHOST_VERTS

			for (u32 uLoopIndex = 0; uLoopIndex < rPoly.GetLoopCount(); ++uLoopIndex) {
				SLoopData& rLoopData = loopDatas.emplace_back();
				bool bAllEdgesHaveBeenRemoved = true;

				#if DBG_PG_GHOST_VERTS
					g_log << "Loop " << uLoopIndex << ":\n" << indent;
				#endif // DBG_PG_GHOST_VERTS

				u64 uEdge = GetTrueVertexLambda(uPolyPrefix | (rPoly.m_LoopBoundaries[uLoopIndex + 1] - 1));

				for (u32 uCurrVertIndex = rPoly.m_LoopBoundaries[uLoopIndex];
					bAllEdgesHaveBeenRemoved && uCurrVertIndex < rPoly.m_LoopBoundaries[uLoopIndex + 1];
					++uCurrVertIndex) {

					uEdge = uEdge << 32 | GetTrueVertexLambda(uPolyPrefix | uCurrVertIndex);

					#if DBG_PG_GHOST_VERTS
						const bool bRemovedInvalidEdgesContainsEdge = removedInvalidEdges.contains(uEdge);

						bAllEdgesHaveBeenRemoved &= bRemovedInvalidEdgesContainsEdge;
						g_log << "Is edge (" << hex << setfill('0') << setw(8) <<  (uEdge >> 32) << " -> " << setw(8) << (uEdge & u32_MAX) << setfill(' ') << dec << ") in removedInvalidEdges? " << (bRemovedInvalidEdgesContainsEdge ? "Yes\n" : "No\n");
					#else // DBG_PG_GHOST_VERTS
						bAllEdgesHaveBeenRemoved &= removedInvalidEdges.contains(uEdge);
					#endif // DBG_PG_GHOST_VERTS

					rLoopData.m_Extrema.ReEvaluate(rLoopData.m_Vertices.emplace_back(rPoly.m_Vertices[uCurrVertIndex]));
				}

				if (bAllEdgesHaveBeenRemoved) {
					#if DBG_PG_GHOST_VERTS
						g_log << "Loop " << uLoopIndex << " of Poly " << (uPoly ? 2 : 1) << ", verts [" << rPoly.m_LoopBoundaries[uLoopIndex] << ", " << rPoly.m_LoopBoundaries[uLoopIndex + 1] << "), is now a ghost loop (loopDatas[" << loopDatas.size() - 1 << "])\n";
					#endif // DBG_PG_GHOST_VERTS

					aLoopOrigins[Ghost].push_back(loopDatas.size() - 1);
				} else {
					loopDatas.pop_back();
				}

				#if DBG_PG_GHOST_VERTS
					g_log.Unindent();
				#endif // DBG_PG_GHOST_VERTS
			}

			#if DBG_PG_GHOST_VERTS
				g_log.Unindent();
			#endif // DBG_PG_GHOST_VERTS
		}

		#if DBG_PG_GHOST_VERTS
			g_log.Unindent();
		#endif // DBG_PG_GHOST_VERTS

		// If there are multiple ghost loops, check for mirror images
		if (aLoopOrigins[Ghost].size() > 1) {
			#if DBG_PG_GHOST_VERTS
				g_log << "There are multiple ghost loops; checking for mirrors:\n" << indent;
			#endif // DBG_PG_GHOST_VERTS

			std::vector<u32>& rGhostLoopIndices = aLoopOrigins[Ghost];
			std::vector<bool> validGhostLoops(rGhostLoopIndices.size(), true);

			for (u32 uGLIIndex1 = 0; uGLIIndex1 < rGhostLoopIndices.size() - 1; ++uGLIIndex1) {
				if (!validGhostLoops[uGLIIndex1]) {
					continue;
				}

				#if DBG_PG_GHOST_VERTS
					g_log << "Comparing ghost loop loopDatas[" << rGhostLoopIndices[uGLIIndex1] << "]:\n" << indent;
				#endif // DBG_PG_GHOST_VERTS

				const SLoopData& rGhostLoop1Data = loopDatas[rGhostLoopIndices[uGLIIndex1]];

				for (u32 uGLIIndex2 = uGLIIndex1 + 1; uGLIIndex2 < rGhostLoopIndices.size(); ++uGLIIndex2) {
					#if DBG_PG_GHOST_VERTS
						g_log << "... Against ghost loop loopDatas[" << rGhostLoopIndices[uGLIIndex2] << "]:\n" << indent;
					#endif // DBG_PG_GHOST_VERTS
					const SLoopData& rGhostLoop2Data = loopDatas[rGhostLoopIndices[uGLIIndex2]];

					if (rGhostLoop1Data.m_Vertices.size()	!=	rGhostLoop2Data.m_Vertices.size()	||
						rGhostLoop1Data.m_Extrema			!=	rGhostLoop2Data.m_Extrema			||
						rGhostLoop1Data.m_Vertices[0]		!=	rGhostLoop2Data.m_Vertices[0]		) {

						#if DBG_PG_GHOST_VERTS
							g_log.Unindent();
						#endif // DBG_PG_GHOST_VERTS

						continue;
					}

					validGhostLoops[uGLIIndex2] = false;

					for (u32 uVertIndex1 = 1, uVertIndex2 = rGhostLoop2Data.m_Vertices.size() - 1;
						uVertIndex1 < rGhostLoop1Data.m_Vertices.size() && !validGhostLoops[uGLIIndex2];
						++uVertIndex1, --uVertIndex2) {

						validGhostLoops[uGLIIndex2] = rGhostLoop1Data.m_Vertices[uVertIndex1] != rGhostLoop2Data.m_Vertices[uVertIndex2];
					}

					#if DBG_PG_GHOST_VERTS
						if (!validGhostLoops[uGLIIndex2]) {
							g_log << "Marking ghost loop loopDatas[" << rGhostLoopIndices[uGLIIndex2] << "] to be removed\n";
						}

						g_log.Unindent();
					#endif // DBG_PG_GHOST_VERTS
				}

				#if DBG_PG_GHOST_VERTS
					g_log.Unindent();
				#endif // DBG_PG_GHOST_VERTS
			}

			#if DBG_PG_GHOST_VERTS
				g_log.Unindent();
			#endif // DBG_PG_GHOST_VERTS

			std::vector<u32> oldGhostLoopIndices;

			std::swap(rGhostLoopIndices, oldGhostLoopIndices);

			for (u32 uGLIIndex = 0; uGLIIndex < oldGhostLoopIndices.size(); ++uGLIIndex) {
				if (validGhostLoops[uGLIIndex]) {
					rGhostLoopIndices.push_back(oldGhostLoopIndices[uGLIIndex]);
				}
			}
		}

		#if DBG_PG_GHOST_VERTS
			g_log.Unindent();
		#endif // DBG_PG_GHOST_VERTS
	}

	SLoopOriginData loopOriginData;
	u32 uLoopCount = 0;
	std::set<u32> aOldPolyLoops[2];
	std::map<u32, std::set<u32>> oldPolyLoopToXingLoops;

	for (u32 uLoopOrigin = EOrigin::Poly1; uLoopOrigin <= EOrigin::Ghost; ++uLoopOrigin) {
		rPolyUnion.m_Extrema.reserve(rPolyUnion.m_Extrema.size() + aLoopOrigins[uLoopOrigin].size());
		loopOriginData.m_LoopOrigins.reserve(loopOriginData.m_LoopOrigins.size() + aLoopOrigins[uLoopOrigin].size());

		for (u32 uLoopDataIndex : aLoopOrigins[uLoopOrigin]) {
			SLoopData& rLoopData = loopDatas[uLoopDataIndex];

			rLoopData.RemoveConsecutiveColinearVertices();

			if (rLoopData.m_Vertices.size() >= 3) {
				if (uLoopOrigin == EOrigin::Xings) {
					for (const std::pair<const u32, std::set<u32>>& rLoopToVertsPair : rLoopData.m_pOldPolyLoopData->m_LoopToVerts) {
						oldPolyLoopToXingLoops[rLoopToVertsPair.first].insert(uLoopCount);
						aOldPolyLoops[rLoopToVertsPair.first >> 16].insert(rLoopToVertsPair.first & u16_MAX);
					}
				}

				loopOriginData.m_LoopOrigins.push_back(static_cast<EOrigin>(uLoopOrigin));
				loopOriginData.m_aOriginLoops[uLoopOrigin].insert(uLoopCount);
				rPolyUnion.m_Vertices.insert(rPolyUnion.m_Vertices.end(), rLoopData.m_Vertices.begin(), rLoopData.m_Vertices.end());
				rPolyUnion.m_Extrema.front().ReEvaluate(rPolyUnion.m_Extrema.emplace_back(rLoopData.m_Extrema));
				rPolyUnion.m_LoopBoundaries.push_back(rPolyUnion.m_Vertices.size());
				rPolyUnion.m_IsLoopClockwise.push_back(rPolyUnion.IsLoopClockwise(uLoopCount++));
			}
		}
	}

	for (u32 uPoly = EOrigin::Poly1; uPoly <= EOrigin::Poly2; ++uPoly) {
		const CPolygon& rPoly = *aPolies[uPoly];
		const u32 uPrefix = kaPrefices[uPoly];
		const EOrigin eOrigin = static_cast<EOrigin>(uPoly + EOrigin::OldP1);

		rPolyUnion.m_Extrema.reserve(rPolyUnion.m_Extrema.size() + aOldPolyLoops[uPoly].size());
		loopOriginData.m_LoopOrigins.reserve(loopOriginData.m_LoopOrigins.size() + aLoopOrigins[uPoly].size());

		for (u32 uOldPolyLoopIndex : aOldPolyLoops[uPoly]) {
			loopOriginData.m_LoopOrigins.push_back(eOrigin);
			loopOriginData.m_aOriginLoops[eOrigin].insert(uLoopCount);

			std::set<u32>& rXingLoops = loopOriginData.m_OldPolyLoops[uLoopCount];

			for (u32 uXingLoopIndex : oldPolyLoopToXingLoops[uPrefix | uOldPolyLoopIndex]) {
				loopOriginData.m_OldPolyLoops[uXingLoopIndex].insert(uLoopCount);
				rXingLoops.insert(uXingLoopIndex);
			}

			rPolyUnion.m_Vertices.insert(
				rPolyUnion.m_Vertices.end(),
				rPoly.m_Vertices.begin() + rPoly.m_LoopBoundaries[uOldPolyLoopIndex],
				rPoly.m_Vertices.begin() + rPoly.m_LoopBoundaries[uOldPolyLoopIndex + 1]);
			rPolyUnion.m_Extrema.emplace_back(rPoly.m_Extrema[uOldPolyLoopIndex + 1]);
			rPolyUnion.m_LoopBoundaries.push_back(rPolyUnion.m_Vertices.size());
			rPolyUnion.m_IsLoopClockwise.push_back(rPoly.m_IsLoopClockwise[uOldPolyLoopIndex]);
			++uLoopCount;
		}
	}

	#if DBG_PG_LOOP_MATCHER
		g_log << "pre-rectification:\n" << indent << rPolyUnion << loopOriginData << flush << unindent;
	#endif // DBG_PG_LOOP_MATCHER

	rPolyUnion.Rectify(&loopOriginData);

	#if DBG_PG_LOOP_MATCHER
		g_log << "post-rectification:\n" << indent << rPolyUnion << loopOriginData << flush << unindent;
	#endif // DBG_PG_LOOP_MATCHER

	if (loopOriginData.m_aOriginLoops[Xings].size()) {
		#if DBG_PG_LOOP_MATCHER
			g_log << "Xings loops found, attempt to find matches in Poly1 and Poly2\n" << indent;
		#endif // DBG_PG_LOOP_MATCHER

		struct SPolyData {
			// Functions
			public:
				SPolyData(u32 uLoopCount) : m_uLoopIndex(0), m_uLoopCount(uLoopCount) {}
		
			// Variables
			public:
				std::vector<u32>	m_FakeXingLoops;
				u32					m_uLoopIndex;
				const u32			m_uLoopCount;
		};

		SPolyData aPolyData[2] = { { rPoly1.GetLoopCount() }, { rPoly2.GetLoopCount() } };

		for (const u32 uLoopIndex : loopOriginData.m_aOriginLoops[Xings]) {
			#if DBG_PG_LOOP_MATCHER
				g_log << "Checking loop " << uLoopIndex << " from Xings\n" << indent;
			#endif // DBG_PG_LOOP_MATCHER

			const CVector2& rInitialLoopVert = rPolyUnion.m_Vertices[rPolyUnion.m_LoopBoundaries[uLoopIndex]];
			bool bMatchFound = false;

			for (u32 uPoly = Poly1; uPoly <= Poly2; ++uPoly) {
				#if DBG_PG_LOOP_MATCHER
					const char* pPolyString =  sm_aOriginStrings[uPoly];
					g_log << "Comparing with loops from " << pPolyString << endl << indent;
				#endif // DBG_PG_LOOP_MATCHER

				u32& rLoopIndex = aPolyData[uPoly].m_uLoopIndex;
				const u32& rLoopCount = aPolyData[uPoly].m_uLoopCount;

				if (rLoopIndex == rLoopCount) {
					#if DBG_PG_LOOP_MATCHER
						g_log << "auLoopIndices[" << pPolyString << "] == " << rLoopIndex << " == auLoopCounts[" << pPolyString << "], continuing" << sm_aOriginStrings[uPoly] << endl << unindent;
					#endif // DBG_PG_LOOP_MATCHER

					continue;
				}

				#if DBG_PG_LOOP_MATCHER
					g_log << "Finding loop from " << pPolyString << " to compare against:\n" << indent << "evaluating loopComparison\n";
				#endif // DBG_PG_LOOP_MATCHER

				const CPolygon& rPoly = *aPolies[uPoly];
				std::strong_ordering loopComparison = CompareVertices(rInitialLoopVert, rPoly.m_Vertices[rPoly.m_LoopBoundaries[rLoopIndex]]);

				while (loopComparison == std::strong_ordering::greater && rLoopIndex < rLoopCount) {
					#if DBG_PG_LOOP_MATCHER
						g_log << "loopComparison == std::strong_ordering::greater && rLoopIndex < rLoopCount, incrementing rLoopIndex";
					#endif // DBG_PG_LOOP_MATCHER

					++rLoopIndex;

					if (rLoopIndex < rLoopCount) {
						#if DBG_PG_LOOP_MATCHER
							g_log << " and re-evaluating loopComparison";
						#endif // DBG_PG_LOOP_MATCHER
		
						loopComparison = CompareVertices(rInitialLoopVert, rPoly.m_Vertices[rPoly.m_LoopBoundaries[rLoopIndex]]);
					}

					#if DBG_PG_LOOP_MATCHER
						g_log << endl;
					#endif // DBG_PG_LOOP_MATCHER
				}

				if (rLoopIndex == rLoopCount || loopComparison != std::strong_ordering::equal) {

					#if DBG_PG_LOOP_MATCHER
						g_log << "No more " << pPolyString << " loops or the loops have different initial verts: continuing" << endl << decindent(2);
					#endif // DBG_PG_LOOP_MATCHER

					continue;
				}

				if (AreLoopsEqual(rPolyUnion, rPoly, uLoopIndex, rLoopIndex)) {
					if (bMatchFound) {
						#if DBG_PG_LOOP_MATCHER
							g_log << "Loop " << uLoopIndex << " also matches loop " << rLoopIndex << " from Poly2, leaving as Xings loop" << endl;
						#endif // DBG_PG_LOOP_MATCHER

						aPolyData[Poly1].m_FakeXingLoops.pop_back();
					} else {
						bMatchFound = true;

						#if DBG_PG_LOOP_MATCHER
							g_log << "Loop " << uLoopIndex << " matches loop " << rLoopIndex << " from " << pPolyString << endl;
						#endif // DBG_PG_LOOP_MATCHER

						aPolyData[uPoly].m_FakeXingLoops.push_back(uLoopIndex);
					}
				}

				#if DBG_PG_LOOP_MATCHER
					g_log.Unindent(2);
				#endif // DBG_PG_LOOP_MATCHER
			}

			#if DBG_PG_LOOP_MATCHER
				g_log.Unindent();
			#endif // DBG_PG_LOOP_MATCHER

			if (
				aPolyData[EOrigin::Poly1].m_uLoopIndex == aPolyData[EOrigin::Poly1].m_uLoopCount &&
				aPolyData[EOrigin::Poly2].m_uLoopIndex == aPolyData[EOrigin::Poly2].m_uLoopCount
			) {
				break;
			}
		}

		std::set<u32> oldPolyLoopsToRemove;

		for (EOrigin ePoly = EOrigin::Poly1; ePoly <= EOrigin::Poly2; ++reinterpret_cast<u32&>(ePoly)) {
			for (u32 uFakeXingLoopIndex : aPolyData[ePoly].m_FakeXingLoops) {
				loopOriginData.m_LoopOrigins[uFakeXingLoopIndex] = ePoly;
				loopOriginData.m_aOriginLoops[EOrigin::Xings].erase(uFakeXingLoopIndex);
				loopOriginData.m_aOriginLoops[ePoly].insert(uFakeXingLoopIndex);

				// Aggregate indices of old poly loops to remove
				for (u32 uOldPolyLoopIndex : loopOriginData.m_OldPolyLoops[uFakeXingLoopIndex]) {
					loopOriginData.m_OldPolyLoops[uOldPolyLoopIndex].erase(uFakeXingLoopIndex);

					if (loopOriginData.m_OldPolyLoops[uOldPolyLoopIndex].empty()
						|| rPolyUnion.AreLoopsEqual(uFakeXingLoopIndex, uOldPolyLoopIndex)
					) {
						for (u32 uOtherXingLoopIndex : loopOriginData.m_OldPolyLoops[uOldPolyLoopIndex]) {
							loopOriginData.m_OldPolyLoops[uOtherXingLoopIndex].erase(uOldPolyLoopIndex);
						}

						loopOriginData.m_OldPolyLoops.erase(uOldPolyLoopIndex);
						oldPolyLoopsToRemove.insert(uOldPolyLoopIndex);
					}
				}

				loopOriginData.m_OldPolyLoops.erase(uFakeXingLoopIndex);
			}
		}

		if (oldPolyLoopsToRemove.size()) {
			std::vector<u32> oldIndicesToNewIndices(rPolyUnion.GetLoopCount(), u32_MAX);

			CPolygon oldPolyUnion;

			std::swap(rPolyUnion, oldPolyUnion);
			rPolyUnion.m_Extrema.front() = oldPolyUnion.m_Extrema.front();

			for (u32 uOldLoopIndex = 0, uLoopIndex = 0; uOldLoopIndex < oldPolyUnion.GetLoopCount(); ++uOldLoopIndex) {
				const EOrigin eLoopOrigin = loopOriginData.m_LoopOrigins[uOldLoopIndex];
				
				if (
					eLoopOrigin < EOrigin::OldP1 ||
					eLoopOrigin > EOrigin::OldP2 ||
					!oldPolyLoopsToRemove.contains(uOldLoopIndex)
				) {
					oldIndicesToNewIndices[uOldLoopIndex] = uLoopIndex++;
					rPolyUnion.m_Vertices.insert(
						rPolyUnion.m_Vertices.end(),
						oldPolyUnion.m_Vertices.begin() + oldPolyUnion.m_LoopBoundaries[uOldLoopIndex],
						oldPolyUnion.m_Vertices.begin() + oldPolyUnion.m_LoopBoundaries[uOldLoopIndex + 1]);
					rPolyUnion.m_Extrema.emplace_back(oldPolyUnion.m_Extrema[uOldLoopIndex + 1]);
					rPolyUnion.m_LoopBoundaries.push_back(rPolyUnion.m_Vertices.size());
					rPolyUnion.m_IsLoopClockwise.push_back(oldPolyUnion.m_IsLoopClockwise[uOldLoopIndex]);
				}
			}

			SLoopOriginData oldLoopOriginData;

			std::swap(loopOriginData, oldLoopOriginData);
			loopOriginData.m_LoopOrigins.resize(rPolyUnion.GetLoopCount());

			// Re-populate loopOriginData.m_LoopOrigins and loopOriginData.m_aOriginLoops
			for (u32 uOldLoopIndex = 0; uOldLoopIndex < oldPolyUnion.GetLoopCount(); ++uOldLoopIndex) {
				const u32 uLoopIndex = oldIndicesToNewIndices[uOldLoopIndex];

				if (uLoopIndex != u32_MAX) {
					const EOrigin eLoopOrigin = oldLoopOriginData.m_LoopOrigins[uOldLoopIndex];

					loopOriginData.m_LoopOrigins[uLoopIndex] = eLoopOrigin;
					loopOriginData.m_aOriginLoops[eLoopOrigin].insert(uLoopIndex);
				}
			}

			// Re-populate loopOriginData.m_OldPolyLoops
			for (const std::pair<const u32, std::set<u32>>& rOldPolyLoopPair : oldLoopOriginData.m_OldPolyLoops) {
				std::set<u32>& rNewMappedSet = loopOriginData.m_OldPolyLoops[oldIndicesToNewIndices[rOldPolyLoopPair.first]];

				for (u32 uValueOldLoopIndex : rOldPolyLoopPair.second) {
					rNewMappedSet.insert(oldIndicesToNewIndices[uValueOldLoopIndex]);
				}
			}
		}

		#if DBG_PG_LOOP_MATCHER
			g_log << "After finding matches: " << loopOriginData << unindent;
		#endif // DBG_PG_LOOP_MATCHER
	}

	rPolyUnion.RemoveHiddenLoops(&loopOriginData);

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

void CPolygon::FindIntersections(CPolygon& rPoly1, CPolygon& rPoly2, std::vector<u32>& rIntersections) {
	#if DBG_PG
		g_log << "CPolygon::FindIntersections(CPolygon&,CPolygon&,std::vector<u32>&)\n" << indent;
	#endif // DBG_PG

	const u64 uVertsSize1 = rPoly1.m_Vertices.size(), uVertsSize2 = rPoly2.m_Vertices.size();

	rIntersections.clear();
	
	if (!rPoly1.m_Extrema.front().OverlapsWithExtrema(rPoly2.m_Extrema.front())) {
		#if DBG_PG
			g_log.Unindent();
		#endif // DBG_PG

		return;
	}

	const bool bIsVertsSize1Invalid = uVertsSize1 < 3 || uVertsSize1 > 1 << 15;
	const bool bIsVertsSize2Invalid = uVertsSize2 < 3 || uVertsSize2 > 1 << 15;

	if (bIsVertsSize1Invalid || bIsVertsSize2Invalid) {
		if (bIsVertsSize1Invalid) {
			rPoly1.Reset();
		}

		if (bIsVertsSize2Invalid) {
			rPoly2.Reset();
		}

		#if DBG_PG
			g_log.Unindent();
		#endif // DBG_PG

		return;
	}

	for (u32 uLoopIndex1 = 0; uLoopIndex1 < rPoly1.GetLoopCount(); ++uLoopIndex1) {
		#if DBG_PG_FIND_XINGS
			g_log << "Poly1 loop " << uLoopIndex1 << ":\n" << indent;
		#endif // DBG_PG_FIND_XINGS

		const CExtrema& rLoopExtrema1 = rPoly1.m_Extrema[uLoopIndex1 + 1];

		if (!rLoopExtrema1.OverlapsWithExtrema(rPoly2.m_Extrema.front())) {
			#if DBG_PG_FIND_XINGS
				g_log << "Loop doesn't overlap with Poly2, continuing\n" << unindent;
			#endif // DBG_PG_FIND_XINGS

			continue;
		}

		const u32 uEndLoopIndex1 = rPoly1.m_LoopBoundaries[uLoopIndex1 + 1];
		const CVector2* pCurrVert1 = nullptr;
		const CVector2* pPrevVert1 = &rPoly1.m_Vertices[uEndLoopIndex1 - 1];

		for (u32 uCurrVertIndex1 = rPoly1.m_LoopBoundaries[uLoopIndex1], uPrevVertIndex1 = uEndLoopIndex1 - 1;
			uCurrVertIndex1 < uEndLoopIndex1;
			uPrevVertIndex1 = uCurrVertIndex1, pPrevVert1 = pCurrVert1, ++uCurrVertIndex1) {

			#if DBG_PG_FIND_XINGS
				g_log << "Vert " << uCurrVertIndex1 << ":\n" << indent;
			#endif // DBG_PG_FIND_XINGS

			pCurrVert1 = &rPoly1.m_Vertices[uCurrVertIndex1];

			for (u32 uLoopIndex2 = 0; uLoopIndex2 < rPoly2.GetLoopCount(); ++uLoopIndex2) {
				#if DBG_PG_FIND_XINGS
					g_log << "Poly2 loop " << uLoopIndex2 << ":\n" << indent;
				#endif // DBG_PG_FIND_XINGS

				if (!rLoopExtrema1.OverlapsWithExtrema(rPoly2.m_Extrema[uLoopIndex2 + 1])) {
					#if DBG_PG_FIND_XINGS
						g_log << "Loops don't overlap, continuing\n" << unindent;
					#endif // DBG_PG_FIND_XINGS

					continue;
				}

				const u32 uEndLoopIndex2 = rPoly2.m_LoopBoundaries[uLoopIndex2 + 1];
				const CVector2* pCurrVert2 = nullptr;
				const CVector2* pPrevVert2 = &rPoly2.m_Vertices[uEndLoopIndex2 - 1];

				for (u32 uCurrVertIndex2 = rPoly2.m_LoopBoundaries[uLoopIndex2], uPrevVertIndex2 = uEndLoopIndex2 - 1;
					uCurrVertIndex2 < uEndLoopIndex2;
					uPrevVertIndex2 = uCurrVertIndex2, pPrevVert2 = pCurrVert2, ++uCurrVertIndex2) {
			
					pCurrVert2 = &rPoly2.m_Vertices[uCurrVertIndex2];

					#if DBG_PG_FIND_XINGS
						g_log << setfill('0') << hex <<
							"P1: " << setw(4) << uPrevVertIndex1 <<
							", P2: " << setw(4) << uCurrVertIndex1 <<
							", Q1: " << setw(4) << uPrevVertIndex2 <<
							", Q2: " << setw(4) << uCurrVertIndex2 <<
							dec << setfill(' ') << endl;
					#endif // DBG_PG_FIND_XINGS

					if (const u32 uResult = CVector2::DoLineSegmentsIntersect(*pPrevVert1, *pCurrVert1, *pPrevVert2, *pCurrVert2)) {
						const u32 uVertIDs = uPrevVertIndex2 << 17 | uPrevVertIndex1 << 2;

						if (uResult & CVector2::PointsP1Q1Coincident) {
								rIntersections.push_back(uVertIDs | std::countr_zero(static_cast<u32>(CVector2::PointsP1Q1Coincident)));
						} else if (uResult & CVector2::Intersect) {
								rIntersections.push_back(uVertIDs | std::countr_zero(static_cast<u32>(CVector2::Intersect)));
						} else {
							if (uResult & CVector2::PointQ1OnSegmentP) {
								rIntersections.push_back(uVertIDs | std::countr_zero(static_cast<u32>(CVector2::PointQ1OnSegmentP)));
							}

							if (uResult & CVector2::PointP1OnSegmentQ) {
								rIntersections.push_back(uVertIDs | std::countr_zero(static_cast<u32>(CVector2::PointP1OnSegmentQ)));
							}
						}
					}
				}

				#if DBG_PG_FIND_XINGS
					g_log.Unindent();
				#endif // DBG_PG_FIND_XINGS
			}

			#if DBG_PG_FIND_XINGS
				g_log.Unindent();
			#endif // DBG_PG_FIND_XINGS
		}

		#if DBG_PG_FIND_XINGS
			g_log.Unindent();
		#endif // DBG_PG_FIND_XINGS
	}

	#if DBG_PG
		g_log.Unindent();
	#endif // DBG_PG
}

bool CPolygon::AreLoopsEqual(const CPolygon& rPoly1, const CPolygon& rPoly2, u32 uLoopIndex1, u32 uLoopIndex2) {
	if (
		(rPoly1.GetLoopSize(uLoopIndex1)		!= rPoly2.GetLoopSize(uLoopIndex2))			||
		(rPoly1.m_IsLoopClockwise[uLoopIndex1]	!= rPoly2.m_IsLoopClockwise[uLoopIndex2])	||
		(rPoly1.m_Extrema[uLoopIndex1 + 1]		!= rPoly2.m_Extrema[uLoopIndex2 + 1])
	) {
		return false;
	}

	for (
		u32 uVertIndex1		= rPoly1.m_LoopBoundaries[uLoopIndex1],
			uVertIndex2		= rPoly2.m_LoopBoundaries[uLoopIndex2],
			uVertIndexEnd	= rPoly1.m_LoopBoundaries[uLoopIndex1 + 1];
		uVertIndex1 < uVertIndexEnd;
		++uVertIndex1, ++uVertIndex2
	) {
		if (rPoly1.m_Vertices[uVertIndex1] != rPoly2.m_Vertices[uVertIndex2]) {
			return false;
		}
	}

	return true;
}

const	char*	CPolygon::sm_aOriginStrings[EOrigin::Count]	= { "Poly1", "Poly2", "Xings", "Ghost", "OldP1", "OldP2" };
const	f32		CPolygon::kfTau								= 2 * std::numbers::pi_v<f32>;