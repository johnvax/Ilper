//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2015  J-F Garnier
// Visual C++ version by Christoph Giesselink 2020
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// Ildrive.cpp: implementation of the CIldrive class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Ildrive.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIldrive::CIldrive(LPCTSTR lpszHdiscfile, bool bAutoExtAddr)
	: CIldevBase(bAutoExtAddr)
{
	// HP-IL data and variables
	m_byAID = 16;							// Accessory ID = mass storage
	m_byDEFADDR = 2;						// default address after AAU
	m_strDID = _T("HP9114B");				// Device ID (HP 9114B)
//	m_strDID = _T("");						// Device ID (HP 82161A)

	SetPhysicalParameters(80,2,16);			// 640kb floppy drive, 80 tracks, 2 sides, 16 sect/track
	SetHdisc(lpszHdiscfile);				// setup disk image file name
}

CIldrive::~CIldrive()
{
}

// set new file name for disk change
VOID CIldrive::SetHdisc(LPCTSTR lpszHdiscfile)
{
	m_lpszHdiscfile = lpszHdiscfile;		// set new file
	m_bValidFilename = CheckFilename();		// file name exists
	ClearDevice();							// clear drive, reset internal pointers
	m_wStatus = 0;							// clear disk status to remove no media error
	return;
}

// set the physical attributes of the medium
VOID CIldrive::SetPhysicalParameters(DWORD dwTracks, DWORD dwSides, DWORD dwSectors)
{
	// 640kb floppy drive descriptor,   80 tracks, 2 sides,  16 sect/track (HP 9114B for Series 40 and 70)
	// 630Kb floppy drive descriptor,   77 tracks, 2 sides,  16 sect/track (HP 9114A)
	// 128Kb cassette drive descriptor,  2 tracks, 1 side,  256 sect/track (HP 82161A)

	SetDWORD(&m_byLif[0],dwTracks);			// tracks
	SetDWORD(&m_byLif[4],dwSides);			// sides
	SetDWORD(&m_byLif[8],dwSectors);		// sect/track

	// number of 256-byte sectors
	m_wNbe = static_cast<WORD>(dwTracks * dwSides * dwSectors);
	return;
}

// get the physical attributes of the medium
VOID CIldrive::GetPhysicalParameters(DWORD& dwTracks, DWORD& dwSides, DWORD& dwSectors) const
{
	dwTracks  = GetDWORD(&m_byLif[0]);		// tracks
	dwSides   = GetDWORD(&m_byLif[4]);		// sides
	dwSectors = GetDWORD(&m_byLif[8]);		// sect/track
	return;
}

// *****************************************************************
// helper functions
// *****************************************************************

// set DWORD in Big Endian
VOID CIldrive::SetDWORD(LPBYTE pbyData,DWORD dwData)
{
	pbyData[3] = static_cast<BYTE>(dwData >> 0);
	pbyData[2] = static_cast<BYTE>(dwData >> 8);
	pbyData[1] = static_cast<BYTE>(dwData >> 16);
	pbyData[0] = static_cast<BYTE>(dwData >> 24);
	return;
}

// get DWORD in Big Endian
DWORD CIldrive::GetDWORD(CONST BYTE *pbyData)
{
	DWORD dwData;

	dwData =   pbyData[0];
	dwData <<= 8;
	dwData |=  pbyData[1];
	dwData <<= 8;
	dwData |=  pbyData[2];
	dwData <<= 8;
	dwData |=  pbyData[3];
	return dwData;
}

// fix wrong LIF header caused by a HP-82160A or a HP-82401A rev 1A module
VOID CIldrive::FixLifHeader()
{
	// LIF identifier
	if (m_byBuf0[0x00] == 0x80 && m_byBuf0[0x01] == 0x00)
	{
		// LIF version 0, tracks = 0, sides = 0 and sect/track = 0
		if ((  *(WORD  *) &m_byBuf0[0x14]	// LIF version
			 | *(DWORD *) &m_byBuf0[0x18]	// tracks
			 | *(DWORD *) &m_byBuf0[0x1C]	// sides
			 | *(DWORD *) &m_byBuf0[0x20]	// sect/track
			) == 0
		   )
		{
			// formatted with a HP-82160A module
			m_byBuf0[0x15] = 0x01;			// LIF version 1

			// write physical medium attributes of drive
			CopyMemory(&m_byBuf0[0x18],m_byLif,ARRAYSIZEOF(m_byLif));
		}

		// LIF version 1
		else if (m_byBuf0[0x14] == 0x00 && m_byBuf0[0x15] == 0x01)
		{
			DWORD dwWrongNo;
			UINT  i;
			BOOL  bEqual = TRUE;

			// check for wrong numbers inside physical medium attributes
			dwWrongNo = ((m_wNbe - 1) << 16) | (m_wNbe - 1);

			for (i = 0x18; bEqual && i < 0x24; i += sizeof(dwWrongNo))
			{
				// check if
				// - number of tracks per surface
				// - number of surfaces
				// - number of blocks per track
				// contain all the wrong number
				bEqual = (GetDWORD(&m_byBuf0[i]) == dwWrongNo);
			}

			if (bEqual)						// is a HP-82401A rev 1A module
			{
				if ((m_byBuf0[0x12] & 0x40) != 0) // default directory size
				{
					// fix length of directory in blocks
					m_byBuf0[0x12] &= ~0x40;
				}

				// overwrite physical medium attributes
				CopyMemory(&m_byBuf0[0x18],m_byLif,ARRAYSIZEOF(m_byLif));
			}
		}
	}
	return;
}

