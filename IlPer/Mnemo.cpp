//////////////////////////////////////////////////////////////////////
// ILPER 2.41 for Windows
// Copyright (c) 2008-2012  J-F Garnier
// Visual C++ version by Christoph Giesselink 2023
//
// Mnemo.cpp: implementation of the CMnemo class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Mnemo.h"
#include "TextSvr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMnemo::CMnemo(CWnd& wndScope, DWORD dwRefresh, INT nColMax)
	: m_TxtSvr(wndScope,dwRefresh)
	, m_bScope(TRUE)
	, m_bRFC(TRUE)
	, m_bIDY(TRUE)
	, m_nColMax(nColMax)
{
}

CMnemo::~CMnemo()
{
}

// --------------------------------
// ILMnemo()
// returns the frame mnemonic
// --------------------------------

VOID CMnemo::ILMnemo(WORD wFrame,LPTSTR s)
{
	const static struct
	{
		WORD wOpc;							// opcode
		WORD wMask;							// opcode mask
		TCHAR cMne[4];						// mnemonic
	} sCodes[] =
	{
		0x000, 0x700, _T("DAB"),
		0x100, 0x700, _T("DSR"),
		0x200, 0x700, _T("END"),
		0x300, 0x700, _T("ESR"),
		0x400, 0x7FF, _T("NUL"),
		0x401, 0x7FF, _T("GTL"),
		0x404, 0x7FF, _T("SDC"),
		0x405, 0x7FF, _T("PPD"),
		0x408, 0x7FF, _T("GET"),
		0x40F, 0x7FF, _T("ELN"),
		0x410, 0x7FF, _T("NOP"),
		0x411, 0x7FF, _T("LLO"),
		0x414, 0x7FF, _T("DCL"),
		0x415, 0x7FF, _T("PPU"),
		0x418, 0x7FF, _T("EAR"),
		0x43F, 0x7FF, _T("UNL"),
		0x420, 0x7E0, _T("LAD"),
		0x45F, 0x7FF, _T("UNT"),
		0x440, 0x7E0, _T("TAD"),
		0x460, 0x7E0, _T("SAD"),
		0x480, 0x7F0, _T("PPE"),
		0x490, 0x7FF, _T("IFC"),
		0x492, 0x7FF, _T("REN"),
		0x493, 0x7FF, _T("NRE"),
		0x49A, 0x7FF, _T("AAU"),
		0x49B, 0x7FF, _T("LPD"),
		0x4A0, 0x7E0, _T("DDL"),
		0x4C0, 0x7E0, _T("DDT"),
		0x400, 0x700, _T("CMD"),
		0x500, 0x7FF, _T("RFC"),
		0x540, 0x7FF, _T("ETO"),
		0x541, 0x7FF, _T("ETE"),
		0x542, 0x7FF, _T("NRD"),
		0x560, 0x7FF, _T("SDA"),
		0x561, 0x7FF, _T("SST"),
		0x562, 0x7FF, _T("SDI"),
		0x563, 0x7FF, _T("SAI"),
		0x564, 0x7FF, _T("TCT"),
		0x580, 0x7E0, _T("AAD"),
		0x5A0, 0x7E0, _T("AEP"),
		0x5C0, 0x7E0, _T("AES"),
		0x5E0, 0x7E0, _T("AMP"),
		0x500, 0x700, _T("RDY"),
		0x600, 0x700, _T("IDY"),
		0x700, 0x700, _T("ISR")
	};

	UINT i;

	// go through HP-IL opcode table
	for (i = 0; i < ARRAYSIZEOF(sCodes); ++i)
	{
		// found opcode in table
		if ((wFrame & sCodes[i].wMask) == sCodes[i].wOpc)
		{
			// get argument from mask
			const WORD wArg = (~sCodes[i].wMask) & 0xFF;

			lstrcpy(s,sCodes[i].cMne);		// copy name
			if (wArg != 0)					// opcode has an argument
			{
				// OxA0 is unbreakable space
				wsprintf(&s[3],_T("%c%02X"), 0xA0, (wFrame & wArg));
			}
			break;
		}
	}
	return;
}

// --------------------------------
// DisplayMnemo()
// display a string in the scope box
// (if the scope is enabled)
// --------------------------------

VOID CMnemo::DisplayMnemo(LPCTSTR lpszText)
{
	CString strText;
/*
	int nLength = m_TxtSvr.GetWindowTextLength();
	INT nCol = (nLength % ((8 * m_nColMax) + 2));
	if ((nCol % 8) != 0)					// fill with spaces to next column
	{
		INT nFill = 8 - (nCol % 8);			// fill the spaces
		strText += CString(_T("       ")).Left(nFill);
		nCol += nFill;
		if (nCol == 8 * m_nColMax)			// add CR + LF at end of line
		{
			nCol = 0;
			strText += _T("\r\n");
		}
	}
	nCol = (nCol + 8) / 8;
	*/
	strText += lpszText;
	/*
	if (nCol == m_nColMax)
	{
		strText += _T("\r\n");
	}*/
	m_TxtSvr.SetData(strText);
	return;
}

// --------------------------------
// DisplayMnemoDbg()
// display a string with time 
// information in the scope box
// (if the scope is enabled)
// --------------------------------

VOID CMnemo::DisplayMnemoDbg(LPCTSTR lpszText)
{
	CString strText;
	SYSTEMTIME st;

	int nLength = m_TxtSvr.GetWindowTextLength();

	GetLocalTime(&st);
	strText.Format(_T("%s%02d:%02d:%02d.%03d  %s"),
		(nLength) ? _T("\r\n") : _T(""),
		st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,
		lpszText);

	m_TxtSvr.SetData(strText);
	return;
}

// --------------------------------
// Device(frame)
//
// manage a HPIL frame
// returns the modified frame
// --------------------------------

WORD CMnemo::Device(WORD wFrame)
{
	if (   m_bScope
		&& (m_bRFC || wFrame != 0x500)
		&& (m_bIDY || (wFrame & 0x600) != 0x600))
	{
		TCHAR s[32];
		UINT  i;

		ILMnemo(wFrame,s);					// disassemble frame
		for (i = lstrlen(s); i < 8; ++i)	// adjust length to 8 char
		{
			s[i] = _T(' ');
		}
		s[i] = 0;							// EOS
		DisplayMnemo(s);					// show disassembly
	}
	return wFrame;
}
