// pch.h : Il s'agit d'un fichier d'en-tête précompilé.
// Les fichiers listés ci-dessous sont compilés une seule fois, ce qui améliore les performances de génération des futures builds.
// Cela affecte également les performances d'IntelliSense, notamment la complétion du code et de nombreuses fonctionnalités de navigation du code.
// Toutefois, les fichiers listés ici sont TOUS recompilés si l'un d'entre eux est mis à jour entre les builds.
// N'ajoutez pas de fichiers fréquemment mis à jour ici, car cela annule les gains de performance.

#ifndef PCH_H
#define PCH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER >= 1400 // valid for VS2005 and later

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\" \
                        type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='x86' \
                        publicKeyToken='6595b64144ccf1df' \
                        language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\" \
                        type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='ia64' \
                        publicKeyToken='6595b64144ccf1df' \
                        language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\" \
                        type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='amd64' \
                        publicKeyToken='6595b64144ccf1df' \
                        language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\" \
                        type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='*' \
                        publicKeyToken='6595b64144ccf1df' \
                        language='*'\"")
#endif

#define WINVER		0x0601
#else
#define WINVER		0x0500
#endif

#define _WIN32_IE	0x0800

#define _CRT_SECURE_NO_WARNINGS

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		    // Exclude rarely-used stuff from Windows headers
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclut infobulles inutiles
#endif

#ifndef _AFXDLL
#define _AFXDLL                 // DLL MFC partagée
#endif

// ajouter les en-têtes à précompiler ici
#include "framework.h"
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
#include <Winsock2.h>
#include <vector>
#include <map>

#if _MSC_VER < 1400 // before VS2005
#define override			// is part of the C++11 standard
#endif

#if !defined INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#ifdef _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#endif

#define ARRAYSIZEOF(a)	(sizeof(a) / sizeof(a[0]))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //PCH_H
