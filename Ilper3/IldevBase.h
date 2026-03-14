// IldevBase.h: interface for the CIldevBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILDEVBASE_H__0AB79F80_3B56_43A0_9DD4_8A8948CF63DD__INCLUDED_)
#define AFX_ILDEVBASE_H__0AB79F80_3B56_43A0_9DD4_8A8948CF63DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IL.h"

class CIldevBase : public CIL
{
public:
	CIldevBase(bool bAutoExtAddr);
	virtual ~CIldevBase() { }

	VOID    SetAID(BYTE byAID);
	BYTE    GetAID() const;
	VOID    SetID$(const CString& strDID);
	CString GetID$() const;
	WORD    GetAddr() const;

	VOID SetAutoExtAddr(bool bAutoExtAddr);	// set HP-IL auto extended address support

	bool SetActiveListener(bool bEnable);	// switch to active listener mode
	bool IsActiveListener();				// check if device is configured as active listener

	WORD Device(WORD wFrame) override;

protected:
	// HP-IL data and variables
	BYTE m_byAID;							// Accessory ID
	BYTE m_byDEFADDR;						// default address after AAU
	CString m_strDID;						// Device ID

	WORD m_wStatus;							// HP-IL status (always 0 here)
	BYTE m_byAddr;							// HP-IL primary address (addressed by TAD or LAD)
											// bits 0-4 = AAD or AEP, bit 7 = 1 means auto address taken
	BYTE m_byAddr2nd;						// HP-IL secondary address (addressed by SAD)
											// bits 0-4 = AES, bit 7 = 1 means auto address taken
	BYTE m_byFstate;						// HP-IL state machine flags:
	// bit 7, bit 6, bit 5, bit 4:
	// 0      0      0      0    idle
	// 0      0      1      0    addressed listen in secondary address mode
	// 0      0      0      1    addressed talker in secondary address mode
	// 1      0      0      0    addressed listen
	// 0      1      0      0    addressed talker
	// bit 0 or bit 1 set        active talker
	// bit 1: SDA, SDI
	// bit 0: SST, SDI, SAI, SDA, NRD
	BYTE m_byPtsdi;							// output pointer for device ID
	BYTE m_byPtssi;							// output pointer for HP-IL status

private:
	bool m_bAutoExtAddr;					// HP-IL auto extended address support
	WORD m_wTalkerFrame;					// last talker frame

	// get IL status
	virtual BYTE GetILStatus(WORD& wStatus)
	{
		return 1u;							// one byte
		UNREFERENCED_PARAMETER(wStatus);
	}

	// implementation for clear device
	virtual VOID ClearDevice() { }

	// implementation for listener transfer
	virtual VOID InData(WORD wFrame) { }

	// implementation for talker transfer, for no response pass frame
	virtual WORD OutData(WORD wFrame) { return wFrame; }

	// hook to extend SAD command frame
	virtual WORD ClassCmdSadExt(WORD wFrame) { return wFrame; }

	// hook to extend command frames
	virtual WORD ClassCmdExt(WORD wFrame) { return wFrame; }

	WORD class_doe(WORD wFrame);			// data frames
	WORD class_cmd(WORD wFrame);			// command frames
	WORD class_rdy(WORD wFrame);			// ready frames
};

#endif // !defined(AFX_ILDEVBASE_H__0AB79F80_3B56_43A0_9DD4_8A8948CF63DD__INCLUDED_)
