/********************************************************************
*
* Copyright (c) 2002 Sven Wiegand <mail@sven-wiegand.de>
*
* You can use this and modify this in any way you want,
* BUT LEAVE THIS HEADER INTACT.
*
* Redistribution is appreciated.
*
* Changes by zett42 (zett42 at users.sourceforge.net):
*    FIX: CTreePropSheet did not use DPI-independent metrics
*    FIX: Prop-page caption colors did not work with some XP themes
*    ADD: Allow different captions for tree and prop-page.
*    ADD: Vista - draw page headline with task dialog "main instruction" style
*    ADD: Vista - use standard OS font and size (typically "Segoe UI", 9 pt)
*********************************************************************/


#include "stdafx.h"
#include "TreePropSheet.h"
#include "PropPageFrameDefault.h"
#include "..\HighColorTab.hpp"
#include "..\VistaFontHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



namespace TreePropSheet
{

//-------------------------------------------------------------------
// class CTreePropSheet
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CTreePropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CTreePropSheet)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(PSM_ADDPAGE, OnAddPage)
	ON_MESSAGE(PSM_REMOVEPAGE, OnRemovePage)
	ON_MESSAGE(PSM_SETCURSEL, OnSetCurSel)
	ON_MESSAGE(PSM_SETCURSELID, OnSetCurSelId)
	ON_MESSAGE(PSM_ISDIALOGMESSAGE, OnIsDialogMessage)
	
	ON_NOTIFY(TVN_SELCHANGINGA, s_unPageTreeId, OnPageTreeSelChanging)
	ON_NOTIFY(TVN_SELCHANGINGW, s_unPageTreeId, OnPageTreeSelChanging)
	ON_NOTIFY(TVN_SELCHANGEDA, s_unPageTreeId, OnPageTreeSelChanged)
	ON_NOTIFY(TVN_SELCHANGEDW, s_unPageTreeId, OnPageTreeSelChanged)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CTreePropSheet, CPropertySheet)

const UINT CTreePropSheet::s_unPageTreeId = 0x7EEE;

CTreePropSheet::CTreePropSheet()
:	CPropertySheet()
{
	Init();
}


CTreePropSheet::CTreePropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	Init();
}


CTreePropSheet::CTreePropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Init();
}

void CTreePropSheet::Init()
{
	m_bPageTreeSelChangedActive = FALSE;
	m_bTreeViewMode = TRUE;
	m_bPageCaption = FALSE;
	m_bTreeImages = FALSE;
	m_nPageTreeWidth = 150;
	m_pwndPageTree = NULL;
	m_pFrame = NULL;

	OSVERSIONINFO verInfo = { sizeof(verInfo) };
	::GetVersionEx( &verInfo );
	m_osVer = ( verInfo.dwMajorVersion << 8 ) | verInfo.dwMinorVersion;
}

CTreePropSheet::~CTreePropSheet()
{}

//----------------------------------------------------------------------------------------------------

INT_PTR CTreePropSheet::DoModal() 
{
	// Enable callback for setting Vista font.
    m_psh.dwFlags |= PSH_USECALLBACK;
    m_psh.pfnCallback = PropSheetProc;

    return CPropertySheet::DoModal();
}

void CTreePropSheet::BuildPropPageArray()
{
    CPropertySheet::BuildPropPageArray();
    
	if( (BYTE) ::GetVersion() >= 6 )
	{
		// Vista and above: set standard OS font and size for all pages 
		// (typically "Segoe UI", 9 pt).
		for( int nPage = 0; nPage < m_pages.GetSize(); ++nPage )
			SetStandardOsFontInDlgResource( (LPDLGTEMPLATE) m_psh.ppsp[ nPage ].pResource );
	}
}

int CALLBACK CTreePropSheet::PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{
    if( uMsg == PSCB_PRECREATE && (BYTE) ::GetVersion() >= 6 )
    {
		// Vista and above: set standard OS font and size for the property sheet itself 
		// (typically "Segoe UI", 9 pt).
		SetStandardOsFontInDlgResource( (LPDLGTEMPLATE) lParam );
	}
    return 0;
}

//----------------------------------------------------------------------------------------------------

