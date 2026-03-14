// Ildrive.h: interface for the CIldrive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILDRIVE_H__73BF8EF8_0604_49BD_A3E1_7CFEB115B352__INCLUDED_)
#define AFX_ILDRIVE_H__73BF8EF8_0604_49BD_A3E1_7CFEB115B352__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IldevBase.h"

class CIldrive : public CIldevBase
{
public:
	CIldrive(LPCTSTR lpszHdiscfile, bool bAutoExtAddr = false);
	virtual ~CIldrive();

	VOID SetHdisc(LPCTSTR lpszHdiscfile);	// set new file name after disk change

	// set/get the physical attributes of the medium
	VOID SetPhysicalParameters(DWORD dwTracks, DWORD dwSides, DWORD dwSectors);
	VOID GetPhysicalParameters(DWORD& dwTracks, DWORD& dwSides, DWORD& dwSectors) const;

private:
	CString m_lpszHdiscfile;
	bool    m_bValidFilename;

	// disk management variables
	BYTE m_byDevl;							// dev lstn
	BYTE m_byDevt;							// dev tlker
	LONG m_lOc;								// byte pointer
	WORD m_wPe;								// record pointer
	WORD m_wPe0;
	bool m_bFpt;							// flag pointer
	BYTE m_byFlpwr;							// flag partial write
	BYTE m_byPtout;							// pointer out

	// 640kb floppy drive descriptor, 80 tracks, 2 sides, 16 sect/track
	// 630Kb floppy drive descriptor, 77 tracks, 2 sides, 16 sect/track (HP 9114A)
	// 128Kb cassette drive descriptor, 2 tracks, 1 side, 256 sect/track
	BYTE m_byLif[12];						// lif descriptor
	WORD m_wNbe;							// number of 256-byte sectors

	BYTE m_byBuf0[256];						// buffer 0
	BYTE m_byBuf1[256];						// buffer 1

	// get DWORD in Big Endian
	static DWORD GetDWORD(CONST BYTE *pbyData);
	// set DWORD in Big Endian
	static VOID SetDWORD(LPBYTE pbyData, DWORD dwData);

	VOID FixLifHeader();					// fix wrong LIF header caused by HP-82401A rev 1A

	bool CheckFilename() const;				// check if current disk image path contain a file name

	VOID rrec();							// read sector
	VOID wrec();							// write sector
	VOID format_disc();						// format LIF image file
	WORD LastSectorNo();					// read last sector n° from LIF header

	VOID copybuf();
	VOID exchbuf();

	VOID ClearDevice() override;			// clear device
	VOID InData(WORD wFrame) override;		// listener transfer
	WORD OutData(WORD wFrame) override;		// talker transfer

	// WORD ClassCmdSadExt(WORD wFrame) override; // SAD command extention
	WORD ClassCmdExt(WORD wFrame) override;	// drive specific DDL and DDT commands
};

#endif // !defined(AFX_ILDRIVE_H__73BF8EF8_0604_49BD_A3E1_7CFEB115B352__INCLUDED_)
