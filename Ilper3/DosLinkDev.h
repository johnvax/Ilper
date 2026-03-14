// DosLinkDev.h: interface for the CDosLinkDev class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOSLINKDEV_H__F94B9C6B_B078_4088_830D_5BB695B0D87A__INCLUDED_)
#define AFX_DOSLINKDEV_H__F94B9C6B_B078_4088_830D_5BB695B0D87A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IldevBase.h"

class CDosLinkDev : public CIldevBase
{
public:
	CDosLinkDev(LPCTSTR lpszOutFile, LPCTSTR lpszInFile, bool bAutoExtAddr = false);
	virtual ~CDosLinkDev();

	VOID SetOutFile(LPCTSTR lpszOutFile);	// set new outfile name
	VOID SetInFile(LPCTSTR lpszInFile);		// set new infile name
	VOID ClearDevice() override;			// clear device

private:
	CRITICAL_SECTION m_csLock;				// critical section for file access lock

	CString m_strOutFile;					// the out file name
	CString m_strInFile;					// the in file name

	// device emulation
	FILE *fpcin;							// in file
	FILE *fpcout;							// out file

	VOID InData(WORD wFrame) override;		// listener transfer
	WORD OutData(WORD wFrame) override;		// talker transfer
};

#endif // !defined(AFX_DOSLINKDEV_H__F94B9C6B_B078_4088_830D_5BB695B0D87A__INCLUDED_)
