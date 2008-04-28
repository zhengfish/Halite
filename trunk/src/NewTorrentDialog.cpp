
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "../res/resource.h"

#include "NewTorrentDialog.hpp"
#include "CSSFileDialog.hpp"

void FilesListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(0, LVCOLSORT_TEXTNOCASE);
	SetColumnSortType(1, LVCOLSORT_TEXTNOCASE);
	SetColumnSortType(2, LVCOLSORT_LONG);
}

void FilesListViewCtrl::OnDestroy()
{
	saveSettings();
}

void FilesListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save();
}

void FileSheet::OnFileBrowse(UINT, int, HWND hWnd)
{	
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"All Files (*.*)|*.*|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
	//	ProcessFile(dlgOpen.m_ofn.lpstrFile);
	}
}

void FileSheet::OnOutBrowse(UINT, int, HWND hWnd)
{	
	CSSFileDialog dlgOpen(false, NULL, NULL, OFN_HIDEREADONLY, L"Torrents. (*.torrent)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
		SetDlgItemText(IDC_NEWT_FILE, dlgOpen.m_ofn.lpstrFile);
		EnableSave_(true);
	}
}

void recurseDirectory(std::vector<wpath>& files, wpath baseDir, wpath relDir)
{	
	wpath currentDir(baseDir / relDir);

	if (hal::fs::is_directory(currentDir))
    {
		for (hal::fs::wdirectory_iterator i(currentDir), end; i != end; ++i)
			recurseDirectory(files, baseDir, relDir / i->leaf());
    }
    else
    {
//		HAL_DEV_MSG(currentDir.string());
		files.push_back(relDir);		
    }
}

void FileSheet::OnDirBrowse(UINT, int, HWND hWnd)
{	
	CFolderDialog fldDlg(NULL, L"",
		BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

	files_.clear();
	filesList_.DeleteAllItems();

	if (IDOK == fldDlg.DoModal())
	{
		fileRoot_ = wpath(fldDlg.m_szFolderPath);

		recurseDirectory(files_, fileRoot_, L"/");

		foreach(wpath& file, files_)
		{
			int itemPos = filesList_.AddItem(0, 0, file.leaf().c_str(), 0);

			filesList_.SetItemText(itemPos, 1, file.branch_path().string().c_str());
			filesList_.SetItemText(itemPos, 2, lexical_cast<wstring>(
				hal::fs::file_size(fileRoot_/file)).c_str());
		}
	}
}

LRESULT FileSheet::onInitDialog(HWND, LPARAM)
{	
	filesList_.Attach(GetDlgItem(IDC_NEWT_LISTFILES));	

	BOOL retval =  DoDataExchange(false);
	return 0;
}
	
wpath FileSheet::FileFullPath() const
{
	return fileRoot_;
}

hal::file_size_pairs_t FileSheet::FileSizePairs() const
{
	hal::file_size_pairs_t filePairs;

	for (int i = 0, e = filesList_.GetItemCount(); i<e; ++i)
	{
		hal::win_c_str<std::wstring> name_buf(MAX_PATH);		
		filesList_.GetItemText(i, 0, name_buf, name_buf.size());

		hal::win_c_str<std::wstring> path_buf(MAX_PATH);		
		filesList_.GetItemText(i, 1, path_buf, path_buf.size());

//		HAL_DEV_MSG(wformat(L"File: %1%, size: %2%, both: %3%") % fileRoot_.string() % name_buf.str() % (fileRoot_ / path_buf / name_buf).string());

		filePairs.push_back(hal::make_pair(
			wpath(wpath(path_buf.str()) / name_buf).string(), 
			hal::fs::file_size(fileRoot_ / path_buf / name_buf)));
	}

	return filePairs;
}

wpath FileSheet::OutputFile()
{
	DoDataExchange(true);

	return outFile_;
}

hal::tracker_details_t TrackerSheet::Trackers() const
{
	hal::tracker_details_t trackers;

	for (int i = 0, e = trackerList_.GetItemCount(); i<e; ++i)
	{
		hal::win_c_str<std::wstring> str_buf(MAX_PATH);		
		trackerList_.GetItemText(i, 0, str_buf, str_buf.size());

		hal::win_c_str<std::wstring> tier_buf(MAX_PATH);		
		trackerList_.GetItemText(i, 1, tier_buf, tier_buf.size());

		trackers.push_back(hal::tracker_detail(
			str_buf, lexical_cast<unsigned>(tier_buf.str())));
	}

	return trackers;
}

#define NEWTORRENT_SELECT_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|150), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_SELECT_TEXT, IDC_NEWT_FILE_BROWSE, IDC_NEWT_DIR_BROWSE), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTFILES,  _r, _r), \
	WMB_END()

#define NEWTORRENT_BUTTONS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_PRIVATE,  IDOK, IDCANCEL), \
	WMB_END()

#define NEWTORRENT_COMMENT_LAYOUT \
	WMB_HEAD(WMB_COL(_auto), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_COMMENT_TEXT,  IDC_NEWTORRENT_COMMENT), \
	WMB_END()

#define NEWTORRENT_CREATOR_LAYOUT \
	WMB_HEAD(WMB_COL(_auto), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_CREATOR_TEXT,  IDC_NEWTORRENT_CREATOR), \
	WMB_END()

#define NEWTORRENT_OUTPUT_LAYOUT \
	WMB_HEAD(WMB_COL(_auto), WMB_COLNOMIN(_exp), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWT_OUTFILE_TEXT,  IDC_NEWT_FILE, IDC_NEWT_OUT_BROWSE), \
	WMB_END()

FileSheet::CWindowMapStruct* FileSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(FileSheet, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_SELECT_LAYOUT),
		WMB_ROW(_auto, NEWTORRENT_OUTPUT_LAYOUT),
		WMB_ROW(_auto, NEWTORRENT_CREATOR_LAYOUT),
		WMB_ROW(_auto, NEWTORRENT_COMMENT_LAYOUT),
		WMB_ROW(_auto, IDC_NEWTORRENT_PRIVATE),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	

#define NEWTORRENT_TRACKERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_TRACKERS_TEXT, _r, _r), \
/*		WMB_ROW(_auto,	IDC_NEWT_TRACKER, IDC_NEWT_TRACKERTIER, IDC_NEWT_ADDTRACKER), */\
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTTRACKERS,  _r, _r), \
	WMB_END()