BOOL CTreePropSheet::SetTreeViewMode(BOOL bTreeViewMode /* = TRUE */, BOOL bPageCaption /* = FALSE */, BOOL bTreeImages /* = FALSE */)
{
	if (IsWindow(m_hWnd))
	{
		// needs to becalled, before the window has been created
		ASSERT(FALSE);
		return FALSE;
	}

	m_bTreeViewMode = bTreeViewMode;
	if (m_bTreeViewMode)
	{
		m_bPageCaption = bPageCaption;
		m_bTreeImages = bTreeImages;
	}

	return TRUE;
}


BOOL CTreePropSheet::SetTreeWidth(int nWidth)
{
	if (IsWindow(m_hWnd))
	{
		// needs to be called, before the window is created.
		ASSERT(FALSE);
		return FALSE;
	}

	m_nPageTreeWidth = nWidth;

	return TRUE;
}


void CTreePropSheet::SetEmptyPageText(LPCTSTR lpszEmptyPageText)
{
	m_strEmptyPageMessage = lpszEmptyPageText;
}


DWORD	CTreePropSheet::SetEmptyPageTextFormat(DWORD dwFormat)
{
	DWORD	dwPrevFormat = m_pFrame->GetMsgFormat();
	m_pFrame->SetMsgFormat(dwFormat);
	return dwPrevFormat;
}


BOOL CTreePropSheet::SetTreeDefaultImages(CImageList *pImages)
{
	if (pImages->GetImageCount() != 2)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_DefaultImages.GetSafeHandle())
		m_DefaultImages.DeleteImageList();
	m_DefaultImages.Create(pImages);

	// update, if necessary
	if (IsWindow(m_hWnd))
		RefillPageTree();
	
	return TRUE;
}


BOOL CTreePropSheet::SetTreeDefaultImages(UINT unBitmapID, int cx, COLORREF crMask)
{
	if (m_DefaultImages.GetSafeHandle())
		m_DefaultImages.DeleteImageList();
	if (!m_DefaultImages.Create(unBitmapID, cx, 0, crMask))
		return FALSE;

	if (m_DefaultImages.GetImageCount() != 2)
	{
		m_DefaultImages.DeleteImageList();
		return FALSE;
	}

	return TRUE;
}


CTreeCtrl* CTreePropSheet::GetPageTreeControl()
{
	return m_pwndPageTree;
}


/////////////////////////////////////////////////////////////////////
// public helpers

BOOL CTreePropSheet::SetPageIcon(CPropertyPage *pPage, HICON hIcon)
{
	pPage->m_psp.dwFlags|= PSP_USEHICON;
	pPage->m_psp.hIcon = hIcon;
	return TRUE;
}


BOOL CTreePropSheet::SetPageIcon(CPropertyPage *pPage, UINT unIconId)
{
	HICON	hIcon = AfxGetApp()->LoadIcon(unIconId);
	if (!hIcon)
		return FALSE;

	return SetPageIcon(pPage, hIcon);
}


BOOL CTreePropSheet::SetPageIcon(CPropertyPage *pPage, CImageList &Images, int nImage)
{
	HICON	hIcon = Images.ExtractIcon(nImage);
	if (!hIcon)
		return FALSE;

	return SetPageIcon(pPage, hIcon);
}


