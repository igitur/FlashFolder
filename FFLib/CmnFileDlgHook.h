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
#ifndef CMNFILEDLGHOOK_H__INCLUDED
#define CMNFILEDLGHOOK_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include "filedlg_base.h"

//-----------------------------------------------------------------------------------------
// class CmnFileDlgHook
//
// Specific code for hooking of common file dialogs.
//-----------------------------------------------------------------------------------------

class CmnFileDlgHook : public FileDlgHook_base
{
public:
	CmnFileDlgHook() : 
		m_hwndFileDlg( 0 ), m_fileDialogCanceled( false ),
        m_initDone( false ), m_isWindowActive( false ) {}

	// overridings of FileDlgHook_base
	virtual bool Init( HWND hwndFileDlg, HWND hWndTool );
	virtual bool SetFolder( LPCTSTR path );
	virtual bool GetFolder( LPTSTR folderPath );
	virtual bool SetFilter( LPCTSTR filter );

private:
	static LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResizeFileDialog();
	void ResizeNonResizableFileDialog( int x, int y, int newWidth, int newHeight );

	HWND m_hwndFileDlg, m_hwndTool;
	WNDPROC m_oldWndProc;
	bool m_isWindowActive;
	bool m_fileDialogCanceled;
	bool m_initDone;

	// options read from INI file specified in Init()

	int m_minFileDialogWidth;			    // prefered minimum size of file dialog
	int m_minFileDialogHeight;
	int m_centerFileDialog;				// true if file dialog should be centered
	bool m_bResizeNonResizableDlgs;			// true if those non-resizable dialogs should
											// be resized by FlashFolder
	int m_folderComboHeight;				// prefered heights of the combo boxes of 
	int m_filetypesComboHeight;				//   the file dialog
};

//-----------------------------------------------------------------------------------------

#endif //CMNFILEDLGHOOK_H__INCLUDED