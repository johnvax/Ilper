//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2020  J-F Garnier & Christoph Giesselink
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// IldevBase.cpp: implementation of the CIldevBase class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "IldevBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIldevBase::CIldevBase(bool bAutoExtAddr)
	: m_bAutoExtAddr(bAutoExtAddr)
{
	// HP-IL data and variables
	m_byAID = 0;							// Accessory ID
	m_byDEFADDR = 31;						// default address after AAU
	m_strDID = _T("");						// Device ID

	m_wStatus = 0;							// HP-IL status (always 0 here)
	m_byAddr = 0;							// HP-IL primary address (addressed by TAD or LAD)
											// bits 0-4 = AAD or AEP, bit 7 = 1 means auto address taken
	m_byAddr2nd = 0;						// HP-IL secondary address (addressed by SAD)
											// bits 0-4 = AES, bit 7 = 1 means auto address taken
	m_byFstate = 0;							// HP-IL state machine flags
	m_byPtsdi = 0;							// output pointer for device ID
	m_byPtssi = 0;							// output pointer for HP-IL status
	m_wTalkerFrame = 0;						// last talker frame
}

// ******************************************
// SetAID(byAID)
//
// set new Accessory ID
// ******************************************

VOID CIldevBase::SetAID(BYTE byAID)
{
	m_byAID = byAID;
	return;
}

// ******************************************
// GetAID()
//
// get current Accessory ID
// ******************************************

BYTE CIldevBase::GetAID() const
{
	return m_byAID;
}

// ******************************************
// SetID$(strDID)
//
// set new ID$
// ******************************************

VOID CIldevBase::SetID$(const CString& strDID)
{
	m_strDID = strDID;
	m_strDID.Replace(_T("\\r"),_T("\r"));
	m_strDID.Replace(_T("\\n"),_T("\n"));
	return;
}

// ******************************************
// GetID$()
//
// return ID$
// ******************************************

CString CIldevBase::GetID$() const
{
	CString strDID(m_strDID);
	strDID.Replace(_T("\r"),_T("\\r"));
	strDID.Replace(_T("\n"),_T("\\n"));
	return strDID;
}

// ******************************************
// GetAddr()
//
// return device address
//   upper byte : secondary address
//   lower byte : primary address
// ******************************************

WORD CIldevBase::GetAddr() const
{
	WORD wAddr = 0;

	if ((m_byAddr & 0x80) != 0)				// assigned primary address
	{
		wAddr = m_byAddr & 0x1F;

		if ((m_byAddr2nd & 0x80) != 0)		// assigned secondary address
		{
			wAddr |= (static_cast<WORD>(m_byAddr2nd & 0x1F) + 1) << 8;
		}
	}
	return wAddr;
}

// set HP-IL auto extended address support
VOID CIldevBase::SetAutoExtAddr(bool bAutoExtAddr)
{
	m_bAutoExtAddr = bAutoExtAddr;
	return;
}

// switch to active listener mode
bool CIldevBase::SetActiveListener(bool bEnable)
{
	if (bEnable)
	{
		// HP-IL state is idle, in default address mode and valid default address
		bEnable = (m_byFstate == 0 && (m_byAddr & 0x80) == 0 && m_byDEFADDR != 31);
		if (bEnable)
		{
			m_byFstate = 0x80;				// addressed listen
		}
	}
	else
	{
		m_byFstate &= ~0x80;				// remove addressed listen
	}
	return bEnable;
}

// check if device is configured as active listener
bool CIldevBase::IsActiveListener()
{
	return m_byFstate == 0x80 && (m_byAddr & 0x80) == 0 && m_byDEFADDR != 31;
}

// ******************************************
// class_doe(frame)
//
// manage the HPIL data frames
// returns the returned frame
// ******************************************