// check if current disk image path contain a file name
bool CIldrive::CheckFilename() const
{
	TCHAR cFname[_MAX_FNAME];
	TCHAR cExt[_MAX_EXT];

	_tsplitpath_s(m_lpszHdiscfile,NULL,0, NULL,0, cFname, _countof(cFname), cExt, _countof(cExt));
	return *cFname != 0 || lstrlen(cExt) > 1;
}

// *****************************************************************
// hardware access functions
// *****************************************************************

// read one sector n° pe (256 bytes) into buf0
VOID CIldrive::rrec()
{
	CFile f;

	ASSERT(sizeof(m_byBuf0) == 256);

	if (f.Open(m_lpszHdiscfile,CFile::modeRead | CFile::typeBinary))
	{
		UINT nCount;

		f.Seek(m_wPe * 256,CFile::begin);
		nCount = f.Read(m_byBuf0,sizeof(m_byBuf0));
		if (nCount < sizeof(m_byBuf0))
		{
			for (UINT i = 0; i < ARRAYSIZEOF(m_byBuf0); ++i)
				m_byBuf0[i] = 0xFF;
		}
		m_wStatus = 0;						// reading successful, clear disk status
		f.Close();
	}
	else
	{
		m_wStatus = m_bValidFilename		// file name exists
				  ? 24						// blank, a foreign or unformatted media is in the drive 
				  : 20;						// no media
	}
	return;
}

// write buf0 to one sector n° pe (256 bytes)
VOID CIldrive::wrec()
{
	CFile f;

	ASSERT(sizeof(m_byBuf0) == 256);

	if (m_wPe == 0)							// LIF header record
	{
		FixLifHeader();						// fix a corrupted LIF header
	}

	if (f.Open(m_lpszHdiscfile,CFile::modeWrite | CFile::typeBinary))
	{
		try
		{
			f.Seek(m_wPe * 256,CFile::begin);
			f.Write(m_byBuf0,sizeof(m_byBuf0));
			m_wStatus = 0;					// writing successful, clear disk status
		}
		catch (CFileException &)
		{
			m_wStatus = 25;					// record number error
		}
		f.Close();
	}
	else
	{
		m_wStatus = 29;						// write protect
	}
	return;
}

// "format" a LIF image file
VOID CIldrive::format_disc()
{
	CFile f;
	BYTE byBuf[256];
	INT i;

	m_wStatus = 0;

	CFileException e;
	if (m_bValidFilename && f.Open(m_lpszHdiscfile,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary,&e))
	{
		// fill sector
		for (i = 0; i < ARRAYSIZEOF(byBuf); ++i)
			byBuf[i] = 0xFF;

		try
		{
			f.SeekToBegin();
			for (i = 0; i < 128; ++i)
			{
				f.Write(byBuf,256);
			}
			f.Close();
		}
		catch (CFileException &)
		{
			m_wStatus = 25;					// record number error
		}
	}
	else
	{
		m_wStatus = (m_bValidFilename && e.m_cause == CFileException::accessDenied)
				  ? 29						// write protect
				  : 20;						// no media
	}
	return;
}

