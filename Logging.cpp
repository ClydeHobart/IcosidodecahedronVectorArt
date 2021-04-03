#include "Logging.h"

#include <iostream>
#include <iomanip>
#include <cstring>

namespace NLog {
	CLog::SManipulator::UData::UData(u32 uValue0 /* = 0 */) {
		m_au32[0] = uValue0;
	}

	CLog::SManipulator::UData::UData(u16 uValue0, u16 uValue1) {
		m_au16[0] = uValue0;
		m_au16[1] = uValue1;
	}

	CLog::SManipulator::UData::UData(u8 uValue0, u8 uValue1, u8 uValue2, u8 uValue3) {
		m_au8[0] = uValue0;
		m_au8[1] = uValue1;
		m_au8[2] = uValue2;
		m_au8[3] = uValue3;
	}

	CLog::SManipulator::SManipulator(EType eType /* = invalid */, u32 uValue0 /* = 0 */) :
		m_eType(eType),
		m_Data(uValue0) {}

	CLog::SManipulator::SManipulator(EType eType, u16 uValue0, u16 uValue1) :
		m_eType(eType),
		m_Data(uValue0, uValue1) {}

	CLog::SManipulator::SManipulator(EType eType, u8 uValue0, u8 uValue1, u8 uValue2, u8 uValue3) :
		m_eType(eType),
		m_Data(uValue0, uValue1, uValue2, uValue3) {}

	CLog::SIndentData::SIndentData(u32 uIndentLevel /* = 0 */, u32 uIndentSize /* = 2 */, bool bHasPendingIndent /* = false */) :
		m_uIndentLevel(uIndentLevel),
		m_uIndentSize(uIndentSize),
		m_bHasPendingIndent(bHasPendingIndent) {}

	CLog::CLog() :
		m_IndentStack({ { 0, 2 } }) {}

	void CLog::Indent(u32 uLevelDelta /* = 1 */) {
		GetIndent().m_uIndentLevel += uLevelDelta;
	}

	void CLog::Unindent(u32 uLevelDelta /* = 1 */) {
		GetIndent().m_uIndentLevel -= std::min(uLevelDelta, GetIndent().m_uIndentLevel);
	}

	CLog::SIndentData& CLog::GetIndent() {
		return m_IndentStack.back();
	}

	const CLog::SIndentData& CLog::GetIndent() const {
		return m_IndentStack.back();
	}

	u32 CLog::GetIndentLevel() const {
		return GetIndent().m_uIndentLevel;
	}

	u32 CLog::GetIndentSize() const {
		return GetIndent().m_uIndentSize;
	}

	bool CLog::HasPendingIndent() const {
		return GetIndent().m_bHasPendingIndent;
	}

	template<typename ..._Types>
	void CLog::PushIndent(_Types... args) {
		m_IndentStack.emplace_back(args...);
	}

	void CLog::PopIndent() {
		if (m_IndentStack.size() > 1) {
			m_IndentStack.pop_back();
		}
	}

	void CLog::SetIndent(const SIndentData& rIndentData) {
		GetIndent() = rIndentData;
	}

	void CLog::SetIndentLevel(u32 uIndentLevel) {
		GetIndent().m_uIndentLevel = uIndentLevel;
	}

	void CLog::SetIndentSize(u32 uIndentSize) {
		GetIndent().m_uIndentSize = uIndentSize;
	}

	template<>
	CLog& CLog::operator<<<CLog::SManipulator>(const SManipulator& rManipulator) {
		const SManipulator::EType& reType = rManipulator.m_eType;

		switch (reType) {
			case SManipulator::EType::endl: {
				std::cout.put('\n');
				std::cout.flush();
				GetIndent().m_bHasPendingIndent = true;

				break;
			} case SManipulator::EType::ends: {
				std::cout.put('\x00');

				break;
			} case SManipulator::EType::flush: {
				std::cout.flush();

				break;
			} case SManipulator::EType::setf: {
				m_Stream.setf(static_cast<std::ios_base::fmtflags>(rManipulator.m_Data.m_au16[0]), static_cast<std::ios_base::fmtflags>(rManipulator.m_Data.m_au16[1]));

				break;
			} case SManipulator::EType::unsetf: {
				m_Stream.unsetf(static_cast<std::ios_base::fmtflags>(rManipulator.m_Data.m_au32[0]));

				break;
			} case SManipulator::EType::fill: {
				m_Stream.fill(rManipulator.m_Data.m_au32[0]);

				break;
			} case SManipulator::EType::precision: {
				m_Stream.precision(rManipulator.m_Data.m_au32[0]);

				break;
			} case SManipulator::EType::width: {
				m_Stream.width(rManipulator.m_Data.m_au32[0]);

				break;
			} case SManipulator::EType::indent: {
				Indent(rManipulator.m_Data.m_au32[0]);

				break;
			} case SManipulator::EType::unindent: {
				Unindent(rManipulator.m_Data.m_au32[0]);

				break;
			} default: {
				// invalid
				break;
			}
		}

		return *this;
	}

	void CLog::HandleStream() {
		while (m_Stream.peek() != EOF) {
			if (m_Stream.peek() == '\n') {
				m_Stream.ignore();
				std::cout.put('\n');
				GetIndent().m_bHasPendingIndent = true;
			} else {
				if (GetIndent().m_bHasPendingIndent) {
					const u8 uFill = std::cout.fill(' ');

					std::cout << std::setw(GetIndentLevel() * GetIndentSize()) << "";
					std::cout.fill(uFill);
					GetIndent().m_bHasPendingIndent = false;
				}

				m_Stream.get(m_aBuffer, sizeof(m_aBuffer));
				std::cout << m_aBuffer;
			}
		}

		m_Stream.clear();
	}