WORD CIldevBase::class_doe(WORD wFrame)
{
	if ((m_byFstate & 0xC0) == 0x40)		// addressed talker
	{
		WORD wTalkerError = 0;				// no talker error

		if ((m_byFstate & 0x03) != 0)		// active talker
		{
			// compare last talker frame with actual frame without SRQ bit
			wTalkerError = static_cast<WORD>((wFrame & 0x6FF) != (m_wTalkerFrame & 0x6FF));

			// data (SDA), status (SST) or accessory ID (SDI)
			if (!wTalkerError && (m_byFstate & 0x02) != 0)
			{
				WORD wSrqBit = wFrame & 0x100; // actual SRQ state

				// status (SST) or accessory ID (SDI)
				if ((m_byFstate & 0x01) != 0)
				{
					// 0x43: active talker (multiple byte status)
					if (m_byPtssi > 0)		// SST
					{
						if (--m_byPtssi > 0)
						{
							wFrame = (m_wStatus >> ((m_byPtssi - 1) * 8)) & 0xFF;
						}
					}
					if (m_byPtsdi > 0)		// SDI
					{
						ASSERT(!m_strDID.IsEmpty());
						wFrame = LPCTSTR(m_strDID)[m_byPtsdi++];

						if (wFrame == 0)	// end of string
							m_byPtsdi = 0;
					}
					if (m_byPtssi == 0 && m_byPtsdi == 0)
					{
						// EOT for SST and SDI
						wFrame = 0x540;		// EOT
					}
				}
				else // 0x42: active talker (data)
				{
					// SDA
					wFrame = OutData(wFrame);
				}

				// a set SRQ bit doesn't matter on ready class frames
				ASSERT((wFrame & 0x700) != 0x400);
				wFrame |= wSrqBit;			// restore SRQ bit
			}
			else // 0x41: active talker (single byte status)
			{
				// end of SAI, NRD or talker error
				wFrame = 0x540;				// EOT
			}
		}

		if (wFrame == 0x540)				// EOT type
		{
			wFrame += wTalkerError;			// check for error and set ETO/ETE frame
			m_byFstate &= ~0x03;			// delete active talker
		}

		m_wTalkerFrame = wFrame;			// last talker frame
	}

	if ((m_byFstate & 0xC0) == 0x80)		// listener
	{
		InData(wFrame);
	}
	return wFrame;
}

// ******************************************
// class_cmd(frame)
//
// manage the HPIL command frames
// returns the returned frame
// ******************************************

WORD CIldevBase::class_cmd(WORD wFrame)
{
	WORD n = wFrame & 0xFF;

	switch (n >> 5)
	{
	case 0:
		switch (n)
		{
		case 4: // SDC
			if ((m_byFstate & 0x80) != 0)	// listener
				ClearDevice();
			break;
		case 20: // DCL
			ClearDevice();
			break;
		}
		break;
	case 1: // LAD
		n = n & 31;
		if (n == 31) // UNL
		{
			if ((m_byFstate & 0xA0) != 0)	// listener
				m_byFstate &= 0x50;
		}
		else
		{
			// else, if MLA go to listen state
			if ((m_byFstate & 0x80) == 0 && n == (m_byAddr & 31))
			{
				m_byFstate = ((m_byAddr2nd & 0x80) == 0)
						   ? 0x80			// addressed listen
						   : 0x20;			// addressed listen in secondary address mode
			}
		}
		break;
	case 2: // TAD
		n = n & 31;
		if (n == (m_byAddr & 31))
		{
			// if MTA go to talker state
			m_byFstate = ((m_byAddr2nd & 0x80) == 0)
					   ? 0x40				// addressed talker
					   : 0x10;				// addressed talker in secondary address mode
		}
		else // UNT
		{
			// else if talker, go to idle state
			if ((m_byFstate & 0x50) != 0)
				m_byFstate &= 0xA0;
		}
		break;
	case 3: // SAD
		if ((m_byFstate & 0x30) != 0)		// LAD or TAD address matched
		{
			n = n & 31;
			if (n == (m_byAddr2nd & 31))	// secondary address match
			{
				m_byFstate <<= 2;			// switch to addressed listen/talker
			}
			else
			{
				m_byFstate = 0;				// secondary address don't match, go to idle state
			}
		}
		else
		{
			wFrame = ClassCmdSadExt(wFrame);
		}
		break;
	case 4:
		n = n & 31;
		switch (n)
		{
		case 16: // IFC
			m_byFstate = 0;
			break;
		case 26: // AAU
			m_byAddr = m_byDEFADDR;
			m_byAddr2nd = 0;
			break;
		}
		break;
	default:
		wFrame = ClassCmdExt(wFrame);		// interface for implementation of extra commands
	}
	return wFrame;
}

