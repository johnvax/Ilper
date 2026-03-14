// Pilbox.h: interface for the CPilbox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PILBOX_H__9FCF856E_B61B_4636_B5EE_FF40BCA8F7F5__INCLUDED_)
#define AFX_PILBOX_H__9FCF856E_B61B_4636_B5EE_FF40BCA8F7F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Interface.h"

class CPilbox : public CInterface
{
public:
	CPilbox(CIL *pIL, LPCTSTR lpszComPort, DWORD dwBaudRate, bool bIDY, CWnd* pParent = NULL);
	virtual ~CPilbox();

	DWORD GetBaudRate() const { return m_dwBaudRate; }

	bool  StartServer() override;
	void  StopServer() override;

private:
	HANDLE m_hComm;

	const bool m_bIDY;						// enable IDY frames on USB
	bool m_bUSE8BITS;						// use 8-bit format with PILBox

	DWORD m_dwBaudRate;						// baud rate
	BYTE  m_byLastHigh;						// store the last sent/received high byte on serial

	VOID InitComm(DWORD dwBaudrate);		// initialize COM port
	bool InitPILBox(WORD wCmd);
	bool PILBoxCmd(WORD wCmd);
	bool SendFrame(WORD wFrame) override;	// PILBox send frame
	VOID PILBox(BYTE byVal);

	static UINT ThreadComServer(LPVOID pParam);
};

#endif // !defined(AFX_PILBOX_H__9FCF856E_B61B_4636_B5EE_FF40BCA8F7F5__INCLUDED_)