	CLog::SManipulator setiosflags(std::ios_base::fmtflags mask) {
		return CLog::SManipulator(CLog::SManipulator::EType::setf, mask);
	}

	CLog::SManipulator resetiosflags(std::ios_base::fmtflags mask) {
		return CLog::SManipulator(CLog::SManipulator::EType::unsetf, mask);
	}

	CLog::SManipulator setbase(u32 base) {
		switch (base) {
			case 8: {
				return oct;
			} case 10: {
				return dec;
			} case 16: {
				return hex;
			} default: {
				return CLog::SManipulator(CLog::SManipulator::EType::unsetf, std::ios_base::basefield);
			}
		}
	}

	CLog::SManipulator setfill(u8 c) {
		return CLog::SManipulator(CLog::SManipulator::EType::fill, c);
	}

	CLog::SManipulator setprecision(u32 n) {
		return CLog::SManipulator(CLog::SManipulator::EType::precision, n);
	}

	CLog::SManipulator setw(u32 n) {
		return CLog::SManipulator(CLog::SManipulator::EType::width, n);
	}

	CLog::SManipulator incindent(s32 sLevelDelta) {
		if (sLevelDelta >= 0) {
			return CLog::SManipulator(CLog::SManipulator::EType::indent, sLevelDelta);
		} else {
			return CLog::SManipulator(CLog::SManipulator::EType::unindent, -sLevelDelta);
		}
	}

	CLog::SManipulator decindent(s32 sLevelDelta) {
		if (sLevelDelta >= 0) {
			return CLog::SManipulator(CLog::SManipulator::EType::unindent, sLevelDelta);
		} else {
			return CLog::SManipulator(CLog::SManipulator::EType::indent, -sLevelDelta);
		}
	}

	CLog g_log;
	const CLog::SManipulator boolalpha		(CLog::SManipulator::EType::setf,		std::ios_base::boolalpha,	std::ios_base::boolalpha);
	const CLog::SManipulator showbase		(CLog::SManipulator::EType::setf,		std::ios_base::showbase,	std::ios_base::showbase);
	const CLog::SManipulator showpoint		(CLog::SManipulator::EType::setf,		std::ios_base::showpoint,	std::ios_base::showpoint);
	const CLog::SManipulator showpos		(CLog::SManipulator::EType::setf,		std::ios_base::showpos,		std::ios_base::showpos);
	const CLog::SManipulator skipws			(CLog::SManipulator::EType::setf,		std::ios_base::skipws,		std::ios_base::skipws);
	const CLog::SManipulator unitbuf		(CLog::SManipulator::EType::setf,		std::ios_base::unitbuf,		std::ios_base::unitbuf);
	const CLog::SManipulator uppercase		(CLog::SManipulator::EType::setf,		std::ios_base::uppercase,	std::ios_base::uppercase);
	const CLog::SManipulator noboolalpha	(CLog::SManipulator::EType::unsetf,		std::ios_base::boolalpha);
	const CLog::SManipulator noshowbase		(CLog::SManipulator::EType::unsetf,		std::ios_base::showbase);
	const CLog::SManipulator noshowpoint	(CLog::SManipulator::EType::unsetf,		std::ios_base::showpoint);
	const CLog::SManipulator noshowpos		(CLog::SManipulator::EType::unsetf,		std::ios_base::showpos);
	const CLog::SManipulator noskipws		(CLog::SManipulator::EType::unsetf,		std::ios_base::skipws);
	const CLog::SManipulator nounitbuf		(CLog::SManipulator::EType::unsetf,		std::ios_base::unitbuf);
	const CLog::SManipulator nouppercase	(CLog::SManipulator::EType::unsetf,		std::ios_base::uppercase);
	const CLog::SManipulator oct			(CLog::SManipulator::EType::setf,		std::ios_base::oct,			std::ios_base::basefield);
	const CLog::SManipulator dec			(CLog::SManipulator::EType::setf,		std::ios_base::dec,			std::ios_base::basefield);
	const CLog::SManipulator hex			(CLog::SManipulator::EType::setf,		std::ios_base::hex,			std::ios_base::basefield);
	const CLog::SManipulator fixed			(CLog::SManipulator::EType::setf,		std::ios_base::fixed,		std::ios_base::floatfield);
	const CLog::SManipulator scientific		(CLog::SManipulator::EType::setf,		std::ios_base::scientific,	std::ios_base::floatfield);
	const CLog::SManipulator internal		(CLog::SManipulator::EType::setf,		std::ios_base::internal,	std::ios_base::adjustfield);
	const CLog::SManipulator left			(CLog::SManipulator::EType::setf,		std::ios_base::left,		std::ios_base::adjustfield);
	const CLog::SManipulator right			(CLog::SManipulator::EType::setf,		std::ios_base::right,		std::ios_base::adjustfield);
	const CLog::SManipulator endl			(CLog::SManipulator::EType::endl);
	const CLog::SManipulator ends			(CLog::SManipulator::EType::ends);
	const CLog::SManipulator flush			(CLog::SManipulator::EType::flush);
	const CLog::SManipulator indent			(CLog::SManipulator::EType::indent,		1);
	const CLog::SManipulator unindent		(CLog::SManipulator::EType::unindent,	1);
} // namespace NLog