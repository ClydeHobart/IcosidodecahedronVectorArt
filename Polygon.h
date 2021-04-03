#ifndef __POLYGON__
#define __POLYGON__

#include <compare>
#include <map>
#include <set>
#include <vector>

#include "Defines.h"
#include "Extrema.h"
#include "Vector2.h"

class CSvg;

class CPolygon {
	// Enums
	public:
		enum Result {
			Disjoint		= 0b00,
			Contained1In2	= 0b01,
			Contained2In1	= 0b10,
			Invalid			= 0b11
		};
	private:
		enum EMasks : u32 {
			XingType	= 0x00000003,
			Vert1		= 0x0001FFFC,
			Vert2		= 0xFFFE0000,
			LS16		= 0x0000FFFF
		};
		enum EOrigin : u32 {
			Poly1	= 0,
			Poly2	= 1,
			Xings	= 2,
			Ghost	= 3,
			OldP1	= 4,
			OldP2	= 5,
			Count	= 6
		};

	// Structs
	public:
		struct SLoopOriginData {
			// Functions
			public:
				friend std::ostream& operator<<(std::ostream& rOStream, const SLoopOriginData& rLoopOriginData);
			private:
				inline void ToOStream(std::ostream& rOStream) const;

			// Variables
			public:
				// std::map<u32, EOrigin>			m_LoopOrigins;
				std::vector<EOrigin>			m_LoopOrigins;
				std::set<u32>					m_aOriginLoops[EOrigin::Count];
				std::map<u32, std::set<u32>>	m_OldPolyLoops;
		};
	private:
		struct SOldPolyLoopData {
			// Functions
			public:
				SOldPolyLoopData();

			// Variables
			public:
				std::map<u32, std::set<u32>>	m_LoopToVerts;	// keys: Poly Loop ID;			values: Xing Loop Vert Indices
				std::map<u32, std::set<u32>>	m_VertToLoops;	// keys: Xing Loop Vert Index;	values: Poly Loop IDs
		};

		struct SLoopData {
			// Functions
			public:
				SLoopData();
				SLoopData(SLoopData&& rrLoopData);
				~SLoopData();

				void RemoveConsecutiveColinearVertices();

			// Variables
			public:
				std::vector<CVector2> m_Vertices;
				CExtrema m_Extrema;
				SOldPolyLoopData* m_pOldPolyLoopData;
		};

	// Functions
	public:
		CPolygon();
		CPolygon(const std::vector<CVector2>& rVertices);
		CPolygon(const std::vector<CVector2>& rVertices, const std::vector<u32>& rLoopBoundaries);

				u32						GetLoopCount			()								const	{ return m_IsLoopClockwise.size(); }
				u32						GetVertexCount			()								const	{ return m_Vertices.size(); }
		const	std::vector<CVector2>&	GetVertices				()								const	{ return m_Vertices; }
		const	CExtrema&				GetExtrema				(u32 uExtremaIndex)				const	{ return m_Extrema[uExtremaIndex < m_Extrema.size() ? uExtremaIndex : 0]; }
				u32						GetLoopSize				(u32 uLoopIndex)				const;
				bool					WereAnyNewLoopsAdded	()								const	{ return m_bWereAnyNewLoopsAdded; }
				void					Reset					();
				void					Translate				(const CVector2& rTranslation);

		CPolygon&	operator|=(CPolygon& rPolygon);

		friend std::ostream& operator<<(std::ostream& rOStream, const CPolygon& rPoly);
		friend CSvg& operator<<(CSvg& rSvg, const CPolygon& rPoly);
		friend bool operator==(const CPolygon& rPoly1, const CPolygon& rPoly2);
	private:
		Result	DoLoopsOverlap						(u32 uLoop1, u32 uLoop2)					const;
		u32		GetLoopIndex						(u32 uIndex)								const;
		u32		GetNextIndex						(u32 uIndex)								const;
		u32		GetPrevIndex						(u32 uIndex)								const;
		bool	HasPointInsideLoop					(const CVector2& rPoint, u32 uLoopIndex)	const;
		void	InitializeLoopTrackers				();
		void	Rectify								(SLoopOriginData* pLoopOriginData = nullptr);
		void	RemoveConsecutiveEquivalentVertices	();
		void	RemoveConsecutiveColinearVertices	();
		bool	RemoveHiddenLoops					(const SLoopOriginData* pLoopOriginData = nullptr);
		void	RemoveInvalidLoops					();
		void	VerifyVerticesAreClockwise			(u32 uLoopIndex = u32_MAX);
		bool	IsLoopClockwise						(u32 uLoopIndex)							const;
		bool	AreLoopsEqual						(u32 uLoopIndex1, u32 uLoopIndex2)			const;

		static	std::strong_ordering	CompareVertices		(const CVector2& rVert1, const CVector2& rVert2);
		static	void					ComputeUnion		(const CPolygon& rPoly1, const CPolygon& rPoly2, const std::vector<u32>& rIntersectionIndices, CPolygon& rPolyUnion);
		static	void					FindIntersections	(CPolygon& rPoly1, CPolygon& rPoly2, std::vector<u32>& rIntersections);
		static	bool					AreLoopsEqual		(const CPolygon& rPoly1, const CPolygon& rPoly2, u32 uLoopIndex1, u32 uLoopIndex2);

	// Variables
	private:
		std::vector<CVector2>	m_Vertices;
		std::vector<CExtrema>	m_Extrema;
		std::vector<u32>		m_LoopBoundaries;
		std::vector<bool>		m_IsLoopClockwise;
		bool					m_bWereAnyNewLoopsAdded;

		static const char* sm_aOriginStrings[EOrigin::Count];
	// Constants
	private:
		static const f32 kfTau;
};

#endif // __POLYGON__