BOOL CTreePropSheet::DestroyPageIcon(CPropertyPage *pPage)
{
	if (!pPage || !(pPage->m_psp.dwFlags&PSP_USEHICON) || !pPage->m_psp.hIcon)
		return FALSE;

	DestroyIcon(pPage->m_psp.hIcon);
	pPage->m_psp.dwFlags&= ~PSP_USEHICON;
	pPage->m_psp.hIcon = NULL;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// Overridable implementation helpers

CString CTreePropSheet::GenerateEmptyPageMessage(LPCTSTR lpszEmptyPageMessage, LPCTSTR lpszCaption)
{
	CString	strMsg;
	strMsg.Format(lpszEmptyPageMessage, lpszCaption);
	return strMsg;
}


CTreeCtrl* CTreePropSheet::CreatePageTreeObject()
{
	return new CTreeCtrl;
}


CPropPageFrame* CTreePropSheet::CreatePageFrame()
{
	return new CPropPageFrameDefault;
}


/////////////////////////////////////////////////////////////////////
// Implementation helpers

void CTreePropSheet::MoveChildWindows(int nDx, int nDy)
{
	CWnd	*pWnd = GetWindow(GW_CHILD);
	while (pWnd)
	{
		CRect	rect;
		pWnd->GetWindowRect(rect);
		rect.OffsetRect(nDx, nDy);
		ScreenToClient(rect);
		pWnd->MoveWindow(rect);

		pWnd = pWnd->GetNextWindow();
	}
}


void CTreePropSheet::RefillPageTree()
{
	if (!IsWindow(m_hWnd))
		return;

	m_pwndPageTree->DeleteAllItems();

	CTabCtrl	*pTabCtrl = GetTabControl();
	if (!IsWindow(pTabCtrl->GetSafeHwnd()))
	{
		ASSERT(FALSE);
		return;
	}

	const int	nPageCount = pTabCtrl->GetItemCount();

	// rebuild image list
	if (m_bTreeImages)
	{
		for (int i = m_Images.GetImageCount()-1; i >= 0; --i)
			m_Images.Remove(i);

		// add page images
		CImageList	*pPageImages = pTabCtrl->GetImageList();
		if (pPageImages)
		{
			for (int nImage = 0; nImage < pPageImages->GetImageCount(); ++nImage)
			{
				HICON	hIcon = pPageImages->ExtractIcon(nImage);
				m_Images.Add(hIcon);
				DestroyIcon(hIcon);
			}
		}

		// add default images
		if (m_DefaultImages.GetSafeHandle())
		{	
			HICON	hIcon;

			// add default images
			hIcon = m_DefaultImages.ExtractIcon(0);
			if (hIcon)
			{
				m_Images.Add(hIcon);
				DestroyIcon(hIcon);
			}
			hIcon = m_DefaultImages.ExtractIcon(1);
			{
				m_Images.Add(hIcon);
				DestroyIcon(hIcon);
			}
		}
	}

	// insert tree items
	for (int nPage = 0; nPage < nPageCount; ++nPage)
	{
		// Get title and image of the page
		CString	strPagePath;

		TCITEM	ti;
		ZeroMemory(&ti, sizeof(ti));
		ti.mask = TCIF_TEXT|TCIF_IMAGE;
		ti.cchTextMax = MAX_PATH;
		ti.pszText = strPagePath.GetBuffer(ti.cchTextMax);
		ASSERT(ti.pszText);
		if (!ti.pszText)
			return;

		pTabCtrl->GetItem(nPage, &ti);
		strPagePath.ReleaseBuffer();

		// Create an item in the tree for the page
		HTREEITEM	hItem = CreatePageTreeItem(ti.pszText);
		ASSERT(hItem);
		if (hItem)
		{
			m_pwndPageTree->SetItemData(hItem, nPage);

			// set image
			if (m_bTreeImages)
			{
				int	nImage = ti.iImage;
				if (nImage < 0 || nImage >= m_Images.GetImageCount())
					nImage = m_DefaultImages.GetSafeHandle()? m_Images.GetImageCount()-1 : -1;

				m_pwndPageTree->SetItemImage(hItem, nImage, nImage);
			}
			m_pwndPageTree->Expand(m_pwndPageTree->GetParentItem(hItem), TVE_EXPAND);
		}
	}
}


HTREEITEM CTreePropSheet::CreatePageTreeItem(LPCTSTR lpszPath, HTREEITEM hParent /* = TVI_ROOT */)
{
	CString		strPath(lpszPath);
	CString		strTopMostItem(SplitPageTreePath(strPath));
	
	// Check if an item with the given text does already exist
	HTREEITEM	hItem = NULL;
	HTREEITEM	hChild = m_pwndPageTree->GetChildItem(hParent);
	while (hChild)
	{
		if (m_pwndPageTree->GetItemText(hChild) == strTopMostItem)
		{
			hItem = hChild;
			break;
		}
		hChild = m_pwndPageTree->GetNextItem(hChild, TVGN_NEXT);
	}

	// Get caption for the tree only, if exists.
	int p = strTopMostItem.ReverseFind( '|' );
	if( p != -1 )
		strTopMostItem = strTopMostItem.Left( p );

	// If item with that text does not already exist, create a new one
	if (!hItem)
	{
		hItem = m_pwndPageTree->InsertItem(strTopMostItem, hParent);
		m_pwndPageTree->SetItemData(hItem, (DWORD_PTR)-1);
		if (!strPath.IsEmpty() && m_bTreeImages && m_DefaultImages.GetSafeHandle())
			// set folder image
			m_pwndPageTree->SetItemImage(hItem, m_Images.GetImageCount()-2, m_Images.GetImageCount()-2);
	}
	if (!hItem)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (strPath.IsEmpty())
		return hItem;
	else
		return CreatePageTreeItem(strPath, hItem);
}


CString CTreePropSheet::SplitPageTreePath(CString &strRest)
{
	int	nSeperatorPos = 0;
#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression constant
	while (TRUE)
	{
		nSeperatorPos = strRest.Find(_T("::"), nSeperatorPos);
		if (nSeperatorPos == -1)
		{
			CString	strItem(strRest);
			strRest.Empty();
			return strItem;
		}
		else if (nSeperatorPos>0)
		{
			// if there is an odd number of backslashes infront of the
			// seperator, than do not interpret it as separator
			int	nBackslashCount = 0;
			for (int nPos = nSeperatorPos-1; nPos >= 0 && strRest[nPos]==_T('\\'); --nPos, ++nBackslashCount);
			if (nBackslashCount%2 == 0)
				break;
			else
				++nSeperatorPos;
		}
	}
#pragma warning(pop)

	CString	strItem(strRest.Left(nSeperatorPos));
	strItem.Replace(_T("\\::"), _T("::"));
	strItem.Replace(_T("\\\\"), _T("\\"));
	strRest = strRest.Mid(nSeperatorPos+2);
	return strItem;
}


BOOL CTreePropSheet::KillActiveCurrentPage()
{
	HWND	hCurrentPage = PropSheet_GetCurrentPageHwnd(m_hWnd);
	if (!IsWindow(hCurrentPage))
	{
		ASSERT(FALSE);
		return TRUE;
	}

	// Check if the current page is really active (if page is invisible
	// an virtual empty page is the active one.
	if (!::IsWindowVisible(hCurrentPage))
		return TRUE;

	// Try to deactivate current page
	PSHNOTIFY	pshn;
	pshn.hdr.code = PSN_KILLACTIVE;
	pshn.hdr.hwndFrom = m_hWnd;
	pshn.hdr.idFrom = GetDlgCtrlID();
	pshn.lParam = 0;
	if (::SendMessage(hCurrentPage, WM_NOTIFY, pshn.hdr.idFrom, (LPARAM)&pshn))
		// current page does not allow page change
		return FALSE;

	// Hide the page
	::ShowWindow(hCurrentPage, SW_HIDE);

	return TRUE;
}


HTREEITEM CTreePropSheet::GetPageTreeItem(int nPage, HTREEITEM hRoot /* = TVI_ROOT */)
{
	// Special handling for root case
	if (hRoot == TVI_ROOT)
		hRoot = m_pwndPageTree->GetNextItem(NULL, TVGN_ROOT);

	// Check parameters
	if (nPage < 0 || nPage >= GetPageCount())
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (hRoot == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	// we are performing a simple linear search here, because we are
	// expecting only little data
	HTREEITEM	hItem = hRoot;
	while (hItem)
	{
		if ((signed)m_pwndPageTree->GetItemData(hItem) == nPage)
			return hItem;
		if (m_pwndPageTree->ItemHasChildren(hItem))
		{
			HTREEITEM	hResult = GetPageTreeItem(nPage, m_pwndPageTree->GetNextItem(hItem, TVGN_CHILD));
			if (hResult)
				return hResult;
		}

		hItem = m_pwndPageTree->GetNextItem(hItem, TVGN_NEXT);
	}

	// we've found nothing, if we arrive here
	return hItem;
}


BOOL CTreePropSheet::SelectPageTreeItem(int nPage)
{
	HTREEITEM	hItem = GetPageTreeItem(nPage);
	if (!hItem)
		return FALSE;

	return m_pwndPageTree->SelectItem(hItem);
}


BOOL CTreePropSheet::SelectCurrentPageTreeItem()
{
	CTabCtrl	*pTab = GetTabControl();
	if (!IsWindow(pTab->GetSafeHwnd()))
		return FALSE;

	return SelectPageTreeItem(pTab->GetCurSel());
}


void CTreePropSheet::UpdateCaption()
{
	HWND hPage = PropSheet_GetCurrentPageHwnd(GetSafeHwnd());
	CWnd* pPage = NULL;
	if( IsWindow( hPage ) )
		pPage = CWnd::FromHandle( hPage );
	BOOL bRealPage = IsWindow(hPage) && ::IsWindowVisible(hPage);
	HTREEITEM	hItem = m_pwndPageTree->GetSelectedItem();
	if (!hItem)
		return;

	// Extract the caption from the path. '|' can be used to specify a different caption for the
	// tree and for the page.
	CString	strCaption = m_pwndPageTree->GetItemText(hItem);
	if( bRealPage )
	{
		pPage->GetWindowText( strCaption );
		int p = strCaption.ReverseFind( '|' );
		if( p != -1 )
			strCaption = strCaption.Mid( p + 1 );
		else
		{
			for( int i = strCaption.GetLength(); i >= 0; --i )
				if( strCaption.Mid( i, 2 ) == _T("::") )
				{
					strCaption = strCaption.Mid( i + 2 );
					break;
				}
		}
	}
	else
	{
		m_pFrame->SetMsgText(GenerateEmptyPageMessage(m_strEmptyPageMessage, strCaption));
	}

	// if no captions are displayed, cancel here
	if (!m_pFrame->GetShowCaption())
		return;

	// get tab control, to the the images from
	CTabCtrl	*pTabCtrl = GetTabControl();
	if (!IsWindow(pTabCtrl->GetSafeHwnd()))
	{
		ASSERT(FALSE);
		return;
	}

	if (m_bTreeImages)
	{
		// get image from tree
		int	nImage;
		m_pwndPageTree->GetItemImage(hItem, nImage, nImage);
		HICON	hIcon = m_Images.ExtractIcon(nImage);
		m_pFrame->SetCaption(strCaption, hIcon);
		if (hIcon)
			DestroyIcon(hIcon);
	}
	else if (bRealPage)
	{
		// get image from hidden (original) tab provided by the original
		// implementation
		CImageList	*pImages = pTabCtrl->GetImageList();
		if (pImages)
		{
			TCITEM	ti;
			ZeroMemory(&ti, sizeof(ti));
			ti.mask = TCIF_IMAGE;

			HICON	hIcon = NULL;
			if (pTabCtrl->GetItem((int)m_pwndPageTree->GetItemData(hItem), &ti))
				hIcon = pImages->ExtractIcon(ti.iImage);

			m_pFrame->SetCaption(strCaption, hIcon);
			if (hIcon)
				DestroyIcon(hIcon);
		}
		else
			m_pFrame->SetCaption(strCaption);
	}
	else
		m_pFrame->SetCaption(strCaption);
}


void CTreePropSheet::ActivatePreviousPage()
{
	if (!IsWindow(m_hWnd))
		return;

	if (!IsWindow(m_pwndPageTree->GetSafeHwnd()))
	{
		// normal tab property sheet. Simply use page index
		int	nPageIndex = GetActiveIndex();
		if (nPageIndex<0 || nPageIndex>=GetPageCount())
			return;

		int	nPrevIndex = (nPageIndex==0)? GetPageCount()-1 : nPageIndex-1;
		SetActivePage(nPrevIndex);
	}
	else
	{
		// property sheet with page tree.
		// we need a more sophisticated handling here, than simply using
		// the page index, because we won't skip empty pages.
		// so we have to walk the page tree
		HTREEITEM	hItem = m_pwndPageTree->GetSelectedItem();
		ASSERT(hItem);
		if (!hItem)
			return;

		HTREEITEM	hPrevItem = NULL;
		if ((hPrevItem=m_pwndPageTree->GetPrevSiblingItem(hItem))!=0)
		{
			while (m_pwndPageTree->ItemHasChildren(hPrevItem))
			{
				hPrevItem = m_pwndPageTree->GetChildItem(hPrevItem);
				while (m_pwndPageTree->GetNextSiblingItem(hPrevItem))
					hPrevItem = m_pwndPageTree->GetNextSiblingItem(hPrevItem);
			}
		}
		else 
			hPrevItem=m_pwndPageTree->GetParentItem(hItem);

		if (!hPrevItem)
		{
			// no prev item, so cycle to the last item
			hPrevItem = m_pwndPageTree->GetRootItem();

#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression constant
			while (TRUE)
			{
				while (m_pwndPageTree->GetNextSiblingItem(hPrevItem))
					hPrevItem = m_pwndPageTree->GetNextSiblingItem(hPrevItem);

				if (m_pwndPageTree->ItemHasChildren(hPrevItem))
					hPrevItem = m_pwndPageTree->GetChildItem(hPrevItem);
				else
					break;
			}
#pragma warning(pop)
		}

		if (hPrevItem)
			m_pwndPageTree->SelectItem(hPrevItem);
	}
}


void CTreePropSheet::ActivateNextPage()
{
	if (!IsWindow(m_hWnd))
		return;

	if (!IsWindow(m_pwndPageTree->GetSafeHwnd()))
	{
		// normal tab property sheet. Simply use page index
		int	nPageIndex = GetActiveIndex();
		if (nPageIndex<0 || nPageIndex>=GetPageCount())
			return;

		int	nNextIndex = (nPageIndex==GetPageCount()-1)? 0 : nPageIndex+1;
		SetActivePage(nNextIndex);
	}
	else
	{
		// property sheet with page tree.
		// we need a more sophisticated handling here, than simply using
		// the page index, because we won't skip empty pages.
		// so we have to walk the page tree
		HTREEITEM	hItem = m_pwndPageTree->GetSelectedItem();
		ASSERT(hItem);
		if (!hItem)
			return;

		HTREEITEM	hNextItem = NULL;
		if ((hNextItem=m_pwndPageTree->GetChildItem(hItem))!=0)
			;
		else if ((hNextItem=m_pwndPageTree->GetNextSiblingItem(hItem))!=0)
			;
		else if (m_pwndPageTree->GetParentItem(hItem))
		{
			while (!hNextItem)
			{
				hItem = m_pwndPageTree->GetParentItem(hItem);
				if (!hItem)
					break;

				hNextItem	= m_pwndPageTree->GetNextSiblingItem(hItem);
			}
		}

		if (!hNextItem)
			// no next item -- so cycle to the first item
			hNextItem = m_pwndPageTree->GetRootItem();

		if (hNextItem)
			m_pwndPageTree->SelectItem(hNextItem);
	}
}


/////////////////////////////////////////////////////////////////////
// Overridings

BOOL CTreePropSheet::OnInitDialog() 
{
	bool isThemed = false;
	if( m_osVer >= 0x0501 )
		isThemed = ::IsThemeActive() == TRUE;

	if (m_bTreeViewMode)
	{
		// be sure, there are no stacked tabs, because otherwise the
		// page caption will be to large in tree view mode
		EnableStackedTabs(FALSE);

		// Initialize image list.
		if (m_DefaultImages.GetSafeHandle())
		{
			IMAGEINFO	ii;
			m_DefaultImages.GetImageInfo(0, &ii);
			if (ii.hbmImage) DeleteObject(ii.hbmImage);
			if (ii.hbmMask) DeleteObject(ii.hbmMask);
			m_Images.Create(ii.rcImage.right-ii.rcImage.left, ii.rcImage.bottom-ii.rcImage.top, ILC_COLOR32|ILC_MASK, 0, 1);
		}
		else
			m_Images.Create(16, 16, ILC_COLOR32|ILC_MASK, 0, 1);
	}

	// perform default implementation
	BOOL bResult = CPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);

	if (!m_bTreeViewMode)
		// stop here, if we would like to use tabs
		return bResult;

	// Get tab control...
	CTabCtrl	*pTab = GetTabControl();
	if (!IsWindow(pTab->GetSafeHwnd()))
	{
		ASSERT(FALSE);
		return bResult;
	}

	// ... and hide it
	pTab->ShowWindow(SW_HIDE);
	pTab->EnableWindow(FALSE);

	// Place another (empty) tab ctrl, to get a frame instead
	CRect	rectFrame;
	pTab->GetWindowRect(rectFrame);
	ScreenToClient(rectFrame);

	m_pFrame = CreatePageFrame();
	if (!m_pFrame)
	{
		ASSERT(FALSE);
		AfxThrowMemoryException();
	}
	m_pFrame->Create(WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, rectFrame, this, 0xFFFF);
	m_pFrame->ShowCaption(m_bPageCaption);

	// Lets make place for the tree ctrl
	CRect rcTreeWidth( 0, 0, m_nPageTreeWidth, 1 );
	MapDialogRect( rcTreeWidth );
	CRect rcTreeSpace( 0, 0, 3, 1 );
	MapDialogRect( rcTreeSpace );

	CRect	rectSheet;
	GetWindowRect(rectSheet);
	rectSheet.right+= rcTreeWidth.Width();
	SetWindowPos(NULL, -1, -1, rectSheet.Width(), rectSheet.Height(), SWP_NOZORDER|SWP_NOMOVE);
	CenterWindow();

	MoveChildWindows(rcTreeWidth.Width(), 0);

	// Lets calculate the rectangle for the tree ctrl
	CRect	rectTree(rectFrame);
	rectTree.right = rectTree.left + rcTreeWidth.Width() - rcTreeSpace.Width();

	// calculate caption height

	CRect	rectFrameCaption;
	CTabCtrl	wndTabCtrl;
	wndTabCtrl.Create(WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, rectFrame, this, 0x1234);
	wndTabCtrl.InsertItem(0, _T(""));
	wndTabCtrl.GetItemRect(0, rectFrameCaption);
	wndTabCtrl.DestroyWindow();
	
	m_pFrame->SetCaptionHeight(rectFrameCaption.Height());

	if( m_bPageCaption )
	{
		if( m_osVer >= 0x0600 && isThemed )
			if( HTHEME hTheme = ::OpenThemeData( *this, L"TEXTSTYLE" ) )
			{
				// Vista: get height of task dialog "main instruction" font and
				// move / resize everything to fit the headline.

				CRect rc( 0, 0, 1, 1 ); 
				CWindowDC dc( this );
				::GetThemeTextExtent( hTheme, dc, TEXT_MAININSTRUCTION, 0, L"�y", -1, 
					DT_SINGLELINE | DT_CALCRECT, NULL, rc );		
				::CloseThemeData( hTheme );				

				CRect margins( 0, 0, 6, 6 ); ::MapDialogRect( *this, margins );			
				rc.bottom += margins.bottom * 2;

				m_pFrame->SetCaptionHeight( rc.Height() );
									
				int diff = rc.Height() - rectFrameCaption.Height();
									
				// enlarge frame
				m_pFrame->GetWnd()->GetWindowRect(rectFrame);
				ScreenToClient(rectFrame);
				rectFrame.top -= diff;		
				m_pFrame->GetWnd()->MoveWindow(rectFrame);
				
				// move all child windows down
				MoveChildWindows( 0, diff );

				// modify rectangle for the tree ctrl
				rectTree.bottom += diff;

				// make us larger
				CRect	rect;
				GetWindowRect(rect);
				rect.top -= diff / 2;
				rect.bottom += diff / 2 + diff % 2 * 2;
				MoveWindow( rect );				
			}			
	}
	else
	{
		// No caption should be displayed -> reduce the window height.

		// make frame smaller
		m_pFrame->GetWnd()->GetWindowRect(rectFrame);
		ScreenToClient(rectFrame);
		rectFrame.top += rectFrameCaption.Height();		
		m_pFrame->GetWnd()->MoveWindow(rectFrame);

		// move all child windows up
		MoveChildWindows(0, -rectFrameCaption.Height());

		// modify rectangle for the tree ctrl
		rectTree.bottom-= rectFrameCaption.Height();

		// make us smaller
		CRect	rect;
		GetWindowRect(rect);
		rect.top+= rectFrameCaption.Height()/2;
		rect.bottom-= rectFrameCaption.Height()-rectFrameCaption.Height()/2;
		MoveWindow(rect);
	}

	// finally create the tree control
	const DWORD	dwTreeStyle = TVS_SHOWSELALWAYS|TVS_TRACKSELECT|TVS_HASLINES/*|TVS_LINESATROOT|TVS_HASBUTTONS*/;
	m_pwndPageTree = CreatePageTreeObject();
	if (!m_pwndPageTree)
	{
		ASSERT(FALSE);
		AfxThrowMemoryException();
	}
	
	// MFC7-support here (Thanks to Rainer Wollgarten)
	// YT: Cast tree control to CWnd and calls CWnd::CreateEx in all cases (VC 6 and7).
	((CWnd*)m_pwndPageTree)->CreateEx(
		WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY,
		_T("SysTreeView32"), _T("PageTree"),
		WS_TABSTOP|WS_CHILD|WS_VISIBLE|dwTreeStyle,
		rectTree, this, s_unPageTreeId);

	m_pwndPageTree->SetFont( GetFont() );
	
	// Use Vista theme if possible
	if( m_osVer >= 0x0600 )
	{
		::SetWindowTheme( *m_pwndPageTree, L"explorer", NULL );
		TreeView_SetExtendedStyle( *m_pwndPageTree, 
			TVS_EX_FADEINOUTEXPANDOS | TVS_EX_DOUBLEBUFFER, TVS_EX_FADEINOUTEXPANDOS | TVS_EX_DOUBLEBUFFER );
	}
		
	if (m_bTreeImages)
	{
		m_pwndPageTree->SetImageList(&m_Images, TVSIL_NORMAL);
		m_pwndPageTree->SetImageList(&m_Images, TVSIL_STATE);
	}

	// Fill the tree ctrl
	RefillPageTree();

	// Select item for the current page
	if (pTab->GetCurSel() > -1)
		SelectPageTreeItem(pTab->GetCurSel());
	return bResult;
}


void CTreePropSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();
	
	if (m_Images.GetSafeHandle())
		m_Images.DeleteImageList();

	delete m_pwndPageTree;
	m_pwndPageTree = NULL;

	delete m_pFrame;
	m_pFrame = NULL;
}


