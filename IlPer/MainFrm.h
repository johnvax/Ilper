
// MainFrm.h : interface de la classe CMainFrame
//

#pragma once
#include "EditCC.h"

class CIL;
class CInterface;							// COM interface object class
class CMnemo;								// the IL scope
class CIldisp;								// the IL printer
class CIldrive;								// the IL disk drive
class CDosLinkDev;							// the DOSLINK interface
class CIntLoop;								// internal HP-IL loop

class CMainFrame : public CFrameWnd
{
	public:
		CMainFrame() noexcept;
		virtual ~CMainFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Dialog Data
	//{{AFX_DATA(CMainFrame)
	CComboBox	m_CFontSize;
	CComboBox	m_CComboPort;
	CEdit		m_CEditDisk1;
	CEdit		m_CEditDisk2;
	CEdit		m_CEditOutFile;
	CEdit		m_CEditInFile;
	CButton		m_btnStop;
	CButton		m_btnStart;
	//}}AFX_DATA

protected:
	BOOL m_bResizing;
	bool m_bInitDone;

protected:
	DECLARE_DYNAMIC(CMainFrame)

	//{{AFX_VIRTUAL(CMainFrame)
	//}}AFX_VIRTUAL

	LRESULT OnInterfaceStopped(WPARAM wParam, LPARAM lParam);
	
	// Generated message map functions
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnSelchangeFontSize();

	afx_msg void OnStart();
	afx_msg void OnStop();

	afx_msg void OnBrowseF1();
	afx_msg void OnBrowseF2();
	afx_msg void OnSelendokComport();

	afx_msg void OnScopeEn();
	afx_msg void OnIdyEn();
	afx_msg void OnRfcEn();

	afx_msg void OnPrtActListen();
	afx_msg void OnUpdatePrtActListen(CCmdUI* pCmdUI);
	afx_msg void OnHpilSetup();
	afx_msg void OnHpilDeviceInfo();
	afx_msg void OnBrowseOut();
	afx_msg void OnBrowseIn();
	afx_msg void OnCloseall();

	afx_msg void OnAppExit();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	//}}AFX_MSG

	//	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	// Variables membres (remplace les globales HWND)
	CButton m_groupDisplay;
	CEditCC m_editDisplay;
	CButton m_groupScope;
	CEditCC m_editScope;
	CFont   m_fontDisplay;
	CFont	m_fontEdit;
	CFont	m_fontSystem;
	int		baseHeight100;		// Font height : Base 100%

	CButton m_massStorage;
    CStatic m_lblDisk1;
    CButton m_btnDisk1;
    CStatic m_lblDisk2;
    CButton m_btnDisk2;

	CButton m_groupDoslink;
	CButton m_btnOut;
	CButton m_btnIn;
	CButton m_btnC;

    CButton m_groupLinks;

	CStatic m_bottomTCPIPLabel;
	CStatic m_bottomTCPIPPorts;
    CButton m_chkScope;
    CButton m_chkIDY;
    CButton m_chkRFC;

	CString m_strHdiskFile1;				// harddisk filename 1
	CString m_strHdiskFile2;				// harddisk filename 2
	CString	m_strOutFile;					// DosLink OutFile
	CString	m_strInFile;					// DosLink InFile

	DWORD   m_dwBaudRate;					// baudrate of Pilbox
	BOOL	m_bPilboxIDY;					// enable IDY frames on USB

	BOOL	m_bScope;						// update scope window
	BOOL	m_bRFC;							// show RFC frames
	BOOL	m_bIDY;							// show IDY frames

	LONG	m_nPosX;						// display position & size
	LONG	m_nPosY;
	LONG	m_nWidth;
	LONG	m_nHeight;

	CString	m_strComPort;

	CString	m_strAddrOut;
	UINT	m_uPortIn;
	UINT	m_uPortOut;
	DWORD	m_dwConnectTimeout;				// client connect timeout

	BOOL	m_bAutoExtAddr;					// HP-IL auto extended address support

	UINT	m_uPosScope;					// position of scope device in the loop
	UINT	m_uPosPrinter;					// position of printer device in the loop
	UINT	m_uPosDisk1;					// position of disk 1 device in the loop
	UINT	m_uPosDisk2;					// position of disk 2 device in the loop
	UINT	m_uPosDosLink;					// position of DosLink device in the loop

	CInterface* m_pComObj;					// interface object class (COM)

	CMnemo* m_pMnemo;					// the mnemonic disassembler
	CIldisp* m_pPrinter;				// the printer
	CIldrive* m_pDiskDrv1;				// the disk drive 1
	CIldrive* m_pDiskDrv2;				// the disk drive 2
	CDosLinkDev* m_pDosLink;				// the DOSLINK device
	CIntLoop* m_pIntLoop;				// internal HP-IL loop

	std::map<CIL*, CString> m_mapNames;		// device names

	CString	ReadString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault);
	UINT	ReadInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault);
	BOOL	WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	BOOL	WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue);

	VOID SetWindowLocation(INT nPosX, INT nPosY, INT nWidth, INT nHeight);
	VOID SetCommList(LPCTSTR szComPort);
	VOID ShowScopeWnd(BOOL bEnable);
};