// ******************************************
// class_rdy(frame)
//
// manage the HPIL ready frames
// returns the returned frame
// ******************************************

WORD CIldevBase::class_rdy(WORD wFrame)
{
	WORD n = wFrame & 0xFF;

	if (n <= 127)
	{
		// SOT
		if ((m_byFstate & 0xC0) == 0x40)	// addressed as talker
		{
			if (n == 66) // NRD
			{
				m_byPtsdi = 0;				// abort transfer
				m_byPtssi = 0;
				m_byFstate = 0x41;			// (single byte status) for EOT
			}
			else
			{
				if (n == 96) // SDA
				{
					wFrame = OutData(wFrame);
					if (wFrame != 0x560)	// not SDA, received data
					{
						m_byFstate = 0x42;	// active talker (data)
						m_wTalkerFrame = wFrame; // last talker frame
					}
				}
				else
				{
					if (n == 97) // SST
					{
						// update IL status and return no. of status bytes
						m_byPtssi = GetILStatus(m_wStatus);

						if (m_byPtssi > 0)	// response to status request
						{
							wFrame = (m_wStatus >> ((m_byPtssi - 1) * 8)) & 0xFF;
							m_byFstate = 0x43; // active talker (multiple byte status)
							m_wTalkerFrame = wFrame; // last talker frame
						}
						// else no response
					}
					else
					{
						if (n == 98) // SDI
						{
							// have a Device ID
							if (!m_strDID.IsEmpty())
							{
								wFrame = LPCTSTR(m_strDID)[0];
								m_byPtsdi = 1;
								m_byFstate = 0x43; // active talker (multiple byte status)
								m_wTalkerFrame = wFrame; // last talker frame
							}
							// else no response
						}
						else
						{
							if (n == 99) // SAI
							{
								wFrame = m_byAID;
								m_byFstate = 0x41; // active talker (single byte status)
								m_wTalkerFrame = wFrame; // last talker frame
							}
						}
					}
				}
			}
		}
	}
	else
	{
		ASSERT(n >= 0x80);

		if (n < 0x80 + 31) // AAD
		{
			// AAD, if not already an assigned address, take it
			if ((m_byAddr & 0x80) == 0 && m_byAddr2nd == 0)
			{
				m_byAddr = static_cast<BYTE>(n);
				wFrame++;
			}
		}
		else if (m_bAutoExtAddr && n >= 0xA0 && n < 0xA0 + 31) // AEP
		{
			// AEP, if not already an assigned address and got an AES frame, take it
			if ((m_byAddr & 0x80) == 0 && (m_byAddr2nd & 0x80) != 0)
			{
				m_byAddr = static_cast<BYTE>(n & 0x9F);
			}
		}
		else if (m_bAutoExtAddr && n >= 0xC0 && n < 0xC0 + 31) // AES
		{
			// AES, if not already an assigned address, take it
			if ((m_byAddr & 0x80) == 0)
			{
				m_byAddr2nd = static_cast<BYTE>(n & 0x9F);
				wFrame++;
			}
		}
	}
	return wFrame;
}

// ******************************************
// Device(frame)
//
// manage a HPIL frame
// returns the modified frame
// ******************************************

WORD CIldevBase::Device(WORD wFrame)
{
	// TRACE(_T("Frame = %04X\n"),wFrame);

	if ((wFrame & 0x400) == 0)				// DOE
	{
		wFrame = class_doe(wFrame);			// data frame
	}
	else if ((wFrame & 0x700) == 0x400)		// CMD
	{
		wFrame = class_cmd(wFrame);			// command frame
	}
	else if ((wFrame & 0x700) == 0x500)		// RDY
	{
		wFrame = class_rdy(wFrame);			// ready frame
	}
	return wFrame;
}