// read last sector n° from LIF header
WORD CIldrive::LastSectorNo()
{
	CFile f;

	DWORD dwLastSector = m_wNbe - 1;		// last sector no. of selected medium

	if (f.Open(m_lpszHdiscfile,CFile::modeRead | CFile::typeBinary))
	{
		BYTE byData[0x24];
		UINT nCount;

		m_wStatus = 0;

		f.SeekToBegin();
		nCount = f.Read(byData,sizeof(byData));
		if (nCount == sizeof(byData))
		{
			// http://www.ata-atapi.com/hiwdos.html
			// BIOS Parameter Block ([00] jmp, [02] nop)
			if (byData[0x00] == 0xEB && byData[0x02] == 0x90)
			{
				dwLastSector = 0xFFFF;		// no sector limitation
			}
			// LIF identifier
			else if (byData[0x00] == 0x80 && byData[0x01] == 0x00)
			{
				// default last sector no. for 80 tracks, 2 sides, 16 sect/track disc
				dwLastSector = 80 * 2 * 16 - 1;

				// LIF version 1
				if (byData[0x14] == 0x00 && byData[0x15] == 0x01)
				{
					DWORD dwSectors = 1;

					// tracks, sides, sectors/track
					for (UINT i = 0x18; i < 0x24; i += sizeof(DWORD))
					{
						dwSectors *= GetDWORD(&byData[i]);
					}

					// valid sector information
					if (dwSectors > 0 && dwSectors <= 0x10000)
					{
						dwLastSector = --dwSectors;	// last sector
					}
					else
					{
						m_wStatus = 28;		// size error
					}
				}
			}
		}
		f.Close();
	}
	else
	{
		m_wStatus = m_bValidFilename		// file name exists
				  ? 24						// blank, a foreign or unformatted media is in the drive 
				  : 20;						// no media
	}
	return static_cast<WORD>(dwLastSector);
}

// ******************************************
// copybuf()
//
// copy buffer 0 to buffer 1
// ******************************************

VOID CIldrive::copybuf()
{
	INT i;

	m_lOc = 0;

	ASSERT(ARRAYSIZEOF(m_byBuf1) == ARRAYSIZEOF(m_byBuf0));

	for (i = 0; i < ARRAYSIZEOF(m_byBuf1); ++i)
	{
		m_byBuf1[i] = m_byBuf0[i];
	}
	return;
}

// ******************************************
// exchbuf()
//
// exchange buffers 0 and 1
// ******************************************

VOID CIldrive::exchbuf()
{
	BYTE byVal;
	INT  i;

	m_lOc = 0;

	ASSERT(ARRAYSIZEOF(m_byBuf1) == ARRAYSIZEOF(m_byBuf0));

	for (i = 0; i < ARRAYSIZEOF(m_byBuf1); ++i)
	{
		byVal		= m_byBuf1[i];
		m_byBuf1[i] = m_byBuf0[i];
		m_byBuf0[i] = byVal;
	}
	return;
}

// ******************************************
// ClearDevice()
//
// Clear drive, reset internal pointers
// ******************************************

VOID CIldrive::ClearDevice()
{
	m_bFpt = false;							// reset DDL4 partial sequence
	m_wPe = 0;								// record pointer = 0
	m_lOc = 0;								// byte pointer = 0

	memset(m_byBuf0,0xFF,sizeof(m_byBuf0));	// invalidate sector buffers
	memset(m_byBuf1,0xFF,sizeof(m_byBuf1));	// with HP41 as controller this buffer contains
											// normally a part of the DIR as cache
	return;
}

// ******************************************
// InData(WORD wFrame)
//
// receive data to disc according to DDL cmd
// ******************************************

VOID CIldrive::InData(WORD wFrame)
{
	switch (m_byDevl)
	{
	case 0:
	case 2:
	case 6:
		m_byBuf0[m_lOc] = wFrame & 255;
		m_lOc++;
		if (m_lOc > 255)
		{
			m_lOc = 0;
			wrec();
			m_wPe++;
			if (m_byFlpwr != 0)
				rrec();
		}
		else
		{
			if ((wFrame & 0x200) != 0)
			{
				// END
				wrec();
				if (m_byFlpwr == 0) m_wPe++;
			}
		}
		break;
	case 1:
		m_byBuf1[m_lOc] = wFrame & 255;
		m_lOc++;
		if (m_lOc > 255)
			m_lOc = 0;
		break;
	case 3:
		m_lOc = wFrame & 255;
		break;
	case 4:
		wFrame = wFrame & 255;
		if (m_bFpt)
		{
			m_wPe0 = m_wPe0 & 0xFF00;
			m_wPe0 = m_wPe0 | wFrame;
			if (m_wPe0 <= LastSectorNo())
			{
				m_wPe = m_wPe0;
			}
			else
			{
				m_wStatus = 28;				// size error
			}
			m_bFpt = false;
		}
		else
		{
			m_wPe0 = m_wPe0 & 255;
			m_wPe0 = m_wPe0 | (wFrame << 8);
			m_bFpt = true;
		}
		break;
	}
	return;
}