LRESULT CTreePropSheet::OnAddPage(WPARAM wParam, LPARAM lParam)
{
	LRESULT	lResult = DefWindowProc(PSM_ADDPAGE, wParam, lParam);
	if (!m_bTreeViewMode)
		return lResult;

	RefillPageTree();
	SelectCurrentPageTreeItem();

	return lResult;
}


LRESULT CTreePropSheet::OnRemovePage(WPARAM wParam, LPARAM lParam)
{
	LRESULT	lResult = DefWindowProc(PSM_REMOVEPAGE, wParam, lParam);
	if (!m_bTreeViewMode)
		return lResult;

	RefillPageTree();
	SelectCurrentPageTreeItem();

	return lResult;
}


LRESULT CTreePropSheet::OnSetCurSel(WPARAM wParam, LPARAM lParam)
{
	LRESULT	lResult = DefWindowProc(PSM_SETCURSEL, wParam, lParam);
	if (!m_bTreeViewMode)
		return lResult;

	SelectCurrentPageTreeItem();
	UpdateCaption();
	return lResult;
}


LRESULT CTreePropSheet::OnSetCurSelId(WPARAM wParam, LPARAM lParam)
{
	LRESULT	lResult = DefWindowProc(PSM_SETCURSEL, wParam, lParam);
	if (!m_bTreeViewMode)
		return lResult;

	SelectCurrentPageTreeItem();
	UpdateCaption();
	return lResult;
}


