#ifndef __LOG__
#define __LOG__

#include <deque>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include "Defines.h"

namespace NLog {
	class CLog {
		// Structs
		public:
			struct SManipulator {
				// Enums
				public:
					enum EType : u16 {
						invalid		= 0,
						endl		= 1 << 0,
						ends		= 1 << 1,
						flush		= 1 << 2,
						setf		= 1 << 3,
						unsetf		= 1 << 4,
						fill		= 1 << 5,
						precision	= 1 << 6,
						width		= 1 << 7,
						indent		= 1 << 8,
						unindent	= 1 << 9,
						immediate	= endl | ends | flush,
						indentation	= indent | unindent
					};

					union UData {
						// Functions
						public:
							UData(u32 uValue0 = 0);
							UData(u16 uValue0, u16 uValue1);
							UData(u8 uValue0, u8 uValue1, u8 uValue2, u8 uValue3);

						// Variables
						public:
							u32	m_au32	[1];
							u16	m_au16	[2];
							u8	m_au8	[4];
					};

				// Functions
				public:
					SManipulator(EType eType = invalid, u32 uValue0 = 0);
					SManipulator(EType eType, u16 uValue0, u16 uValue1);
					SManipulator(EType eType, u8 uValue0, u8 uValue1, u8 uValue2, u8 uValue3);

				// Variables
				public:
					EType m_eType;
					UData m_Data;
			};

			struct SIndentData {
				// Functions
				public:
					SIndentData(u32 uIndentLevel = 0, u32 uIndentSize = 2, bool bHasPendingIndent = false);
				
				// Variables
				public:
					u32 m_uIndentLevel;
					u32 m_uIndentSize;
					bool m_bHasPendingIndent;
			};

		// Functions
		public:
			CLog();

			SIndentData&		GetIndent			();
			const SIndentData&	GetIndent			()	const;
			u32					GetIndentLevel		()	const;
			u32					GetIndentSize		()	const;
			bool				HasPendingIndent	()	const;

			void				SetIndent			(const SIndentData& rIndentData);
			void				SetIndentLevel		(u32 uIndentLevel);
			void				SetIndentSize		(u32 uIndentSize);

			void				Indent				(u32 uLevelDelta = 1);
			void				Unindent			(u32 uLevelDelta = 1);

			template<typename ..._Types>
			void				PushIndent			(_Types... args);
			void				PopIndent			();

			template<typename _Type>
			CLog& operator<<(const _Type& rT) {
				m_Stream << rT;
				HandleStream();

				return *this;
			}
		private:
			void HandleStream();

		// Variables
		private:
			std::stringstream m_Stream;
			std::vector<SIndentData> m_IndentStack;
			char m_aBuffer[256];
	}; // class CLog

	template<>
	CLog& CLog::operator<<<CLog::SManipulator>(const SManipulator& rManipulator);

	CLog::SManipulator setiosflags(std::ios_base::fmtflags mask);
	CLog::SManipulator resetiosflags(std::ios_base::fmtflags mask);
	CLog::SManipulator setbase(u32 base);
	CLog::SManipulator setfill(u8 c);
	CLog::SManipulator setprecision(u32 n);
	CLog::SManipulator setw(u32 n);
	CLog::SManipulator incindent(s32 sLevelDelta);
	CLog::SManipulator decindent(s32 sLevelDelta);

	extern CLog g_log;

	extern const CLog::SManipulator boolalpha;
	extern const CLog::SManipulator showbase;
	extern const CLog::SManipulator showpoint;
	extern const CLog::SManipulator showpos;
	extern const CLog::SManipulator skipws;
	extern const CLog::SManipulator unitbuf;
	extern const CLog::SManipulator uppercase;
	extern const CLog::SManipulator noboolalpha;
	extern const CLog::SManipulator noshowbase;
	extern const CLog::SManipulator noshowpoint;
	extern const CLog::SManipulator noshowpos;
	extern const CLog::SManipulator noskipws;
	extern const CLog::SManipulator nounitbuf;
	extern const CLog::SManipulator nouppercase;
	extern const CLog::SManipulator oct;
	extern const CLog::SManipulator dec;
	extern const CLog::SManipulator hex;
	extern const CLog::SManipulator fixed;
	extern const CLog::SManipulator scientific;
	extern const CLog::SManipulator internal;
	extern const CLog::SManipulator left;
	extern const CLog::SManipulator right;
	extern const CLog::SManipulator endl;
	extern const CLog::SManipulator ends;
	extern const CLog::SManipulator flush;
	extern const CLog::SManipulator indent;
	extern const CLog::SManipulator unindent;
} // namespace NLog

#endif // __LOG__