// ******************************************
// OutData()
//
// send data from disc according to DDT cmd
// ******************************************

WORD CIldrive::OutData(WORD wFrame)
{
	if (wFrame == 0x560)	// initial SDA
	{
		m_byPtout = 0;
	}

	switch (m_byDevt)
	{
	case 0:
	case 2:
		wFrame = m_byBuf0[m_lOc];
		m_lOc++;
		if (m_lOc > 255)
		{
			m_lOc = 0;
			rrec();
			m_wPe++;
		}
		break;
	case 1:
		wFrame = m_byBuf1[m_lOc];
		m_lOc++;
		if (m_lOc > 255)
		{
			m_lOc = 0;
			m_byDevt = 15;					// send EOT on the next SDA
		}
		break;
	case 3:
		switch (m_byPtout)
		{
			case 0:
				wFrame = m_wPe >> 8;
				m_byPtout++;
				break;
			case 1:
				wFrame = m_wPe & 255;
				m_byPtout++;
				break;
			case 2:
				wFrame = static_cast<WORD>(m_lOc) & 255;
				m_byPtout++;
				break;
			default:
				wFrame = 0x540; 			// EOT
				break;
		}
		break;
	case 6:
		if (m_byPtout < 12)
		{
			wFrame = m_byLif[m_byPtout];
			m_byPtout++;
		}
		else
		{
			wFrame = 0x540; 				// EOT
		}
		break;
	case 7:
		switch (m_byPtout)
		{
			case 0:
				wFrame = (m_wNbe - 1) >> 8;
				m_byPtout++;
				break;
			case 1:
				wFrame = (m_wNbe - 1) & 255;
				m_byPtout++;
				break;
			default:
				wFrame = 0x540; 			// EOT
		}
		break;
	default:
		wFrame = 0x540; 					// EOT
	}
	return wFrame;
}

// ******************************************
// ClassCmdSadExt(WORD wFrame)
//
// add SAD extention
// ******************************************

/*
WORD CIldrive::ClassCmdSadExt(WORD wFrame)
{
	WORD n = wFrame & 0xFF;

	// listener with disabled auto extended address mode
	if ((m_byFstate & 0xC0) == 0x80 && m_byAddr2nd == 0)
	{
		// SS/80 protocol
		switch (n) // transaction
		{
		case 0x65: // command message
			// @todo add implementation here
			break;
		case 0x6E: // execution message
			// @todo add implementation here
			break;
		}
	}
	// talker with disabled auto extended address mode
	else if ((m_byFstate & 0xC0) == 0x40 && m_byAddr2nd == 0)
	{
		// SS/80 protocol
		switch (n) // transaction
		{
		case 0x70: // reporting message
			// @todo add implementation here
			break;
		}
	}
	return wFrame;
}
*/

// ******************************************
// ClassCmdExt(WORD wFrame)
//
// add DDL and DDT commands
// ******************************************

WORD CIldrive::ClassCmdExt(WORD wFrame)
{
	WORD n = wFrame & 0xFF;

	switch (n >> 5)
	{
	case 5: // DDL
		n = n & 31;
		if ((m_byFstate & 0xC0) == 0x80)
		{
			m_byDevl = static_cast<BYTE>(n);
			switch (n)
			{
			case 1:
				m_byFlpwr = 0;
				break;
			case 2:
				m_lOc = 0;
				m_byFlpwr = 0;
				break;
			case 4:
				m_byFlpwr = 0;
				m_bFpt = false;
				break;
			case 5:
				format_disc();
				break;
			case 6:
				m_byFlpwr = 0x80;
				rrec();
				break;
			case 7:
				m_bFpt = false;
				m_wPe = 0;
				m_lOc = 0;
				break;
			case 8:
				wrec();
				if (m_byFlpwr == 0) m_wPe++;
				break;
			case 9:
				copybuf();
				break;
			case 10:
				exchbuf();
				break;
			}
		}
		break;
	case 6: // DDT
		n = n & 31;
		if ((m_byFstate & 0x40) == 0x40)
		{
			m_byDevt = static_cast<BYTE>(n);
			switch (n)
			{
			case 0:
				m_byFlpwr = 0;
				break;
			case 2:
				rrec();
				m_lOc = 0;
				m_byFlpwr = 0;
				m_wPe++;
				break;
			case 4:
				exchbuf();
				break;
			}
		}
		break;
	}
	return wFrame;
}
