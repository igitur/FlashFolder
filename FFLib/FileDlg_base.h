/* This file is part of FlashFolder. 
 * Copyright (C) 2007 zett42 ( zett42 at users.sourceforge.net ) 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef FILEDLG_BASE_H__INCLUDED
#define FILEDLG_BASE_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include <windows.h>

//-----------------------------------------------------------------------------------

// callback interface for FileDlgHook_base derivates to forward messages to 
// the FlashFolder tool window

namespace FileDlgHookCallbacks
{
	void OnInitDone();
	void OnFolderChange();
    void OnResize();
	void OnEnable( bool bEnable );
	void OnShow( bool bShow );
	void OnActivate( WPARAM wParam, LPARAM lParam );
	void OnDestroy( bool isOkBtnPressed );
	void SetTimer( DWORD interval );
};

//-----------------------------------------------------------------------------------

// abstract base class for manipulation of file dialogs

class FileDlgHook_base
{
public:
	FileDlgHook_base() {}
	virtual ~FileDlgHook_base() {}

	virtual bool Init( HWND hWndFileDlg, HWND hWndTool ) = 0;
	virtual void Uninstall() {}

	virtual bool SetFolder( PCIDLIST_ABSOLUTE folder ) = 0;
	virtual SpITEMIDLIST GetFolder() = 0;
	virtual bool SetFilter( LPCTSTR filter ) = 0;

	virtual void OnTimer() {}
};

//-----------------------------------------------------------------------------------

#endif //FILEDLG_BASE_H__INCLUDED
