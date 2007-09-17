﻿
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Files.hpp"

#define TVS_EX_DOUBLEBUFFER 0x0004

FileListView::FileListView() :
	iniClass("listviews/advFiles", "FileListView"),
	listClass(true,false,false)
{					
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	// "Filename;Path;Size;Progress;Priority"
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 5> widths = {100,70,70,70,70};
	array<int, 5> order = {0,1,2,3,4};
	array<bool, 5> visible = {true,true,true,true,true};
	
	SetDefaults(names, widths, order, visible, true);
	Load();
}

HWND FileListView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
	DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = listClass::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	ApplyDetails();
	
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
	
	SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::Size());
	SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::Progress());
	SetColumnSortType(4, LVCOLSORT_CUSTOM, new ColumnAdapters::Priority());
	
	return hwnd;
}

void FileListView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	std::vector<int> indices;
	
	for (int i=0, e=GetItemCount(); i<e; ++i)
	{
		UINT flags = GetItemState(i, LVIS_SELECTED);
		
		if (flags & LVIS_SELECTED)
			indices.push_back(GetItemData(i));
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;
	
	std::string torrent = hal::to_utf8(hal::bittorrent().torrentDetails().selectedTorrent()->filename());
	hal::bittorrent().setTorrentFilePriorities(torrent, indices, priority);
}

HWND FileTreeView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = treeClass::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(IDR_FILESLISTVIEW_MENU);
	assert(menu_created);	
	
	menu_.Attach(menu.GetSubMenu(0));
	
	return hwnd;
}

LRESULT FileTreeView::OnRClick(int i, LPNMHDR pnmh, BOOL&)
{
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
	
	if (menu_)
	{
		assert (menu_.IsMenu());

		POINT ptPoint;
		GetCursorPos(&ptPoint);
		menu_.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
	}

	return 0;
}

void FileTreeView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	hal::FileDetails fileDetails;
	
//	foreach (const hal::TorrentDetail_ptr torrent, tD.selectedTorrents())
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().focusedTorrent())
	{
		std::copy(torrent->fileDetails().begin(), torrent->fileDetails().end(), 
			std::back_inserter(fileDetails));
	}

	wpath branch;
	
	if (CTreeItem ti = GetSelectedItem())
	{			
		do
		{
			if (!ti.GetParent()) break;
			
			boost::array<wchar_t, MAX_PATH> buffer;
			ti.GetText(buffer.elems, MAX_PATH);
			
			branch = wstring(buffer.elems)/branch;
		} 
		while (ti = ti.GetParent());
	}

	std::vector<int> indices;
	
	for (hal::FileDetails::iterator i=fileDetails.begin(), e=fileDetails.end();
		i != e; ++i)
	{			
		if (std::equal(branch.begin(), branch.end(), (*i).branch.begin()))
		{
			indices.push_back((*i).order());
		}
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;
	
	std::string torrent = hal::to_utf8(hal::bittorrent().torrentDetails().selectedTorrent()->filename());
	hal::bittorrent().setTorrentFilePriorities(torrent, indices, priority);
}

LRESULT FileTreeView::OnSelChanged(int, LPNMHDR pnmh, BOOL&)
{		
	wpath branch;
	
	TryUpdateLock<thisClass> lock(*this);
	if (lock)
	{		
		if (CTreeItem ti = GetSelectedItem())
		{			
			do
			{
				if (!ti.GetParent()) break;
				
				boost::array<wchar_t, MAX_PATH> buffer;
				ti.GetText(buffer.elems, MAX_PATH);
				
				branch = wstring(buffer.elems)/branch;
			} 
			while (ti = ti.GetParent());
		}
		
		signal();
	}
	
	focused_ = branch;
	return 0;
}

LRESULT AdvFilesDialog::onInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	CRect rc; GetClientRect(&rc);
	
	static_.SubclassWindow(GetDlgItem(IDC_CONTAINER));
	
	splitter_.Create(GetDlgItem(IDC_CONTAINER), rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
	splitter_.SetSplitterExtendedStyle(!SPLIT_PROPORTIONAL, SPLIT_PROPORTIONAL);
	
	list_.Create(splitter_, rc, NULL, 
		LVS_REPORT|WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|LVS_SHOWSELALWAYS,
		WS_EX_STATICEDGE|LVS_EX_DOUBLEBUFFER);
		
	tree_.Create(splitter_, rc, NULL, 
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|TVS_HASBUTTONS|
			TVS_HASLINES|TVS_TRACKSELECT|TVS_SHOWSELALWAYS,
		TVS_EX_DOUBLEBUFFER|WS_EX_STATICEDGE);
	
	tree_.attach(bind(&AdvFilesDialog::doUiUpdate, this));
	
	splitter_.SetSplitterPanes(tree_, list_);
	splitter_.SetSplitterPos(splitterPos);
	
	CTreeItem ti = tree_.InsertItem(hal::app().res_wstr(HAL_TORRENT_ROOT).c_str(), TVI_ROOT, TVI_LAST);
	
//	DoDataExchange(false);
	
	return 0;
}

