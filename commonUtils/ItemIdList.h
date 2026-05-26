/* This file is part of FlashFolder. 
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net ) 
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
#pragma once

#include <memory>

typedef std::shared_ptr< ITEMIDLIST > SpITEMIDLIST;
typedef std::shared_ptr< const ITEMIDLIST > SpcITEMIDLIST;

struct SharedPidlDeleter
{
	void operator()( ITEMIDLIST* pidl ) const { ::CoTaskMemFree( (LPVOID) pidl ); }
	void operator()( const ITEMIDLIST* pidl ) const { ::CoTaskMemFree( (LPVOID) pidl ); }
};

inline SpITEMIDLIST MakeSharedPidl( LPITEMIDLIST pidl )
{
	return SpITEMIDLIST( reinterpret_cast<ITEMIDLIST*>( pidl ), SharedPidlDeleter() );
}

inline SpcITEMIDLIST MakeSharedPidl( LPCITEMIDLIST pidl )
{
	return SpcITEMIDLIST( reinterpret_cast<const ITEMIDLIST*>( pidl ), SharedPidlDeleter() );
}

inline HRESULT GetPidlFromPath( PIDLIST_ABSOLUTE* pidlResult, LPCWSTR path )
{
	SFGAOF attr = 0;
	return ::SHParseDisplayName( path, NULL, pidlResult, 0, &attr );
}

HRESULT GetPidlFromPath( SpITEMIDLIST* pidlResult, LPCWSTR path );