void CTreePropSheet::OnPageTreeSelChanging(NMHDR *pNotifyStruct, LRESULT *plResult)
{
	*plResult = 0;
	if (m_bPageTreeSelChangedActive)
		return;
	else
		m_bPageTreeSelChangedActive = TRUE;

	NMTREEVIEW	*pTvn = reinterpret_cast<NMTREEVIEW*>(pNotifyStruct);
	UINT	nPage = UINT( m_pwndPageTree->GetItemData( pTvn->itemNew.hItem ) );
	BOOL				bResult;
	if (nPage >= m_pwndPageTree->GetCount())
		bResult = KillActiveCurrentPage();
	else
		bResult = SetActivePage( nPage );

	if (!bResult)
		// prevent selection to change
		*plResult = TRUE;

	// Set focus to tree ctrl (I guess that's what the user expects)
	m_pwndPageTree->SetFocus();

	m_bPageTreeSelChangedActive = FALSE;

	return;
}


void CTreePropSheet::OnPageTreeSelChanged(NMHDR * /*pNotifyStruct*/, LRESULT *plResult)
{
	*plResult = 0;

	UpdateCaption();

	return;
}


LRESULT CTreePropSheet::OnIsDialogMessage(WPARAM wParam, LPARAM lParam)
{
	MSG	*pMsg = reinterpret_cast<MSG*>(lParam);
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_TAB && GetKeyState(VK_CONTROL)&0x8000)
	{
		if (GetKeyState(VK_SHIFT)&0x8000)
			ActivatePreviousPage();
		else
			ActivateNextPage();
		return TRUE;
	}


	return CPropertySheet::DefWindowProc(PSM_ISDIALOGMESSAGE, wParam, lParam);
}

} //namespace TreePropSheet