void AdvFilesDialog::DlgResize_UpdateLayout(int cxWidth, int cyHeight)
{
	resizeClass::DlgResize_UpdateLayout(cxWidth, cyHeight);
	
	CRect rect; ::GetClientRect(GetDlgItem(IDC_CONTAINER), &rect);
	
	splitter_.SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}


void AdvFilesDialog::doUiUpdate()
{
	uiUpdate(hal::bittorrent().torrentDetails());
}

void AdvFilesDialog::uiUpdate(const hal::TorrentDetails& tD)
{	
	if (hal::TorrentDetail_ptr torrent = tD.selectedTorrent()) 	
	{	
		string torrent_name = hal::to_utf8(torrent->filename());
		
		if (current_torrent_name_ != torrent_name)
		{	
			current_torrent_name_ = torrent_name;
			focusChanged(current_torrent_name_);
		}
	}
	else
	{	
		if (current_torrent_name_ != "")
		{	
			current_torrent_name_ = "";
			focusChanged(current_torrent_name_);
		}	
	}
	
	std::pair<hal::FileDetails::iterator, hal::FileDetails::iterator> range =
		std::equal_range(fileDetails.begin(), fileDetails.end(), hal::FileDetail(tree_.focused(), L""));
	
	std::sort(range.first, range.second, &hal::FileDetailNamesLess);
	
	TryUpdateLock<FileListView::listClass> lock(list_);
	if (lock) 
	{	
		// Wipe details not present
		for(int i = 0; i < list_.GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> fullPath;
			list_.GetItemText(i, 0, fullPath.c_array(), MAX_PATH);
			
			hal::FileDetail file(L"", wstring(fullPath.c_array()));
			hal::FileDetails::iterator iter = 
				std::lower_bound(range.first, range.second, file, &hal::FileDetailNamesLess);
			
			if (iter == range.second || !(hal::FileDetailNamesEqual((*iter), file)))
			{
				if (iter == range.second)
					hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Deleting %1%") % file.filename).str().c_str())));
				else
					hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Deleting %1% != %2%") % file.filename % (*iter).filename).str().c_str())));
				
				list_.DeleteItem(i);
			}
			else
			{
				list_.SetItemData(i, std::distance(range.first, iter));
				++i;
			}
		}
		
		// Add additional details
		for (hal::FileDetails::iterator i=range.first, e=range.second;
			i != e; ++i)
		{			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).filename.c_str());
			
			int itemPos = list_.FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = list_.AddItem(list_.GetItemCount(), 0, (*i).filename.c_str(), 0);
			
			list_.SetItemData(itemPos, (*i).order());
			
			list_.SetItemText(itemPos, 1, (*i).branch.string().c_str());			
			list_.SetItemText(itemPos, 2, list_.getColumnAdapter(2)->print(*i).c_str());
			list_.SetItemText(itemPos, 3, list_.getColumnAdapter(3)->print(*i).c_str());
			list_.SetItemText(itemPos, 4, list_.getColumnAdapter(4)->print(*i).c_str());			
		}

		list_.ConditionallyDoAutoSort();
	}
}

void AdvFilesDialog::focusChanged(string& torrent_name)
{
	const hal::TorrentDetails& tD = hal::bittorrent().torrentDetails();
	
//	foreach (const hal::TorrentDetail_ptr torrent, tD.selectedTorrents())
	if (hal::TorrentDetail_ptr torrent = tD.focusedTorrent())
	{
		std::copy(torrent->fileDetails().begin(), torrent->fileDetails().end(), 
			std::back_inserter(fileDetails));
	}
	
	list_.setFocused(tD.focusedTorrent());
	
	std::sort(fileDetails.begin(), fileDetails.end());
	
	{ 	UpdateLock<FileTreeView> lock(tree_);
	
		treeManager_.InvalidateAll();
		
		foreach (hal::FileDetail file, fileDetails)
		{
			treeManager_.EnsureValid(file.branch);
		}
		
		treeManager_.ClearInvalid();
	}
	

	
	splitterPos = splitter_.GetSplitterPos();
}

void AdvFilesDialog::onClose()
{		
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}