TrackerSheet::CWindowMapStruct* TrackerSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(TrackerSheet, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_TRACKERS_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	

#define NEWTORRENT_DETAILS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|33), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_COMMENT_TEXT,  IDC_NEWTORRENT_COMMENT), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_CREATOR_TEXT,  IDC_NEWTORRENT_CREATOR), \
	WMB_END()


#define NEWTORRENT_PEERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_PEERS_TEXT, _r, _r), \
/*		WMB_ROW(_auto,	IDC_NEWT_TRACKER, IDC_NEWT_TRACKERTIER, IDC_NEWT_ADDTRACKER), */\
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTPEERS,  _r, _r), \
	WMB_END()

PeersSheet::CWindowMapStruct* PeersSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(PeersSheet, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_PEERS_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	

void NewTorrentDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
    resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);

	hal::event().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"NewTorrentDialog::OnShowWindow()")));

    if (bShow && !inited_)
    {
        CMenuHandle pSysMenu = GetSystemMenu(FALSE);

    	if (pSysMenu != NULL)
            pSysMenu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, SC_SIZE, L"&Size");

        ModifyStyle(0, WS_THICKFRAME, 0);

		if (rect_.left == rect_.right)
		{
			CenterWindow();
		}
		else
		{
			MoveWindow(rect_.left, rect_.top, 
				rect_.right-rect_.left, rect_.bottom-rect_.top, true);	
		}

		::SetWindowText(GetDlgItem(0x1), hal::app().res_wstr(HAL_SAVE_TEXT).c_str());
		::EnableWindow(GetDlgItem(0x1), false);

		inited_ = true;
		resizeActiveSheet();
    }
	else
	{
		SetMsgHandled(false);
	}
}

LRESULT NewTorrentDialog::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

	hal::event().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"NewTorrentDialog::OnSave()")));

	hal::create_torrent_params params;

	params.file_size_pairs = fileSheet_.FileSizePairs();
	params.root_path = fileSheet_.FileFullPath();

	params.trackers = trackerSheet_.Trackers();

	hal::bittorrent().create_torrent(params, fileSheet_.OutputFile());

	}
	catch(const std::exception& e)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(hal::Event::critical, e, 
				L"NewTorrentDialog::OnSave")));
	}

	return 0;
}