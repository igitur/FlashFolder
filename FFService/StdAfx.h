
#pragma once

#define WINVER 0x0601
#define _WIN32_WINNT WINVER
#define _WIN32_WINDOWS WINVER
#define _WIN32_IE 0x0600

#define VC_EXTRALEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxdisp.h>

#include <atlbase.h>

#include <windows.h>
#include <winsvc.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <wtsapi32.h>

#include <vector>
#include <map>
#include <string>
#include <memory>

#include <commonUtils\_autolink.h>
#include <commonUtils\Utils.h>
#include <common\_autolink.h>
#include <common\ProcessNames.h>
